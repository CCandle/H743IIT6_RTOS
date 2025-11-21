#pragma once

#include "FreeRTOS.h"
#include "Drivers/GpioKeys.hpp"
#include "semphr.h"

/**
 * @brief FreeRTOS 友好的按键驱动：消抖、长按/重复判定，事件阻塞获取。
 */
class KeyDriver {
public:
  void Init();
  void ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low = true);
  void SetTimings(const GpioKeys::Timings& t);

  /**
   * @brief EXTI 中断回调中调用，通知有边沿。
   */
  void OnExtiFromISR(BaseType_t* hpw);

  /**
   * @brief 阻塞等待按键事件。
   * @param evt 输出事件
   * @param wait_ticks 等待时间
   * @return true 表示有事件填充
   */
  bool WaitEvent(KeyEvent& evt, TickType_t wait_ticks = portMAX_DELAY);

private:
  GpioKeys keys_;
  SemaphoreHandle_t exti_sem_ = nullptr;
  bool pressed_ = false;
  uint32_t press_start_ms_ = 0;
  uint32_t next_repeat_ms_ = 0;
  uint32_t repeat_count_ = 0;
  bool cancel_ = false;
};
