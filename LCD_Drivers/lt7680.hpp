#pragma once
#include "lt7680_if.hpp"
#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

class LT7680 {
public:
    struct PanelTiming {
        uint16_t hbp;
        uint16_t hfp;
        uint16_t hspw;
        uint16_t vbp;
        uint16_t vfp;
        uint16_t vspw;
        uint16_t width;
        uint16_t height;

        PanelTiming(uint16_t w=800, uint16_t h=480)
            : hbp(140), hfp(160), hspw(20),
              vbp(20),  vfp(12),  vspw(3),
              width(w), height(h) {}
    };

    LT7680(LT7680_IF& io, const PanelTiming& timing = PanelTiming());

    void init();
    void display_on();
    void display_off();
    void pwm1_init(bool on_off, uint8_t clock_div, uint8_t prescalar,
                   uint16_t count_buffer, uint16_t compare_buffer);
    void fill_screen(uint16_t rgb565);

    void LCD_CmdWrite(uint8_t cmd);
    void LCD_DataWrite(uint8_t data);
    void LCD_DataWrite_Pixel(uint16_t data);
    uint8_t LCD_StatusRead();
    uint16_t LCD_DataRead();

private:
    LT7680_IF& io_;
    PanelTiming timing_;

    void System_Check_Temp();
    void LT768_PLL_Initial();
    void LT768_SDRAM_initail(uint8_t mclk);
    void Set_LCD_Panel();
    void LT768_initial();

    void Foreground_color_65k(uint16_t color);
    void Square_Start_XY(uint16_t x,uint16_t y);
    void Square_End_XY(uint16_t x,uint16_t y);
    void Start_Square_Fill();
    void Check_2D_Busy();

    void Delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
};
