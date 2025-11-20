#include "GpioKeys.hpp"

void GpioKeys::ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low) {
  if (idx >= kMaxKeys)
    return;
  keys_[idx].port = port;
  keys_[idx].pin = pin;
  keys_[idx].lvgl_key = lvgl_key;
  keys_[idx].active_low = active_low;
}

bool GpioKeys::ReadKeyRaw(std::size_t idx, bool& pressed) const {
  if (idx >= kMaxKeys)
    return false;
  const auto& cfg = keys_[idx];
  if (cfg.port == nullptr || cfg.pin == 0 || cfg.lvgl_key == 0)
    return false;

  GPIO_PinState raw = HAL_GPIO_ReadPin(cfg.port, cfg.pin);
  pressed = cfg.active_low ? (raw == GPIO_PIN_RESET) : (raw == GPIO_PIN_SET);
  return true;
}

std::optional<KeyEvent> GpioKeys::Poll(uint32_t now_ms) {
  for (std::size_t i = 0; i < kMaxKeys; ++i) {
    bool raw_pressed = false;
    bool valid = ReadKeyRaw(i, raw_pressed);
    auto& st = rt_[i];
    if (!valid) {
      st = KeyRuntime{};
      continue;
    }
    if (!st.valid) {
      st.valid = true;
      st.stable_pressed = raw_pressed;
      st.pending_pressed = raw_pressed;
      st.last_change_ms = now_ms;
      continue;
    }

    if (raw_pressed != st.pending_pressed) {
      st.pending_pressed = raw_pressed;
      st.last_change_ms = now_ms;
    }

    if (st.pending_pressed != st.stable_pressed &&
        (now_ms - st.last_change_ms) >= timings_.debounce_ms) {
      st.stable_pressed = st.pending_pressed;
      KeyEvent evt;
      evt.key = GetLvglKey(i);
      if (st.stable_pressed) {
        st.press_start_ms = now_ms;
        st.repeat_due_ms = now_ms + timings_.long_press_ms;
        evt.state = KeyState::Press;
      } else {
        uint32_t held = now_ms - st.press_start_ms;
        if (timings_.cancel_ms > 0 && held >= timings_.cancel_ms) {
          continue;
        }
        evt.state = (held >= timings_.long_press_ms) ? KeyState::LongRelease : KeyState::Release;
      }
      return evt;
    }

    if (st.stable_pressed && st.repeat_due_ms != 0 && now_ms >= st.repeat_due_ms) {
      uint32_t held = now_ms - st.press_start_ms;
      if (timings_.cancel_ms > 0 && held >= timings_.cancel_ms) {
        st.repeat_due_ms = 0;
        continue;
      }
      st.repeat_due_ms = now_ms + timings_.repeat_ms;
      KeyEvent evt;
      evt.key = GetLvglKey(i);
      evt.state = KeyState::LongRepeat;
      return evt;
    }
  }

  return std::nullopt;
}
