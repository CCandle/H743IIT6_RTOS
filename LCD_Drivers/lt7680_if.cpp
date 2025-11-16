// lt7680_if.cpp
#include "lt7680_if.hpp"
#include <cstring>

LT7680_IF::LT7680_IF(SPI_HandleTypeDef* hspi,
                     GPIO_TypeDef* cs_port, uint16_t cs_pin,
                     GPIO_TypeDef* rst_port, uint16_t rst_pin,
                     delay_fn delay)
    : hspi_(hspi),
      cs_port_(cs_port), cs_pin_(cs_pin),
      rst_port_(rst_port), rst_pin_(rst_pin),
      delay_(delay)
{
    // ensure CS high idle
    HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_SET);
}

void LT7680_IF::hw_reset() {
    HAL_GPIO_WritePin(rst_port_, rst_pin_, GPIO_PIN_RESET);
    delay_(100);
    HAL_GPIO_WritePin(rst_port_, rst_pin_, GPIO_PIN_SET);
    delay_(100);
}

// Note: vendor protocol wraps each command/data with a one-byte header:
// 0x00 = command, 0x80 = data, 0x40 = status read, 0xC0 = data read
void LT7680_IF::cmd_write(uint8_t cmd) {
    uint8_t hdr = 0x00;
    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hspi_, &cmd, 1, HAL_MAX_DELAY);
    cs_high();
}

void LT7680_IF::data_write(uint8_t data) {
    uint8_t hdr = 0x80;
    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hspi_, &data, 1, HAL_MAX_DELAY);
    cs_high();
}

void LT7680_IF::pixel_write16(uint16_t px) {
    // vendor sends 8-bit low then high each preceded by 0x80 header (in their C code).
    // We'll follow exactly their sequence to be safe.
    uint8_t hdr = 0x80;
    uint8_t lo = (uint8_t)(px & 0xFF);
    uint8_t hi = (uint8_t)((px >> 8) & 0xFF);

    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hspi_, &lo, 1, HAL_MAX_DELAY);
    cs_high();

    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hspi_, &hi, 1, HAL_MAX_DELAY);
    cs_high();
}

uint8_t LT7680_IF::status_read() {
    uint8_t hdr = 0x40;
    uint8_t rx = 0xFF;
    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_TransmitReceive(hspi_, (uint8_t*)"\xFF", &rx, 1, HAL_MAX_DELAY);
    cs_high();
    return rx;
}

uint16_t LT7680_IF::data_read16() {
    uint8_t hdr = 0xC0;
    uint8_t r = 0;
    cs_low();
    HAL_SPI_Transmit(hspi_, &hdr, 1, HAL_MAX_DELAY);
    HAL_SPI_TransmitReceive(hspi_, (uint8_t*)"\xFF", &r, 1, HAL_MAX_DELAY);
    cs_high();
    // vendor code tends to return u16 = r (single byte). Keep semantics consistent:
    return (uint16_t)r;
}
