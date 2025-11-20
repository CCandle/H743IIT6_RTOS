#include "TouchInputTask.hpp"

#include "Drivers/Drivers.hpp"
#include "gpio.h"
#include "lvgl.h"
#include "main.h"
#include "task.h"

SemaphoreHandle_t TouchInputTask::gt_sem_ = nullptr;
TouchEvent TouchInputTask::latest_evt_{};
bool TouchInputTask::has_evt_ = false;
SemaphoreHandle_t TouchInputTask::lvgl_ready_sem_ = nullptr;
SemaphoreHandle_t TouchInputTask::lvgl_mutex_ = nullptr;

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == CTP_INT_Pin) {
    BaseType_t hp = pdFALSE;
    SemaphoreHandle_t sem = TouchInputTask::GetSemaphore();
    if (sem != nullptr) {
      xSemaphoreGiveFromISR(sem, &hp);
    }
    if (hp == pdTRUE) {
      portYIELD_FROM_ISR(hp);
    }
  } else if (GPIO_Pin == GPIO_PIN_8) {
    BaseType_t hpw = pdFALSE;
    Drivers::GetKeyDriver().OnExtiFromISR(&hpw);
    if (hpw == pdTRUE) {
      portYIELD_FROM_ISR(hpw);
    }
  }
}

void TouchInputTask::Run() {
  gt_sem_ = xSemaphoreCreateBinary();
  ITouchDevice& touch = Drivers::GetPrimaryTouch();
  touch.ResetChip();
  touch.Init();

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

  if (lvgl_mutex_ != nullptr) {
    xSemaphoreGive(lvgl_mutex_);
  }

  for (;;) {
    if (xSemaphoreTake(gt_sem_, portMAX_DELAY) == pdTRUE) {
      TouchEvent evt;
      if (touch.ReadTouch(evt)) {
        latest_evt_ = evt;
        has_evt_ = true;
        if (lvgl_mutex_ != nullptr && xSemaphoreTake(lvgl_mutex_, pdMS_TO_TICKS(2)) == pdTRUE) {
          lv_indev_read(pointer);
          xSemaphoreGive(lvgl_mutex_);
        } else if (lvgl_mutex_ == nullptr) {
          lv_indev_read(pointer);
        }
      }
    }
  }
}

void TouchInputTask::touch_read(lv_indev_t* indev, lv_indev_data_t* data) {
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
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  has_evt_ = false;
}

SemaphoreHandle_t TouchInputTask::GetSemaphore() {
  return gt_sem_;
}

void TouchInputTask::inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex) {
  lvgl_ready_sem_ = lvgl_ready_sem;
  lvgl_mutex_ = lvgl_mutex;
}
