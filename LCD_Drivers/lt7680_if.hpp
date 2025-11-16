#pragma once
// lt7680_if.hpp - HAL SPI / GPIO small wrapper (injectable delay)
/*
  Designed for STM32H7 HAL. Uses blocking HAL_SPI_Transmit/HAL_SPI_TransmitReceive.
  Delay function is injected so the same class works for baremetal (HAL_Delay)
  or FreeRTOS (vTaskDelay wrapper).
*/
#include "spi.h"
#include "gpio.h"
#include <cstdint>
#include <functional>

class LT7680_IF {
public:
    using delay_fn = std::function<void(uint32_t)>;

    LT7680_IF(SPI_HandleTypeDef* hspi,
              GPIO_TypeDef* cs_port, uint16_t cs_pin,
              GPIO_TypeDef* rst_port, uint16_t rst_pin,
              delay_fn delay);

    // low-level protocol (as vendor AP-note defines)
    void cmd_write(uint8_t cmd);         // send command (0x00 header then cmd)
    void data_write(uint8_t data);       // send byte data (0x80 header then data)
    void pixel_write16(uint16_t px);     // send 16-bit pixel (two data writes per vendor)
    uint8_t status_read();               // header 0x40 then read
    uint16_t data_read16();              // header 0xC0 then read

    void hw_reset();                     // toggles RST with injected delay
    void delay_ms(uint32_t ms) { delay_(ms); }

private:
    SPI_HandleTypeDef* hspi_;
    GPIO_TypeDef* cs_port_; uint16_t cs_pin_;
    GPIO_TypeDef* rst_port_; uint16_t rst_pin_;
    delay_fn delay_;

    void cs_low()  { HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_RESET); }
    void cs_high() { HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_SET);   }
};
