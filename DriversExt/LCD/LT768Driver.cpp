#include "DriversExt/LCD/LT768Driver.hpp"
#include "LT768_Lib.h"
#include "if_port.h"
#include "spi.h"

void LT768Driver::Init(uint16_t lcd_width, uint16_t lcd_height, uint32_t canvas_base) {
  Parallel_Init();
  LT768_Init();
  HAL_Delay(100);

  Display_ON();

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);
  Main_Image_Width(lcd_width);
  Main_Window_Start_XY(0, 0);

  Active_Window_XY(0, 0);
  Active_Window_WH(lcd_width, lcd_height);

  HAL_Delay(100);

  LT768_PWM1_Init(1, 0, 50, 100, 100);
  HAL_Delay(100);

  Canvas_Image_Start_address(canvas_base);
  Canvas_image_width(lcd_width);
}

HAL_StatusTypeDef LT768Driver::StartLineDMA(uint32_t dest_addr, const uint8_t* line_ptr, uint16_t byte_len) {
  if (line_ptr == nullptr || byte_len == 0) {
    return HAL_ERROR;
  }

  Goto_Linear_Addr(dest_addr);
  LCD_CmdWrite(0x04);

  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);

  return HAL_SPI_Transmit_DMA(&hspi2, const_cast<uint8_t*>(line_ptr), byte_len);
}

