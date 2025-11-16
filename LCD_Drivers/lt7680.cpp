#include "lt7680.hpp"
#include <cstdio>

// ---- FIXED: Full bit mask set ----
static constexpr uint8_t cSetb0 = 0x01;
static constexpr uint8_t cSetb1 = 0x02;
static constexpr uint8_t cSetb2 = 0x04;
static constexpr uint8_t cSetb3 = 0x08;
static constexpr uint8_t cSetb4 = 0x10;
static constexpr uint8_t cSetb5 = 0x20;
static constexpr uint8_t cSetb6 = 0x40;
static constexpr uint8_t cSetb7 = 0x80;

static constexpr uint8_t cClrb0 = 0xFE;
static constexpr uint8_t cClrb1 = 0xFD;
static constexpr uint8_t cClrb2 = 0xFB;
static constexpr uint8_t cClrb3 = 0xF7;
static constexpr uint8_t cClrb4 = 0xEF;
static constexpr uint8_t cClrb5 = 0xDF;
static constexpr uint8_t cClrb6 = 0xBF;
static constexpr uint8_t cClrb7 = 0x7F;

// -----------------------------------------------------

LT7680::LT7680(LT7680_IF& io, const PanelTiming& timing)
    : io_(io), timing_(timing) {}

// === low-level wrappers ===
void LT7680::LCD_CmdWrite(uint8_t cmd) { io_.cmd_write(cmd); }
void LT7680::LCD_DataWrite(uint8_t data) { io_.data_write(data); }
void LT7680::LCD_DataWrite_Pixel(uint16_t d) { io_.pixel_write16(d); }
uint8_t LT7680::LCD_StatusRead() { return io_.status_read(); }
uint16_t LT7680::LCD_DataRead() { return io_.data_read16(); }

// ======================== System_Check_Temp ======================
void LT7680::System_Check_Temp() {
  uint8_t i = 0;
  uint8_t system_ok = 0;

  do {
    if ((LCD_StatusRead() & 0x02) == 0x00) {
      Delay_ms(1);
      LCD_CmdWrite(0x01);
      Delay_ms(1);
      uint8_t temp = LCD_DataRead();
      if ((temp & 0x80) == 0x80) {
        system_ok = 1;
        i = 0;
      } else {
        Delay_ms(1);
        LCD_CmdWrite(0x01);
        Delay_ms(1);
        LCD_DataWrite(0x80);
      }
    } else {
      system_ok = 0;
      i++;
    }

    if (system_ok == 0 && i == 5) {
      io_.hw_reset();
      i = 0;
    }

  } while (!system_ok);
}

// ======================== PLL Initial ======================
void LT7680::LT768_PLL_Initial() {
  uint32_t temp = (timing_.hbp + timing_.hfp + timing_.hspw + timing_.width) * (timing_.vbp + timing_.vfp + timing_.vspw + timing_.height) * 60;

  uint32_t t1 = (temp % 1000000) / 100000;
  if (t1 > 5)
    temp = temp / 1000000 + 1;
  else
    temp /= 1000000;

  uint8_t SCLK = temp;
  uint8_t CCLK = 80;
  uint8_t MCLK = 80;
  if (SCLK > 65)
    SCLK = 65;

  // 10MHz crystal branch
  uint16_t lpllOD_sclk = 3, lpllOD_cclk = 2, lpllOD_mclk = 2;
  uint16_t lpllR_sclk = 5, lpllR_cclk = 5, lpllR_mclk = 5;
  uint16_t lpllN_sclk = 2 * SCLK;
  uint16_t lpllN_mclk = MCLK;
  uint16_t lpllN_cclk = CCLK;

  LCD_CmdWrite(0x05);
  LCD_DataWrite((lpllOD_sclk << 6) | (lpllR_sclk << 1) | ((lpllN_sclk >> 8) & 1));
  LCD_CmdWrite(0x07);
  LCD_DataWrite((lpllOD_mclk << 6) | (lpllR_mclk << 1) | ((lpllN_mclk >> 8) & 1));
  LCD_CmdWrite(0x09);
  LCD_DataWrite((lpllOD_cclk << 6) | (lpllR_cclk << 1) | ((lpllN_cclk >> 8) & 1));

  LCD_CmdWrite(0x06);
  LCD_DataWrite(lpllN_sclk);
  LCD_CmdWrite(0x08);
  LCD_DataWrite(lpllN_mclk);
  LCD_CmdWrite(0x0A);
  LCD_DataWrite(lpllN_cclk);

  LCD_CmdWrite(0x00);
  Delay_ms(1);
  LCD_DataWrite(0x80);
  Delay_ms(1);
}

// ======================== SDRAM Init ======================
void LT7680::LT768_SDRAM_initail(uint8_t mclk) {
  uint16_t sdram_itv = (64000000 / 8192) / (1000 / mclk);
  sdram_itv -= 2;

  LCD_CmdWrite(0xE0);
  LCD_DataWrite(0x20);
  LCD_CmdWrite(0xE1);
  LCD_DataWrite(0x03);
  LCD_CmdWrite(0xE2);
  LCD_DataWrite(sdram_itv & 0xFF);
  LCD_CmdWrite(0xE3);
  LCD_DataWrite(sdram_itv >> 8);
  LCD_CmdWrite(0xE4);
  LCD_DataWrite(0x01);

  while (!(LCD_StatusRead() & 0x04))
    ;
  Delay_ms(1);
}

// ======================== Panel Setup ======================
void LT7680::Set_LCD_Panel() {
  // TFT_16bit
  LCD_CmdWrite(0x01);
  {
    uint8_t t = LCD_DataRead();
    t |= cSetb4;
    t &= cClrb3;
    LCD_DataWrite(t);
  }

  // Host 16bit
  LCD_CmdWrite(0x01);
  {
    uint8_t t = LCD_DataRead();
    t |= cSetb0;
    LCD_DataWrite(t);
  }

  // RGB 16bpp
  LCD_CmdWrite(0x02);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb7;
    t |= cSetb6;
    LCD_DataWrite(t);
  }

  // L->R, T->D
  LCD_CmdWrite(0x02);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb2;
    t &= cClrb1;
    LCD_DataWrite(t);
  }

  // Graphic mode
  LCD_CmdWrite(0x03);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb2;
    LCD_DataWrite(t);
  }

  // SDRAM select
  LCD_CmdWrite(0x03);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb1;
    t &= cClrb0;
    LCD_DataWrite(t);
  }

  // PCLK falling
  LCD_CmdWrite(0x12);
  {
    uint8_t t = LCD_DataRead();
    t |= cSetb7;
    LCD_DataWrite(t);
  }

  // VSCAN top→bottom
  LCD_CmdWrite(0x12);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb3;
    LCD_DataWrite(t);
  }

  // RGB order
  LCD_CmdWrite(0x12);
  {
    uint8_t t = LCD_DataRead();
    t &= 0xF8;
    LCD_DataWrite(t);
  }

  // HSYNC low
  LCD_CmdWrite(0x13);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb7;
    LCD_DataWrite(t);
  }

  // VSYNC low
  LCD_CmdWrite(0x13);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb6;
    LCD_DataWrite(t);
  }

  // DE high
  LCD_CmdWrite(0x13);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb5;
    LCD_DataWrite(t);
  }

  // ---- resolution ----
  uint16_t w = timing_.width;
  uint16_t h = timing_.height;

  uint8_t a = (w / 8) - 1, b = w % 8;
  LCD_CmdWrite(0x14);
  LCD_DataWrite(a);
  LCD_CmdWrite(0x15);
  LCD_DataWrite(b);

  LCD_CmdWrite(0x1A);
  LCD_DataWrite((uint8_t)(h - 1));
  LCD_CmdWrite(0x1B);
  LCD_DataWrite((uint8_t)((h - 1) >> 8));

  // HBPD
  a = (timing_.hbp / 8) - 1;
  b = timing_.hbp % 8;
  LCD_CmdWrite(0x16);
  LCD_DataWrite(a);
  LCD_CmdWrite(0x17);
  LCD_DataWrite(b);

  // HFPD
  a = (timing_.hfp / 8) - 1;
  LCD_CmdWrite(0x18);
  LCD_DataWrite(a);

  // HSPW
  a = (timing_.hspw / 8) - 1;
  LCD_CmdWrite(0x19);
  LCD_DataWrite(a);

  LCD_CmdWrite(0x1C);
  LCD_DataWrite((uint8_t)(timing_.vbp - 1));
  LCD_CmdWrite(0x1D);
  LCD_DataWrite((uint8_t)((timing_.vbp - 1) >> 8));

  LCD_CmdWrite(0x1E);
  LCD_DataWrite((uint8_t)(timing_.vfp - 1));
  LCD_CmdWrite(0x1F);
  LCD_DataWrite((uint8_t)(timing_.vspw - 1));

  // Memory XY, 16bpp
  LCD_CmdWrite(0x5E);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb2;
    LCD_DataWrite(t);
  }
  LCD_CmdWrite(0x5E);
  {
    uint8_t t = LCD_DataRead();
    t &= cClrb1;
    t |= cSetb0;
    LCD_DataWrite(t);
  }
}

void LT7680::LT768_initial() {
  LT768_PLL_Initial();
  LT768_SDRAM_initail(80);
  Set_LCD_Panel();
}

void LT7680::init() {
  Delay_ms(100);
  io_.hw_reset();
  System_Check_Temp();
  Delay_ms(100);
  while (LCD_StatusRead() & 0x02)
    Delay_ms(1);
  LT768_initial();
}

// ================= Drawing ===================
void LT7680::Foreground_color_65k(uint16_t c) {
  LCD_CmdWrite(0xD2);
  LCD_DataWrite(c >> 8);
  LCD_CmdWrite(0xD3);
  LCD_DataWrite(c >> 3);
  LCD_CmdWrite(0xD4);
  LCD_DataWrite(c << 3);
}

void LT7680::Square_Start_XY(uint16_t x, uint16_t y) {
  LCD_CmdWrite(0x68);
  LCD_DataWrite(x);
  LCD_CmdWrite(0x69);
  LCD_DataWrite(x >> 8);
  LCD_CmdWrite(0x6A);
  LCD_DataWrite(y);
  LCD_CmdWrite(0x6B);
  LCD_DataWrite(y >> 8);
}

void LT7680::Square_End_XY(uint16_t x, uint16_t y) {
  LCD_CmdWrite(0x6C);
  LCD_DataWrite(x);
  LCD_CmdWrite(0x6D);
  LCD_DataWrite(x >> 8);
  LCD_CmdWrite(0x6E);
  LCD_DataWrite(y);
  LCD_CmdWrite(0x6F);
  LCD_DataWrite(y >> 8);
}

void LT7680::Start_Square_Fill() {
  LCD_CmdWrite(0x76);
  LCD_DataWrite(0xE0);
  while (LCD_StatusRead() & 0x08)
    ;
}

void LT7680::Check_2D_Busy() {
  while (LCD_StatusRead() & 0x08)
    Delay_ms(1);
}

void LT7680::fill_screen(uint16_t c) {
  Foreground_color_65k(c);
  Square_Start_XY(0, 0);
  Square_End_XY(timing_.width - 1, timing_.height - 1);
  Start_Square_Fill();
  Check_2D_Busy();
}

void LT7680::display_on() {
  LCD_CmdWrite(0x12);
  uint8_t t = LCD_DataRead();
  t |= cSetb6;
  LCD_DataWrite(t);
}

void LT7680::display_off() {
  LCD_CmdWrite(0x12);
  uint8_t t = LCD_DataRead();
  t &= cClrb6;
  LCD_DataWrite(t);
}

// ================= PWM ===================
void LT7680::pwm1_init(bool on, uint8_t, uint8_t pres, uint16_t cnt, uint16_t cmp) {
  LCD_CmdWrite(0x85);
  {
    uint8_t t = LCD_DataRead();
    t |= 0x08;
    t &= ~0x04;
    LCD_DataWrite(t);
  }

  LCD_CmdWrite(0x84);
  LCD_DataWrite(pres - 1);

  LCD_CmdWrite(0x8E);
  LCD_DataWrite(cnt & 0xFF);
  LCD_CmdWrite(0x8F);
  LCD_DataWrite(cnt >> 8);
  LCD_CmdWrite(0x8C);
  LCD_DataWrite(cmp & 0xFF);
  LCD_CmdWrite(0x8D);
  LCD_DataWrite(cmp >> 8);

  if (on) {
    LCD_CmdWrite(0x86);
    uint8_t t = LCD_DataRead();
    t |= 0x10;
    LCD_DataWrite(t);
  }
}
