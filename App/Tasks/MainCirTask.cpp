#include "Tasks/MainCirTask.hpp"
#include "adc.h"
#include "tim.h"
#include "stm32h7xx_hal.h"
#include <algorithm>

// DMA 缓冲：32B 对齐，长度按配置保留 6 路（实际用 4 路）
#ifndef ALIGN_32BYTES
#define ALIGN_32BYTES(buf) buf __attribute__((aligned(32)))
#endif

__attribute__((section(".axi.data"))) ALIGN_32BYTES(static uint16_t adc_dma_buf[SystemConfig::ADC_CHANNEL]);
static constexpr size_t adc_dma_len = SystemConfig::ADC_CHANNEL;
static TaskHandle_t g_maincir_task_handle = nullptr;
static volatile uint32_t g_isr_fault_code = 0;

static inline void invalidateAdcCache() {
#if defined(D_CACHE_PRESENT) && (D_CACHE_PRESENT == 1)
  constexpr uint32_t len_bytes = adc_dma_len * sizeof(uint16_t);
  constexpr uint32_t len_aligned = (len_bytes + 31U) & ~31U;
  SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t*>(adc_dma_buf),
                               len_aligned);
#endif
}

void MainCirTask::inject(RingBuffer<MainCirDataRaw, 64>* ringBuffer) {
  ringBuffer_ = ringBuffer;
}

void MainCirTask::setRunEnabled(bool enable) {
  run_enabled_ = enable;
}

void MainCirTask::requestReset() {
  reset_request_ = true;
}

void MainCirTask::Run() {
  self_handle_ = xTaskGetCurrentTaskHandle();
  g_maincir_task_handle = self_handle_;

  sampling_.attachBuffer(adc_dma_buf, adc_dma_len);
  startHardware();

  constexpr TickType_t kSyncTimeout = pdMS_TO_TICKS(2);

  for (;;) {
    // 等待 DMA 完成；若超时，视为同步异常
    if (ulTaskNotifyTake(pdTRUE, kSyncTimeout) == 0) {
      fault_latched_ = true;
      fault_code_ = SysEvent::SYS_CRITICAL_SYNC_FAULT;
      controller_.shutdown();
    }

    // ISR 报错
    if (g_isr_fault_code != 0) {
      fault_latched_ = true;
      fault_code_ = g_isr_fault_code;
      g_isr_fault_code = 0;
      controller_.shutdown();
    }

    if (!ringBuffer_ || fault_latched_) {
      continue;
    }

    if (reset_request_) {
      reset_request_ = false;
      fault_latched_ = false;
      fault_code_ = 0;
      pwm_running_ = false;
      if (run_enabled_) {
        controller_.start();
        pwm_running_ = true;
      } else {
        controller_.shutdown();
      }
    }

    execute();
  }
}

void MainCirTask::startHardware() {
  fault_latched_ = false;
  pwm_running_ = false;
  controller_.shutdown();

  // TIM15: 用于 ADC 触发 OC/TRGO，计数将被 TIM1 TRGO2 复位
  HAL_TIM_OC_Start(&htim15, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim15);

  // ADC1 + DMA 外部触发
  HAL_ADC_Start_DMA(&hadc1,
                    reinterpret_cast<uint32_t*>(adc_dma_buf),
                    adc_dma_len);
  adc_started_ = true;
}

void MainCirTask::stopHardware() {
  if (adc_started_) {
    HAL_ADC_Stop_DMA(&hadc1);
    adc_started_ = false;
  }
  HAL_TIM_Base_Stop(&htim15);
  HAL_TIM_OC_Stop(&htim15, TIM_CHANNEL_1);
  controller_.shutdown();
}

void MainCirTask::execute() {
  static MainCirData data;

  // 采样/保护/调制/执行
  sampling_.update(data);
  protection_.detect(data);

  if (run_enabled_ && !pwm_running_ && !fault_latched_) {
    controller_.start();
    pwm_running_ = true;
  }

  if (data.state.Fault) {
    fault_latched_ = true;
    fault_code_ = data.state.code;
    controller_.shutdown();
    pwm_running_ = false;
  } else if (!run_enabled_) {
    controller_.shutdown();
    pwm_running_ = false;
    data.control.Duty_IGBT_up = Duty::fromFloat(0.0f);
    data.control.Duty_IGBT_dn = Duty::fromFloat(0.0f);
    data.control.I_ref = Curr::fromFloat(0.0f);
  } else {
    if constexpr (SystemConfig::PWM_OPEN_LOOP) {
      data.control.Duty_IGBT_up = Duty::fromFloat(SystemConfig::PWM_OPEN_LOOP_DUTY_UP);
      data.control.Duty_IGBT_dn = Duty::fromFloat(SystemConfig::PWM_OPEN_LOOP_DUTY_DN);
      data.control.I_ref = Curr::fromFloat(0.0f);
    } else {
      modulate_.compute(data);
    }
    controller_.apply(data);
  }

  MainCirDataRaw frame{};
  const bool fault = fault_latched_ || data.state.Fault;
  const uint32_t code = fault_latched_ ? fault_code_ : data.state.code;
  writeFrame(frame, data, fault, code);
  ringBuffer_->pushOverwrite(frame);
}

void MainCirTask::writeFrame(MainCirDataRaw& frame, const MainCirData& data, bool fault, uint32_t fault_code) noexcept {
  frame.sample.V_bus = data.sample.V_bus.raw();
  frame.sample.V_cap_up = data.sample.V_cap_up.raw();
  frame.sample.V_cap_dn = data.sample.V_cap_dn.raw();
  frame.sample.I_bus = data.sample.I_bus.raw();
  frame.sample.ch4_resv = data.sample.ch4_resv;
  frame.sample.ch5_resv = data.sample.ch5_resv;

  frame.control.I_ref = data.control.I_ref.raw();
  frame.control.Duty_IGBT_up = data.control.Duty_IGBT_up.raw();
  frame.control.Duty_IGBT_dn = data.control.Duty_IGBT_dn.raw();

  frame.state.Fault = fault ? 1 : 0;
  frame.state.code = fault ? fault_code : data.state.code;
  frame.timestamp = DWT->CYCCNT;
}

void MainCirTask::OnAdcCpltFromISR() {
  if (!g_maincir_task_handle) {
    return;
  }

  invalidateAdcCache();

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(g_maincir_task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void MainCirTask::OnAdcErrorFromISR(uint32_t err_code) {
  g_isr_fault_code = err_code;
  if (!g_maincir_task_handle) {
    return;
  }
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(g_maincir_task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  if (hadc == &hadc1) {
    MainCirTask::OnAdcCpltFromISR();
  }
}

extern "C" void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {
  if (hadc == &hadc1) {
    MainCirTask::OnAdcErrorFromISR(SysEvent::SYS_CRITICAL_ADC_FAULT);
  }
}
