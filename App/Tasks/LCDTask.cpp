#include "LCDTask.hpp"
#include "if_port.h"
#include "spi.h"

#define BUF_MAX (800 * 480 * 2) // 最大支持的 DMA buffer size，可根据你字符最大宽高调整

// static uint8_t spi_dma_buf[BUF_MAX]; // 你的 SRAM2 完全能放下，推荐放 SRAM2/D2

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
  if (hspi == &hspi2) {
    SPI_CS_chooseless();
  }
}

void LCDTask::LT7680_DrawBitmap_DMA(uint32_t canvas_base,
                                    uint16_t X1, uint16_t Y1,
                                    uint16_t x_w, uint16_t y_h,
                                    const uint8_t* fdata) {
  uint32_t total_size = x_w * y_h;
  if (total_size == 0)
    return;

  // Goto_Pixel_XY(X1, Y1);
  Goto_Linear_Addr(canvas_base + (Y1 * 800 + X1) * 2);
  LCD_CmdWrite(0x04);
  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);

  uint32_t bytes_per_line = x_w * 2; // 假设每个像素2字节
  const uint16_t MAX_LINES_PER_BLOCK = 40;

  uint32_t remaining_lines = y_h;
  uint32_t current_line = 0;
  const uint8_t* current_data = fdata;

  while (remaining_lines > 0) {
    uint16_t lines_to_send = (remaining_lines > MAX_LINES_PER_BLOCK) ? MAX_LINES_PER_BLOCK : remaining_lines;

    uint32_t block_size = lines_to_send * bytes_per_line;

    if (block_size > 65535) {
      lines_to_send = 65535 / bytes_per_line;
      if (lines_to_send == 0)
        lines_to_send = 1; // 至少传输一行
      block_size = lines_to_send * bytes_per_line;
    }

    SCB_CleanDCache_by_Addr((uint32_t*)current_data, block_size);
    HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)current_data, block_size);
    while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) {
      delay_ms(1);
    }

    current_data += block_size;
    remaining_lines -= lines_to_send;
    current_line += lines_to_send;

    if (remaining_lines > 0) {
      // Goto_Pixel_XY(X1, Y1 + current_line);
      Goto_Linear_Addr(canvas_base + ((Y1 + current_line) * 800 + X1) * 2);
      LCD_CmdWrite(0x04);
      SPI_CS_choosed();
      SPI2_ReadWriteByte(0x80);
    }
  }
}