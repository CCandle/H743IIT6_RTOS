#pragma once
#include "Config/SystemConfig.hpp"
#include "Data/MainCirData.hpp"
#include "Data/RingBuffer.hpp"
#include "OS/TaskBase.hpp"
#include "Logic/Controller.hpp"
#include "Logic/Modulate.hpp"
#include "Logic/Protection.hpp"
#include "Logic/Sampling.hpp"
#include "IPC/SysEvents.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <cstdint>

class MainCirTask : public DTCMTask<MainCirTask, 512> {
public:
  MainCirTask() = default;

  // 20 kHz 控制主循环（阻塞等待 ADC 完成通知）
  __attribute__((section(".itcm.text"), noinline)) void Run();

  // 注入单生产者 RingBuffer 引用
  void inject(RingBuffer<MainCirDataRaw, 64>* ringBuffer);

  // 控制接口（由系统任务调用）
  void setRunEnabled(bool enable);
  void requestReset();

  // ADC DMA 结束回调中调用的静态入口
  static void OnAdcCpltFromISR();
  static void OnAdcErrorFromISR(uint32_t err_code);

private:
  void startHardware();
  void stopHardware();
  void execute();
  static void writeFrame(MainCirDataRaw& frame, const MainCirData& data, bool fault, uint32_t fault_code) noexcept;

private:
  bool adc_started_{false};
  bool fault_latched_{false};
  uint32_t fault_code_{0};
  bool run_enabled_{false};
  bool pwm_running_{false};
  volatile bool reset_request_{false};
  RingBuffer<MainCirDataRaw, 64>* ringBuffer_{nullptr};
  Sampling sampling_;
  Protection protection_;
  Modulate modulate_;
  Controller controller_;
  TaskHandle_t self_handle_{nullptr};
};
