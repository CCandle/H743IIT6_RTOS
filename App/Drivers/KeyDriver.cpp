#include "KeyDriver.hpp"

#include "task.h"

void KeyDriver::Init() {
  if (exti_sem_ == nullptr) {
    exti_sem_ = xSemaphoreCreateBinary();
  }
}

void KeyDriver::ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low) {
  keys_.ConfigureKey(idx, port, pin, lvgl_key, active_low);
}

void KeyDriver::SetTimings(const GpioKeys::Timings& t) {
  keys_.SetTimings(t);
}

void KeyDriver::OnExtiFromISR(BaseType_t* hpw) {
  if (exti_sem_ != nullptr) {
    xSemaphoreGiveFromISR(exti_sem_, hpw);
  }
}

bool KeyDriver::WaitEvent(KeyEvent& evt, TickType_t wait_ticks) {
  TickType_t wait = wait_ticks;
  if (pressed_ && next_repeat_ms_ != 0) {
    uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (next_repeat_ms_ > now_ms) {
      uint32_t diff_ms = next_repeat_ms_ - now_ms;
      TickType_t diff_ticks = pdMS_TO_TICKS(diff_ms);
      if (diff_ticks < wait)
        wait = diff_ticks;
    } else {
      wait = 0;
    }
  }

  if (exti_sem_ != nullptr && xSemaphoreTake(exti_sem_, wait) == pdTRUE) {
    vTaskDelay(pdMS_TO_TICKS(keys_.GetTimings().debounce_ms));
    bool pressed = false;
    if (keys_.ReadKeyRaw(0, pressed)) {
      uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
      if (!pressed_ && pressed) {
        pressed_ = true;
        press_start_ms_ = now_ms;
        next_repeat_ms_ = now_ms + keys_.GetTimings().long_press_ms;
        repeat_count_ = 0;
        cancel_ = false;
        evt = {keys_.GetLvglKey(0), KeyState::Press};
        return true;
      }
      if (pressed_ && !pressed) {
        pressed_ = false;
        next_repeat_ms_ = 0;
        uint32_t held = now_ms - press_start_ms_;
        if (keys_.GetTimings().cancel_ms > 0 && held >= keys_.GetTimings().cancel_ms) {
          evt = {keys_.GetLvglKey(0), KeyState::Cancel};
        } else if (held >= keys_.GetTimings().long_press_ms) {
          evt = {keys_.GetLvglKey(0), KeyState::LongRelease};
        } else {
          evt = {keys_.GetLvglKey(0), KeyState::Release};
        }
        repeat_count_ = 0;
        cancel_ = false;
        return true;
      }
    }
  }

  if (pressed_ && next_repeat_ms_ != 0) {
    uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t held = now_ms - press_start_ms_;
    if (keys_.GetTimings().cancel_ms > 0 && held >= keys_.GetTimings().cancel_ms) {
      cancel_ = true;
      next_repeat_ms_ = 0;
      evt = {keys_.GetLvglKey(0), KeyState::Cancel};
      return true;
    }
    if (!cancel_ && now_ms >= next_repeat_ms_) {
      next_repeat_ms_ = now_ms + keys_.GetTimings().repeat_ms;
      repeat_count_++;
      evt = {keys_.GetLvglKey(0), KeyState::LongRepeat};
      return true;
    }
  }

  return false;
}
