#pragma once

#include "Drivers/Interfaces.hpp"
#include "Lib/TaskBase.hpp"
#include "lvgl.h"
#include "semphr.h"

/**
 * @brief 触摸输入任务：等待中断，读取触摸事件并推送给 LVGL。
 */
class TouchInputTask : public RAM_D2Task<TouchInputTask, 1024> {
public:
  void Run();
  static SemaphoreHandle_t GetSemaphore();

  /**
   * @brief 注入 LVGL 同步对象。
   * @param lvgl_ready_sem LVGL 初始化完成信号。
   * @param lvgl_mutex LVGL 互斥锁。
   */
  void inject(SemaphoreHandle_t lvgl_ready_sem, SemaphoreHandle_t lvgl_mutex);

private:
  static void touch_read(lv_indev_t* indev, lv_indev_data_t* data);

  static SemaphoreHandle_t gt_sem_;
  static TouchEvent latest_evt_;
  static bool has_evt_;
  static SemaphoreHandle_t lvgl_ready_sem_;
  static SemaphoreHandle_t lvgl_mutex_;
};
