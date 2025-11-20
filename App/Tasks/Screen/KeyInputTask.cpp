#include "KeyInputTask.hpp"

#include "Drivers/Drivers.hpp"
#include "Test/UI/TestAnimation.hpp"
#include "lvgl.h"
#include "task.h"

KeyEvent KeyInputTask::latest_key_evt_{};
bool KeyInputTask::has_key_evt_ = false;
SemaphoreHandle_t KeyInputTask::lvgl_ready_sem_ = nullptr;
SemaphoreHandle_t KeyInputTask::lvgl_mutex_ = nullptr;

void KeyInputTask::Run() {
  auto& key_drv = Drivers::GetKeyDriver();
  key_drv.Init();
  key_drv.ConfigureKey(0, GPIOI, GPIO_PIN_8, LV_KEY_ENTER, false); // 高电平按下
  key_drv.SetTimings({20, 2000, 200, 10000});

  if (lvgl_ready_sem_ != nullptr) {
    xSemaphoreTake(lvgl_ready_sem_, portMAX_DELAY);
  }

  if (lvgl_mutex_ != nullptr) {
    xSemaphoreTake(lvgl_mutex_, portMAX_DELAY);
  }

  lv_indev_t* keypad = lv_indev_create();
  lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(keypad, key_read);
  lv_indev_set_mode(keypad, LV_INDEV_MODE_EVENT);

  if (lvgl_mutex_ != nullptr) {
    xSemaphoreGive(lvgl_mutex_);
  }

  uint32_t repeat_count = 0;
  for (;;) {
    KeyEvent evt;
    if (key_drv.WaitEvent(evt, portMAX_DELAY)) {
      latest_key_evt_ = evt;
      has_key_evt_ = true;
      if (evt.state == KeyState::LongRepeat) {
        repeat_count++;
      } else {
        repeat_count = 0;
      }

      while (has_key_evt_) {
        if (lvgl_mutex_ == nullptr || xSemaphoreTake(lvgl_mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
          lv_indev_read(keypad);

          const char* msg = "unknown";
          switch (evt.state) {
          case KeyState::Press:
            msg = "short";
            break;
          case KeyState::Release:
            msg = "release";
            break;
          case KeyState::LongRelease:
            msg = "long";
            break;
          case KeyState::LongRepeat:
            msg = "repeat";
            break;
          case KeyState::Cancel:
            msg = "cancel";
            break;
          default:
            break;
          }
          UpdateKeyStatus(msg, repeat_count);

          has_key_evt_ = false;
          if (lvgl_mutex_ != nullptr) {
            xSemaphoreGive(lvgl_mutex_);
          }
        } else {
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      }
    }
  }
}

void KeyInputTask::key_read(lv_indev_t* indev, lv_indev_data_t* data) {
  (void)indev;
  if (!has_key_evt_) {
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = 0;
    return;
  }

  const KeyEvent evt = latest_key_evt_;
  data->key = evt.key;
  if (evt.state == KeyState::Press || evt.state == KeyState::LongRepeat) {
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void KeyInputTask::inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex) {
  lvgl_ready_sem_ = lvgl_ready_sem;
  lvgl_mutex_ = lvgl_mutex;
}
