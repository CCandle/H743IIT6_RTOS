#pragma once

#include "Drivers/Interfaces.hpp"
#include "OS/TaskBase.hpp"
#include "lvgl.h"
#include "semphr.h"
#include "main.h"

/**
 * @brief 按键输入任务：依赖 KeyDriver 产生事件，持锁调用 LVGL。
 */
class KeyInputTask : public RAM_D2Task<KeyInputTask, 1024> {
public:
  void Run();
  void inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex);

private:
  static void key_read(lv_indev_t* indev, lv_indev_data_t* data);
  static KeyEvent latest_key_evt_;
  static bool has_key_evt_;
  static SemaphoreHandle_t lvgl_ready_sem_;
  static SemaphoreHandle_t lvgl_mutex_;
};
