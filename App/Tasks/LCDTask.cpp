#include "LCDTask.hpp"
#include "cmsis_os.h" // optional
#include "if_port.h"
#include "spi.h"

// HAL SPI 完成回调：用信号量通知任务并保持原有 CS 释放行为
extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hspi == &hspi2) {
    // 释放 CS（保持你原来行为）
    SPI_CS_chooseless();

    // 给任务信号量（如果已创建）
    if (LCDTask::dma_done_semaphore_ != nullptr) {
      xSemaphoreGiveFromISR(LCDTask::dma_done_semaphore_, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

void LCDTask::clean_dcache_for_range(const void* ptr, uint32_t len) {
  if (ptr == nullptr || len == 0)
    return;
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t start = addr & ~static_cast<uintptr_t>(0x1F);
  uintptr_t end = (addr + len + 31) & ~static_cast<uintptr_t>(0x1F);
  SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
}

void LCDTask::invalidate_dcache_for_range(const void* ptr, uint32_t len) {
  if (ptr == nullptr || len == 0)
    return;
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t start = addr & ~static_cast<uintptr_t>(0x1F);
  uintptr_t end = (addr + len + 31) & ~static_cast<uintptr_t>(0x1F);
  SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
}

void LCDTask::InitializeDisplay() {
  Parallel_Init();
  LT768_Init();
  DelayMs(100);

  Display_ON();

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);
  Main_Image_Width(LCD_WIDTH);
  Main_Window_Start_XY(0, 0);

  Active_Window_XY(0, 0);
  Active_Window_WH(LCD_WIDTH, LCD_HEIGHT);

  DelayMs(100);

  LT768_PWM1_Init(1, 0, 50, 100, 100);
  DelayMs(100);

  Canvas_Image_Start_address(CANVAS_BASE);
  Canvas_image_width(LCD_WIDTH);

  // create DMA done semaphore if not exists
  if (dma_done_semaphore_ == nullptr) {
    dma_done_semaphore_ = xSemaphoreCreateBinary();
    // If creation failed, you may want to handle error
  }
}

void LCDTask::UpdateTestPictureBuffer(uint16_t color) {
  for (auto& pixel : test_pic_buffer_) {
    pixel = color;
  }
}
void LCDTask::DrawBitmapDMA(uint32_t canvas_base,
                            uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height,
                            const uint8_t* bitmap_data) {
  if (width == 0 || height == 0 || bitmap_data == nullptr)
    return;

  const uint32_t bytes_per_line = width * 2;
  const uint8_t* line_ptr = bitmap_data;

  // 预清理整块缓存 (减少多次清理)
  LCDTask::clean_dcache_for_range(bitmap_data, bytes_per_line * height);

  for (uint16_t row = 0; row < height; row++) {
    // 计算此行在显存中的线性地址
    uint32_t addr = canvas_base + ((uint32_t)(y + row) * LCD_WIDTH + x) * 2;

    // 设置显存写指针
    Goto_Linear_Addr(addr);
    LCD_CmdWrite(0x04);

    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);

    // 清空之前的信号量
    if (dma_done_semaphore_ != nullptr) {
      xSemaphoreTake(dma_done_semaphore_, 0);
    }

    // 一行 DMA 输出
    HAL_SPI_Transmit_DMA(&hspi2,
                         const_cast<uint8_t*>(line_ptr),
                         bytes_per_line);

    // 等待 DMA 完成（无 busy-wait）
    if (dma_done_semaphore_ != nullptr) {
      xSemaphoreTake(dma_done_semaphore_, portMAX_DELAY);
    } else {
      while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) {
        __NOP();
      }
    }

    line_ptr += bytes_per_line;
  }
}

void LCDTask::Run() {
  InitializeDisplay();

  lv_init();
  lv_tick_set_cb(xTaskGetTickCount);
  lv_display_t* disp = lv_display_create(800, 480);
  lv_display_set_user_data(disp, this);

  lv_display_set_buffers(disp, test_pic_buffer_, nullptr, LCD_WIDTH * ROW_HEIGHT * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, lcd_flush_cb_handle);

  // 创建一个简单的测试UI
  lv_obj_t* screen = lv_screen_active();

  // 设置背景色以确保整个屏幕都被标记为脏区域
  lv_obj_set_style_bg_color(screen, lv_color_hex(0xcf6560), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  CreateSimpleAnimation();

  // ui_init();

  while (true) {
    lv_timer_handler();
    DelayMs(10);
  }
}

void LCDTask::lcd_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_buf) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  // px_buf[0] = 0x00;
  // px_buf[1] = 0xff;
  // px_buf[800 * 96 - 2] = 0x00;
  // px_buf[800 * 96 - 3] = 0xf8;
  // 清理缓存以确保数据一致性
  LCDTask::clean_dcache_for_range(px_buf, w * h * 2u);

  // 使用 DMA 绘制图像
  LCDTask::DrawBitmapDMA(CANVAS_BASE, area->x1, area->y1, static_cast<uint16_t>(w), static_cast<uint16_t>(h), px_buf);
  LT768_BTE_Memory_Copy(CANVAS_BASE, LCD_WIDTH, 0, 0, 0, LCD_WIDTH,
                        0, 0, 0, LCD_WIDTH, 0, 0, 0b1100, LCD_WIDTH, LCD_HEIGHT);
  // 通知 LVGL 刷新完成
  lv_display_flush_ready(disp);
}

void LCDTask::lcd_flush_cb_handle(lv_display_t* disp, const lv_area_t* area, uint8_t* px_buf) {
  LCDTask* task = static_cast<LCDTask*>(lv_display_get_user_data(disp));
  if (task) {
    task->lcd_flush_cb(disp, area, px_buf);
  }
}
void LCDTask::CreateSimpleAnimation() {
  lv_obj_t* screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x9876bd), 0);

  // 创建移动的方块
  lv_obj_t* moving_rect = lv_obj_create(screen);
  lv_obj_set_size(moving_rect, 50, 50);
  lv_obj_set_style_bg_color(moving_rect, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_radius(moving_rect, 0, 0);
  lv_obj_set_pos(moving_rect, 0, 0);

  // 使用静态变量存储动画状态
  static int16_t x = 0;
  static int16_t y = 0;
  static int16_t dx = 5;
  static int16_t dy = 3;

  // 创建动画定时器 - 提供所有3个参数
  lv_timer_create([](lv_timer_t* timer) {
    // 更新位置
    x += dx;
    y += dy;

    // 边界碰撞检测
    if (x <= 0 || x >= LCD_WIDTH - 50)
      dx = -dx;
    if (y <= 0 || y >= LCD_HEIGHT - 60)
      dy = -dy;

    // 获取屏幕上的第一个子对象（我们的方块）
    lv_obj_t* obj = lv_obj_get_child(lv_screen_active(), 0);
    if (obj) {
      lv_obj_set_pos(obj, x, y);
    }
  },
                  16, nullptr); // 第三个参数是用户数据，设为nullptr
}