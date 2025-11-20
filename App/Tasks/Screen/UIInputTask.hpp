#pragma once
#include "Drivers/Interfaces.hpp"
#include "Lib/TaskBase.hpp"
#include "lvgl.h"
#include "semphr.h"

class UIInputTask : public RAM_D2Task<UIInputTask, 1024> {
public:
  void Run();
  static SemaphoreHandle_t GetSemaphore();
  void inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex);

private:
  static void touch_read(lv_indev_t* indev, lv_indev_data_t* data);
  static void key_read(lv_indev_t* indev, lv_indev_data_t* data);
  static SemaphoreHandle_t gt_sem_;
  static TouchEvent latest_evt_;
  static bool has_evt_;

  static KeyEvent latest_key_evt_;
  static bool has_key_evt_;
  static SemaphoreHandle_t lvgl_ready_sem_;
  static SemaphoreHandle_t lvgl_mutex_;

  // 配置参数
  static constexpr uint32_t DEBOUNCE_MS = 20;
  static constexpr uint32_t LONG_PRESS_MS = 2000;
  static constexpr uint32_t REPEAT_MS = 200;      // 长按连续触发间隔
  static constexpr uint32_t CANCEL_MS = 10000;    // 超过则取消（例如电源键安全）
};
