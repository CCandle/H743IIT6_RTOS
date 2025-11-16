#pragma once
#include "Lib/TaskBase.hpp"
#include "gpio.h"
#include "lt7680.hpp"
#include "lt7680_if.hpp"
#include "main.h"
#include "spi.h"

#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define LCD_NRST_Pin GPIO_PIN_11
#define LCD_NRST_GPIO_Port GPIOD

// void lcd_delay(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

class LCDTask : public RAM_D2Task<LCDTask, 1024> {
public:
  void Run() {
    LT7680_IF iface(&hspi2, LCD_CS_GPIO_Port, LCD_CS_Pin, LCD_NRST_GPIO_Port, LCD_NRST_Pin, delay_ms);
    LT7680 lcd(iface);
    uint8_t status = 0x00;

    status = lcd.LCD_StatusRead();
    lcd.init();
    status = lcd.LCD_StatusRead();
    lcd.pwm1_init(true, 0, 200, 100, 100); // enable backlight as vendor did
    lcd.display_on();

    for (;;) {
      lcd.fill_screen(0xF800); // red
      vTaskDelay(pdMS_TO_TICKS(1000));
      lcd.fill_screen(0x07E0); // green
      vTaskDelay(pdMS_TO_TICKS(1000));
      lcd.fill_screen(0x001F); // blue
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

private:
  static void delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }
};