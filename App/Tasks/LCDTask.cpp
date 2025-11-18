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

void LCDTask::DrawBitmapDMA(uint32_t canvas_base, uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height, const uint8_t* bitmap_data) {
  const uint32_t total_pixels = static_cast<uint32_t>(width) * static_cast<uint32_t>(height);
  if (total_pixels == 0 || bitmap_data == nullptr)
    return;

  const uint32_t linear_address = canvas_base + (static_cast<uint32_t>(y) * LCD_WIDTH + x) * 2u;
  Goto_Linear_Addr(linear_address);
  LCD_CmdWrite(0x04);
  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);

  constexpr uint32_t bytes_per_pixel = 2;
  const uint32_t bytes_per_line = static_cast<uint32_t>(width) * bytes_per_pixel;
  constexpr uint16_t max_lines_per_block = 40;

  uint32_t remaining_lines = height;
  uint32_t current_line = 0;
  const uint8_t* current_data = bitmap_data;

  while (remaining_lines > 0) {
    uint16_t lines_to_send = (remaining_lines > max_lines_per_block) ? max_lines_per_block : static_cast<uint16_t>(remaining_lines);
    uint32_t block_size = static_cast<uint32_t>(lines_to_send) * bytes_per_line;

    // DMA 单次限制
    const uint32_t DMA_BYTE_LIMIT = 65535u;
    if (block_size > DMA_BYTE_LIMIT) {
      lines_to_send = static_cast<uint16_t>(DMA_BYTE_LIMIT / bytes_per_line);
      if (lines_to_send == 0)
        lines_to_send = 1;
      block_size = static_cast<uint32_t>(lines_to_send) * bytes_per_line;
    }

    // 清 cache：对齐到 32 字节
    clean_dcache_for_range(current_data, block_size);

    // 启动 DMA
    // 清空之前的信号量状态（确保不会误接收旧信号）
    if (dma_done_semaphore_ != nullptr) {
      xSemaphoreTake(dma_done_semaphore_, 0); // non-blocking clear
    }

    HAL_SPI_Transmit_DMA(&hspi2, const_cast<uint8_t*>(current_data), block_size);

    // 等待 DMA 完成（使用信号量替代 busy-wait）
    if (dma_done_semaphore_ != nullptr) {
      // 阻塞直到 DMA 完成（和原来行为等价，但更高效）
      xSemaphoreTake(dma_done_semaphore_, portMAX_DELAY);
    } else {
      // 退回到原来的轮询方式，作为回退
      while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) {
        DelayMs(1);
      }
    }

    // 更新指针、计数
    current_data += block_size;
    remaining_lines -= lines_to_send;
    current_line += lines_to_send;

    if (remaining_lines > 0) {
      const uint32_t new_linear_address = canvas_base + ((static_cast<uint32_t>(y) + current_line) * LCD_WIDTH + x) * 2u;
      Goto_Linear_Addr(new_linear_address);
      LCD_CmdWrite(0x04);
      SPI_CS_choosed();
      SPI2_ReadWriteByte(0x80);
    }
  }
}

void LCDTask::Run() {
  InitializeDisplay();

  uint16_t color = 0x0000;
  uint16_t current_row = 0;
  constexpr uint16_t total_rows = LCD_HEIGHT / ROW_HEIGHT;

  while (true) {
    DrawBitmapDMA(CANVAS_BASE, 0, current_row * ROW_HEIGHT,
                  LCD_WIDTH, ROW_HEIGHT,
                  reinterpret_cast<const uint8_t*>(test_pic_buffer_));

    current_row = (current_row + 1) % total_rows;

    if (current_row == 0) {
      color += 0b0010000010000100; // 更新颜色值
      UpdateTestPictureBuffer(color);

      // Invalidate cache range before BTE copy (aligned)
      invalidate_dcache_for_range(test_pic_buffer_, LCD_WIDTH * ROW_HEIGHT * 2u);

      LT768_BTE_Memory_Copy(CANVAS_BASE, LCD_WIDTH, 0, 0, 0, LCD_WIDTH,
                            0, 0, 0, LCD_WIDTH, 0, 0, 0b1100, LCD_WIDTH, LCD_HEIGHT);
    }
  }
}
