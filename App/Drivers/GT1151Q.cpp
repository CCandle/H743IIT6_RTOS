#include "GT1151Q.hpp"
#include <cstring>

GT1151Q::GT1151Q(I2C_HandleTypeDef* hi2c,
                 GPIO_TypeDef* rstPort, uint16_t rstPin,
                 GPIO_TypeDef* intPort, uint16_t intPin)
    : _hi2c(hi2c), _rstPort(rstPort), _rstPin(rstPin),
      _intPort(intPort), _intPin(intPin) {
}

// ------------------ Reset ------------------

void GT1151Q::ResetChip() {
  // RST Low ≥100us，这里更宽裕一点
  HAL_GPIO_WritePin(_rstPort, _rstPin, GPIO_PIN_RESET);
  HAL_Delay(10);

  // RST High → 等待 FW 初始化
  HAL_GPIO_WritePin(_rstPort, _rstPin, GPIO_PIN_SET);
  HAL_Delay(60);
}

// ------------------ Init ------------------

bool GT1151Q::Init() {
  ResetChip();

  // 读取 Product ID
  uint8_t id[4] = {0};
  if (!ReadReg(REG_PRODUCT_ID, id, 4)) {
    return false;
  }

  // 清除状态寄存器
  uint8_t zero = 0;
  WriteReg(REG_STATUS, &zero, 1);

  _lastPressed = false;
  return true;
}

// ------------------ HAL I2C 底层 ------------------

bool GT1151Q::WriteReg(uint16_t reg, const uint8_t* data, uint16_t len) {
  uint8_t buf[2 + 16];
  if (len > 16)
    return false;

  buf[0] = reg >> 8;
  buf[1] = reg & 0xFF;
  memcpy(&buf[2], data, len);

  HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(
      _hi2c, I2C_ADDR << 1, buf, len + 2, 20);

  return st == HAL_OK;
}

bool GT1151Q::ReadReg(uint16_t reg, uint8_t* data, uint16_t len) {
  uint8_t a[2] = {uint8_t(reg >> 8), uint8_t(reg & 0xFF)};

  if (HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, a, 2, 20) != HAL_OK)
    return false;

  if (HAL_I2C_Master_Receive(_hi2c, (I2C_ADDR << 1) | 1, data, len, 20) != HAL_OK)
    return false;

  return true;
}

// ------------------ 状态检测 ------------------

bool GT1151Q::DataReady() {
  uint8_t status = 0;
  if (!ReadReg(REG_STATUS, &status, 1))
    return false;

  return (status & 0x80) != 0; // bit7 = buffer status
}

// ------------------ 读取触摸事件 ------------------

bool GT1151Q::ReadTouch(TouchEvent& evt) {
  uint8_t status = 0;
  if (!ReadReg(REG_STATUS, &status, 1))
    return false;

  bool bufferReady = status & 0x80;
  uint8_t touchCount = status & 0x0F;

  // 清除状态位
  uint8_t zero = 0;
  WriteReg(REG_STATUS, &zero, 1);

  if (!bufferReady) {
    evt.state = TouchState::None;
    return false;
  }

  // ----------- 无触点 → Release 事件 -----------
  if (touchCount == 0) {
    if (_lastPressed) {
      evt.state = TouchState::Release;
      evt.x = evt.y = 0;
    } else {
      evt.state = TouchState::None;
    }
    _lastPressed = false;
    return true;
  }

  // ----------- 有触点 → Press 或 Move -----------
  uint8_t buf[8];
  if (!ReadReg(REG_POINT1, buf, 8))
    return false;

  uint16_t x = buf[0] | (buf[1] << 8);
  uint16_t y = buf[2] | (buf[3] << 8);

  evt.x = x;
  evt.y = y;

  if (!_lastPressed)
    evt.state = TouchState::Press;
  else
    evt.state = TouchState::Move;

  _lastPressed = true;
  return true;
}