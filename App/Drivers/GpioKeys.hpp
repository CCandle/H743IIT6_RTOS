#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "Drivers/Interfaces.hpp"
#include "main.h"
#include "stm32h7xx_hal.h"

/**
 * @brief 基于 GPIO 的多按键输入设备，适配 LVGL keypad。
 *
 * 用户通过 ConfigureKey 配置最多 N 个按键的端口/引脚/键值和触发电平。
 */
class GpioKeys : public IKeyDevice {
public:
  static constexpr std::size_t kMaxKeys = 6;

  struct KeyConfig {
    GPIO_TypeDef* port = nullptr;
    uint16_t pin = 0;
    uint32_t lvgl_key = 0; // 参考 lv_key_t
    bool active_low = true;
  };

  struct Timings {
    uint32_t debounce_ms = 20;
    uint32_t long_press_ms = 2000;
    uint32_t repeat_ms = 200;
    uint32_t cancel_ms = 10000;
  };

  bool Init() override { return true; }

  void ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low = true);

  /// 可选：设置消抖/长按时序，默认 20ms/2000ms/200ms/10000ms。
  void SetTimings(const Timings& t) { timings_ = t; }
  const Timings& GetTimings() const { return timings_; }

  /**
   * @brief 读取单个按键当前是否为按下状态。
   */
  bool ReadKeyRaw(std::size_t idx, bool& pressed) const;

  /**
   * @brief 获取按键对应的 LVGL 键值。
   */
  uint32_t GetLvglKey(std::size_t idx) const {
    return (idx < kMaxKeys) ? keys_[idx].lvgl_key : 0;
  }

  /**
   * @brief 轮询一次按键，返回一条事件（若有）。
   * @param now_ms 当前时间戳（毫秒）
   * @return std::optional<KeyEvent> 有事件则包含键值和状态。
   */
  std::optional<KeyEvent> Poll(uint32_t now_ms);

private:
  struct KeyRuntime {
    bool valid = false;
    bool stable_pressed = false;
    bool pending_pressed = false;
    uint32_t last_change_ms = 0;
    uint32_t press_start_ms = 0;
    uint32_t repeat_due_ms = 0;
  };

  std::array<KeyConfig, kMaxKeys> keys_{};
  std::array<KeyRuntime, kMaxKeys> rt_{};
  Timings timings_{};
};
