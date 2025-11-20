#pragma once

#include <cstdint>

#include "Drivers/Interfaces.hpp"
#include "stm32h7xx_hal.h"

class GT1151Q : public ITouchDevice {
public:
  GT1151Q(I2C_HandleTypeDef* hi2c,
          GPIO_TypeDef* rstPort, uint16_t rstPin,
          GPIO_TypeDef* intPort, uint16_t intPin);

  bool Init() override;      // 上电后调用一次
  void ResetChip() override; // 执行硬件复位
  bool DataReady() override; // 读取状态寄存器 bit7 = 1 ?

  // 读取坐标 → 返回 true 表示本次读取有事件
  bool ReadTouch(TouchEvent& evt) override;

private:
  bool ReadReg(uint16_t reg, uint8_t* data, uint16_t len);
  bool WriteReg(uint16_t reg, const uint8_t* data, uint16_t len);

private:
  I2C_HandleTypeDef* _hi2c;
  GPIO_TypeDef* _rstPort;
  uint16_t _rstPin;
  GPIO_TypeDef* _intPort;
  uint16_t _intPin;

  bool _lastPressed = false;

  // 常量地址
  static constexpr uint8_t I2C_ADDR = 0x14;
  static constexpr uint16_t REG_PRODUCT_ID = 0x8140;
  static constexpr uint16_t REG_STATUS = 0x814E;
  static constexpr uint16_t REG_POINT1 = 0x8150;
};
