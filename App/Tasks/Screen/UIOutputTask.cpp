#include "UIOutputTask.hpp"

#include "LVGLTask.hpp" // 为了比较 buf 指针以释放对应 semaphore
#include "if_port.h"
#include "spi.h"

/**
 * @brief SPI2 DMA 发送完成中断回调，释放 CS 并给任务发送完成信号。
 * @param hspi SPI 句柄。
 */
extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hspi == &hspi2) {
    SPI_CS_chooseless();

    if (UIOutputTask::dma_done_sem_ != nullptr) {
      xSemaphoreGiveFromISR(UIOutputTask::dma_done_sem_, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

/**
 * @brief 清理一段内存的 D-Cache，确保 DMA 读到最新数据。
 * @param ptr 数据起始地址。
 * @param len 字节长度。
 */
void UIOutputTask::clean_dcache_for_range(const void* ptr, uint32_t len) {
  if (ptr == nullptr || len == 0)
    return;
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t start = addr & ~static_cast<uintptr_t>(0x1F);
  uintptr_t end = (addr + len + 31) & ~static_cast<uintptr_t>(0x1F);
  SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
}

/**
 * @brief 注入同步原语，在启动任务前调用。
 * @param display_queue 来自 LVGL 的显示消息队列。
 * @param buf1_sem 缓冲区 1 对应的信号量。
 * @param buf2_sem 缓冲区 2 对应的信号量。
 */
void UIOutputTask::inject(
    QueueHandle_t& display_queue,
    SemaphoreHandle_t& buf1_sem,
    SemaphoreHandle_t& buf2_sem) {
  display_queue_ = display_queue;
  buf1_sem_ = buf1_sem;
  buf2_sem_ = buf2_sem;

  // 创建 DMA 完成信号量（如果尚未创建）
  if (dma_done_sem_ == nullptr) {
    dma_done_sem_ = xSemaphoreCreateBinary();
    // 不要 xSemaphoreGive 这里，让第一次 HAL 回调来 give
  }
}

/**
 * @brief 完成 LT768 初始化和窗口配置。
 */
void UIOutputTask::InitializeDisplay() {
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
}

/**
 * @brief DMA 任务主循环：等待 LVGL 消息并串行处理。
 */
void UIOutputTask::Run() {
  InitializeDisplay();

  // 等待同步原语注入
  while (display_queue_ == nullptr || buf1_sem_ == nullptr || buf2_sem_ == nullptr || dma_done_sem_ == nullptr) {
    DelayMs(10);
  }

  DisplayMessage msg;

  while (true) {
    // 等待显示消息
    if (xQueueReceive(display_queue_, &msg, portMAX_DELAY) == pdTRUE) {
      ProcessDisplayMessage(msg);
      // 不再主动批量拉取 queue，保持简单串行处理
    }
    // DrawBitmapDMA(CANVAS_BASE, 0, 0, 800, 60, (uint8_t*)buf_test);
    // LT768_BTE_Memory_Copy(CANVAS_BASE, LCD_WIDTH, 0, 0, 0, LCD_WIDTH,
    //                       0, 0, 0, LCD_WIDTH, 0, 0, 0b1100, LCD_WIDTH, LCD_HEIGHT);
  }
}

/**
 * @brief 处理一条显示消息：DMA 写入 + BTE 拷贝 + 通知 LVGL。
 */
/**
 * @brief 处理一条显示消息：DMA 写入 + BTE 拷贝 + 通知 LVGL。
 * @param msg 显示消息。
 */
void UIOutputTask::ProcessDisplayMessage(const DisplayMessage& msg) {
  uint32_t w = lv_area_get_width(&msg.area);
  uint32_t h = lv_area_get_height(&msg.area);

  // 使用DMA传输图像数据
  DrawBitmapDMA(CANVAS_BASE,
                msg.area.x1, msg.area.y1,
                static_cast<uint16_t>(w), static_cast<uint16_t>(h),
                msg.px_buf);

  // 执行 BTE 内存复制到主窗口（保留你既有逻辑）
  LT768_BTE_Memory_Copy(CANVAS_BASE, LCD_WIDTH, 0, 0, 0, LCD_WIDTH,
                        0, 0, 0, LCD_WIDTH, 0, 0, 0b1100, LCD_WIDTH, LCD_HEIGHT);

  // 通知 LVGL 刷新完成（此处在任务上下文中调用安全）
  lv_display_flush_ready(msg.disp);

  // 释放对应的缓冲区信号量（允许 LVGL 重用该 buffer）
  if (msg.px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf1())) {
    xSemaphoreGive(buf1_sem_);
  } else if (msg.px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf2())) {
    xSemaphoreGive(buf2_sem_);
  } else {
    // 未识别的缓冲区指针：不操作（防止释放错误的 sem）
  }
}

/**
 * @brief 通过 SPI DMA 将指定区域写入离屏缓冲。
 */
/**
 * @brief 通过 SPI DMA 将指定区域写入离屏缓冲。
 * @param canvas_base 显存基地址。
 * @param x 目标区域 X 起点。
 * @param y 目标区域 Y 起点。
 * @param width 区域宽度（像素）。
 * @param height 区域高度（像素）。
 * @param bitmap_data 指向像素数据的指针。
 */
void UIOutputTask::DrawBitmapDMA(uint32_t canvas_base,
                                 uint16_t x, uint16_t y,
                                 uint16_t width, uint16_t height,
                                 const uint8_t* bitmap_data) {
  if (width == 0 || height == 0 || bitmap_data == nullptr)
    return;

  const uint32_t bytes_per_line = static_cast<uint32_t>(width) * 2u;
  const uint8_t* line_ptr = bitmap_data;

  // 预清理整块缓存
  clean_dcache_for_range(bitmap_data, bytes_per_line * height);

  for (uint16_t row = 0; row < height; row++) {
    uint32_t addr = canvas_base + ((uint32_t)(y + row) * LCD_WIDTH + x) * 2;

    Goto_Linear_Addr(addr);
    LCD_CmdWrite(0x04);

    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);

    // 启动 DMA 输出（不要在启动前 take dma_done_sem_）
    HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi2,
                                                    const_cast<uint8_t*>(line_ptr),
                                                    static_cast<uint16_t>(bytes_per_line));
    if (status != HAL_OK) {
      // DMA 启动失败：尽量恢复（解锁）并退出行循环
      // 触发回退：直接不给予 dma_done_sem_，跳出
      break;
    }

    // 等待 DMA 完成（带超时保护）
    if (xSemaphoreTake(dma_done_sem_, pdMS_TO_TICKS(200)) != pdTRUE) {
      // 超时处理：尽量继续下一行或退出
      break;
    }

    line_ptr += bytes_per_line;
  }
}
