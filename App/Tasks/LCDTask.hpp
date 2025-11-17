#pragma once
#include "Lib/TaskBase.hpp"
#include "gpio.h"
// #include "lt7680.hpp"
// #include "lt7680_if.hpp"
#include "LT768_Lib.h"
#include "main.h"
#include "spi.h"

#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define LCD_NRST_Pin GPIO_PIN_11
#define LCD_NRST_GPIO_Port GPIOD

// void lcd_delay(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

class LCDTask : public RAM_D2Task<LCDTask, 1024> {
public:
  void StartUp_picture(void) {
    Select_Main_Window_24bpp();
    Main_Image_Start_Address(0);
    Main_Image_Width(LCD_XSIZE_TFT);
    Main_Window_Start_XY(0, 0);
    Canvas_Image_Start_address(0);
    Canvas_image_width(LCD_XSIZE_TFT);
    Active_Window_XY(0, 0);
    Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
  }

  void Load_Drow_Dialog(void) // 从左到右  从上到下
  {
    LT768_DrawSquare_Fill(0, 0, 480, 270, color65k_white);
    Active_Window_XY(0, 30);
    Active_Window_WH(4000, 1200);
    LT768_DrawSquare_Fill(120, 50, 250, 200, color65k_red);
  }

  void Run() {
    Parallel_Init();
    LT768_Init();

    vTaskDelay(pdMS_TO_TICKS(300));

    Display_ON();
    StartUp_picture();

    vTaskDelay(pdMS_TO_TICKS(300));

    LT768_PWM1_Init(1, 0, 50, 100, 100);

    // Load_Drow_Dialog();
    delay_ms(1000);
    LT768_Select_Internal_Font_Init(24, 2, 2, 1, 1);

    while (1) {
      // Fill the screen with red, green and blue with rectangular function
      LT768_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, color16M_red);
      delay_ms(1000);

      // LT768_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, color16M_green);
      // delay_ms(1000);

      // LT768_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, color16M_blue);
      // delay_ms(1000);

      LT768_DrawCircleSquare(200, 30, 500, 20, 10, 10, color16M_blue);
      delay_ms(1000);

      // LT768_DrawLine(20,20,50,50,color16M_yellow);
      // delay_ms(1000);

      LT768_Print_Internal_Font_String(400, 200, color16M_black, color16M_white, (char*)"Hello LT768!");
      delay_ms(1000);
    }
  }

private:
  static void delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }
};