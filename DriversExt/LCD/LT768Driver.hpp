#pragma once

#include <cstdint>
#include "main.h"

/**
 * @class LT768Driver
 * @brief Thin C++ wrapper around vendor LT768 C driver to initialize panel and stream pixels via SPI DMA.
 */
class LT768Driver {
public:
  /**
   * @brief Initialize LT768 controller and canvas geometry.
   * @param lcd_width  Logical LCD width in pixels.
   * @param lcd_height Logical LCD height in pixels.
   * @param canvas_base Off-screen canvas base address.
   */
  void Init(uint16_t lcd_width, uint16_t lcd_height, uint32_t canvas_base);

  /**
   * @brief Start one line DMA write to LT768 GRAM.
   * @param dest_addr Destination GRAM address in bytes.
   * @param line_ptr  Pointer to line pixel data (RGB565).
   * @param byte_len  Length of the line in bytes.
   * @return HAL status of DMA start.
   */
  HAL_StatusTypeDef StartLineDMA(uint32_t dest_addr, const uint8_t* line_ptr, uint16_t byte_len);
};

