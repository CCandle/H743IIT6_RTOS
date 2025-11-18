#pragma once

#include "FreeRTOS.h"
#include "LT768_Lib.h"
#include "Lib/TaskBase.hpp"
#include "main.h"
#include "semphr.h"
#include "spi.h"
#include <cstdint>

/**
 * @class LCDTask
 * @brief LCD显示任务类，负责管理LT7680液晶显示器的初始化和图像显示
 */
class LCDTask : public RAM_D2Task<LCDTask, 1024> {
public:
  void Run();

private:
  static constexpr uint16_t LCD_WIDTH = 800;
  static constexpr uint16_t LCD_HEIGHT = 480;
  static constexpr uint16_t ROW_HEIGHT = 96;
  static constexpr uint32_t CANVAS_BASE = 800 * 480 * 8;

  // 对齐到32字节的测试缓冲区（AXI段）
  __attribute__((section(".axi.data.testPic2"), aligned(32))) static inline uint16_t test_pic_buffer_[LCD_WIDTH * ROW_HEIGHT];

  // DMA 完成信号量（用于替代轮询）
  static inline SemaphoreHandle_t dma_done_semaphore_ = nullptr;

  void DrawBitmapDMA(uint32_t canvas_base, uint16_t x, uint16_t y,
                     uint16_t width, uint16_t height, const uint8_t* bitmap_data);

  static void DelayMs(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }

  static void WritePixelViaSPI(uint16_t pixel) {
    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);
    SPI2_ReadWriteByte(static_cast<uint8_t>(pixel & 0xFF));
    SPI_CS_chooseless();

    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);
    SPI2_ReadWriteByte(static_cast<uint8_t>(pixel >> 8));
    SPI_CS_chooseless();
  }

  void InitializeDisplay();
  void UpdateTestPictureBuffer(uint16_t color);

  // cache helper: clean (for DMA from memory) and invalidate (for CPU read)
  static void clean_dcache_for_range(const void* ptr, uint32_t len);
  static void invalidate_dcache_for_range(const void* ptr, uint32_t len);

  // friend ISR accessor
  friend void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi);
};
