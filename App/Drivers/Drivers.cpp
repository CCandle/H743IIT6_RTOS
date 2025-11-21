#include "Drivers.hpp"

#include "gpio.h"
#include "i2c.h"

namespace Drivers {
namespace {
GpioKeys& KeyDevice() {
  static GpioKeys keys;
  return keys;
}
KeyDriver& KeyDrv() {
  static KeyDriver driver;
  return driver;
}
} // namespace

/**
 * @brief 返回主触摸设备（单例），便于未来替换具体实现。
 */
ITouchDevice& GetPrimaryTouch() {
  static GT1151Q touch_controller(
      &hi2c4,
      CTP_RST_GPIO_Port, CTP_RST_Pin,
      CTP_INT_GPIO_Port, CTP_INT_Pin);
  return touch_controller;
}

void init() {
  ITouchDevice& touch = GetPrimaryTouch();
  touch.Init();
}

IKeyDevice& GetPrimaryKeys() {
  return KeyDevice();
}

KeyDriver& GetKeyDriver() {
  return KeyDrv();
}

void ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low) {
  KeyDevice().ConfigureKey(idx, port, pin, lvgl_key, active_low);
}

} // namespace Drivers
