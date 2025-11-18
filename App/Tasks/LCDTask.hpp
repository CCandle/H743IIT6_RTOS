#pragma once
#include "Lib/TaskBase.hpp"
#include "gpio.h"
// #include "lt7680.hpp"
// #include "lt7680_if.hpp"
#include "LT768_Lib.h"
// #include "jpg.h"
#include "main.h"
#include "spi.h"

#define ROW 96
__attribute__((section(".axi.data.testPic2"),aligned(32))) static uint16_t testPic2[800 * ROW];

// void lcd_delay(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

class LCDTask : public RAM_D2Task<LCDTask, 1024> {
public:
  void Run() {
    Parallel_Init();
    LT768_Init();
    delay_ms(100);
    Display_ON();

    Select_Main_Window_16bpp();
    Main_Image_Start_Address(0);
    Main_Image_Width(LCD_XSIZE_TFT);
    Main_Window_Start_XY(0, 0);

    Active_Window_XY(0, 0);
    Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);

    delay_ms(100);

    LT768_PWM1_Init(1, 0, 50, 100, 100);

    delay_ms(100);
    LT768_Select_Internal_Font_Init(24, 2, 2, 1, 1);

    uint32_t canvas_base = 800 * 480 * 8;
    Canvas_Image_Start_address(canvas_base);
    Canvas_image_width(LCD_XSIZE_TFT);
    uint32_t dest_base = 0;

    uint16_t color = 0x0000;
    uint16_t row = 0;

    while (1) {
      // BTE_Disable();
      // LT768_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, color65k_blue);
      // LT768_BTE_Memory_Copy(canvas_base, 800, 0, 0, 0, 800, 0, 0, dest_base, 800, 0, 0, 0b1100, 800, 480);
      // delay_ms(1000);

      // BTE_Write_fixed(
      //     canvas_base,   // s0 canvas base (bytes)
      //     LCD_XSIZE_TFT, // s0 width in pixels (canvas stride)
      //     0, 0,          // s0_x, s0_y in canvas
      //     canvas_base,   // destination base byte address
      //     LCD_XSIZE_TFT, // dest width in pixels
      //     200, 120,      // dest window start (ignored when using absolute des_addr; vendor expects base+window, but many code uses base plus window)
      //     0x0C,          // ROP = S0 (copy)
      //     testPic2,
      //     200, 120 // 1width, height of block to write
      // );

      LT7680_DrawBitmap_DMA(canvas_base, 0, row * ROW, 800, ROW, (uint8_t*)testPic2);
      // delay_ms(100);
      // color = 0xffff;
      row += 1;
      row %= 480 / ROW;
      if (!row) {
        color += 0b0010000010000100;
        for (int i = 0; i < 800 * ROW; i++) {
          testPic2[i] = color;
        }
        SCB_InvalidateDCache_by_Addr(testPic2, 800 * ROW * 2);
        LT768_BTE_Memory_Copy(canvas_base, 800, 0, 0, 0, 800, 0, 0, dest_base, 800, 0, 0, 0b1100, 800, 480);
      }
    }
  }

private:
  void LT7680_DrawBitmap_DMA(uint32_t canvas_base,
                             uint16_t X1, uint16_t Y1,
                             uint16_t x_w, uint16_t y_h,
                             const uint8_t* fdata);

  static void delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }

  // Use vendor SPI pixel write helper (use existing SPI_DataWrite_Pixel if available).
  static void write_pixel_via_spi(uint16_t pix) {
    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);
    SPI2_ReadWriteByte((uint8_t)(pix & 0xFF));
    SPI_CS_chooseless();

    SPI_CS_choosed();
    SPI2_ReadWriteByte(0x80);
    SPI2_ReadWriteByte((uint8_t)(pix >> 8));
    SPI_CS_chooseless();
  }

  static void BTE_Write_fixed(uint32_t s0_canvas_base, uint16_t s0_width_pixels, uint16_t s0_x, uint16_t s0_y,
                              uint32_t des_base, uint16_t des_width_pixels, uint16_t des_x, uint16_t des_y,
                              uint8_t rop, const uint16_t* data, uint16_t x_w, uint16_t y_h) {
    uint16_t i, j;

    // 1) Configure S0 (we are writing MCU data into S0)
    BTE_S0_Color_16bpp();                        // S0 color format 16bpp
    BTE_S0_Memory_Start_Address(s0_canvas_base); // S0 memory base (bytes)
    BTE_S0_Image_Width(s0_width_pixels);         // S0 stride in pixels (canvas width)
    BTE_S0_Window_Start_XY(s0_x, s0_y);          // S0 window start

    // 2) Configure Destination (Main window area)
    BTE_Destination_Color_16bpp();                  // Destination format 16bpp
    BTE_Destination_Memory_Start_Address(des_base); // dest base (bytes)
    BTE_Destination_Image_Width(des_width_pixels);  // dest stride in pixels
    BTE_Destination_Window_Start_XY(des_x, des_y);  // dest XY (usually where to place)

    // 3) Window size, ROP and operation code
    BTE_Window_Size(x_w, y_h);
    BTE_ROP_Code(rop & 0x0F); // 4-bit ROP (0x0C = S0 copy)
    BTE_Operation_Code(0x00); // 0x00 = MPU Write BTE with ROP (we will write S0 via MCU)
    BTE_Enable();

    // 4) Write pixel data into S0 via Memory Data Write Port (cmd 0x04)
    LCD_CmdWrite(0x04); // Memory Data Read/Write Port

    // Note: the vendor's LCD_DataWrite_Pixel() sends low byte then high byte per pixel.
    // Use that semantic here (we use write_pixel_via_spi which follows vendor wiring).
    const uint16_t* p = data;
    for (i = 0; i < y_h; ++i) {
      for (j = 0; j < x_w; ++j) {
        Check_Mem_WR_FIFO_not_Full();
        write_pixel_via_spi(*p++);
      }
    }

    Check_Mem_WR_FIFO_Empty();
    Check_BTE_Busy();
    // optional: BTE_Disable();
  }
};