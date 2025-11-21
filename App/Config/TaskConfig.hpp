#pragma once
#include "FreeRTOS.h"

// ==========================
// FreeRTOS任务配置参数
// ==========================

namespace TaskConfig {

constexpr uint16_t MAIN_CIR_TASK_STATCK_SIZE = 512;
constexpr UBaseType_t MAIN_CIR_TASK_PRIORITY = configMAX_PRIORITIES - 1;
constexpr uint16_t LOGGER_TASK_STACK_SIZE = 512;
constexpr UBaseType_t LOGGER_TASK_PRIORITY = configMAX_PRIORITIES - 2;
constexpr uint16_t SERIAL_TASK_STATCK_SIZE = 256;
constexpr UBaseType_t SERIAL_TASK_PRIORITY = configMAX_PRIORITIES - 2;
constexpr uint16_t SYSTEM_TASK_STATCK_SIZE = 512;
constexpr UBaseType_t SYSTEM_TASK_PRIORITY = configMAX_PRIORITIES - 2;

} // namespace TaskConfig
