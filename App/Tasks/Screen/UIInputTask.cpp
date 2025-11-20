#include "UIInputTask.hpp"

#include "Drivers/Drivers.hpp"
#include "Drivers/GpioKeys.hpp"
#include "gpio.h"
#include "lvgl.h"
#include "main.h"
#include "task.h"

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == CTP_INT_Pin) {
    BaseType_t hp = pdFALSE;
    SemaphoreHandle_t sem = UIInputTask::GetSemaphore();
    if (sem != nullptr) {
      xSemaphoreGiveFromISR(sem, &hp);
    }
    portYIELD_FROM_ISR(hp);
  }
}

SemaphoreHandle_t UIInputTask::gt_sem_ = nullptr;
TouchEvent UIInputTask::latest_evt_{};
bool UIInputTask::has_evt_ = false;
KeyEvent UIInputTask::latest_key_evt_{};
bool UIInputTask::has_key_evt_ = false;
SemaphoreHandle_t UIInputTask::lvgl_ready_sem_ = nullptr;
SemaphoreHandle_t UIInputTask::lvgl_mutex_ = nullptr;

void UIInputTask::Run() {
  gt_sem_ = xSemaphoreCreateBinary();
  ITouchDevice& touch = Drivers::GetPrimaryTouch();
  touch.ResetChip();
  touch.Init();
  IKeyDevice& keys = Drivers::GetPrimaryKeys();
  keys.Init();

  if (lvgl_ready_sem_ != nullptr) {
    xSemaphoreTake(lvgl_ready_sem_, portMAX_DELAY);
  }

  if (lvgl_mutex_ != nullptr) {
    xSemaphoreTake(lvgl_mutex_, portMAX_DELAY);
  }

  lv_indev_t* pointer = lv_indev_create();
  lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(pointer, touch_read);
  lv_indev_set_mode(pointer, LV_INDEV_MODE_EVENT);

  lv_indev_t* keypad = lv_indev_create();
  lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(keypad, key_read);
  lv_indev_set_mode(keypad, LV_INDEV_MODE_EVENT);

  if (lvgl_mutex_ != nullptr) {
    xSemaphoreGive(lvgl_mutex_);
  }

  // 按键扫描状态
  TickType_t last_wake = xTaskGetTickCount();
  auto& key_dev = static_cast<GpioKeys&>(keys);
  key_dev.SetTimings({DEBOUNCE_MS, LONG_PRESS_MS, REPEAT_MS, CANCEL_MS});

  for (;;) {
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));

    if (xSemaphoreTake(gt_sem_, pdMS_TO_TICKS(10)) == pdTRUE) {
      TouchEvent evt;
      if (touch.ReadTouch(evt)) {
        latest_evt_ = evt;
        has_evt_ = true;
        // 通知 LVGL 有新事件（需持有 LVGL 互斥）
        if (lvgl_mutex_ != nullptr && xSemaphoreTake(lvgl_mutex_, pdMS_TO_TICKS(2)) == pdTRUE) {
          lv_indev_read(pointer);
          xSemaphoreGive(lvgl_mutex_);
        } else if (lvgl_mutex_ == nullptr) {
          lv_indev_read(pointer);
        }
      }
    }

    if (auto evt_opt = key_dev.Poll(xTaskGetTickCount() * portTICK_PERIOD_MS)) {
      latest_key_evt_ = *evt_opt;
      has_key_evt_ = true;
    }

    if (has_key_evt_) {
      if (lvgl_mutex_ != nullptr && xSemaphoreTake(lvgl_mutex_, pdMS_TO_TICKS(2)) == pdTRUE) {
        lv_indev_read(keypad);
        xSemaphoreGive(lvgl_mutex_);
      } else if (lvgl_mutex_ == nullptr) {
        lv_indev_read(keypad);
      }
    }
  }
}

void UIInputTask::touch_read(lv_indev_t* indev, lv_indev_data_t* data) {
  (void)indev;
  if (!has_evt_) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  const TouchEvent evt = latest_evt_;
  if (evt.state == TouchState::Press || evt.state == TouchState::Move) {
    data->point.x = evt.x;
    data->point.y = evt.y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else if (evt.state == TouchState::Release) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else if (evt.state == TouchState::None) {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void UIInputTask::key_read(lv_indev_t* indev, lv_indev_data_t* data) {
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
  } else if (evt.state == KeyState::Release || evt.state == KeyState::LongRelease) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  // 事件已消费，等待下一次边沿
  has_key_evt_ = false;
}

SemaphoreHandle_t UIInputTask::GetSemaphore() {
  return gt_sem_;
}

void UIInputTask::inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex) {
  lvgl_ready_sem_ = lvgl_ready_sem;
  lvgl_mutex_ = lvgl_mutex;
}
