#pragma once
#include "FreeRTOS.h"
#include "Tasks/Screen/UIOutputTask.hpp" // For DisplayMessage definition
#include "queue.h"
#include "semphr.h"

namespace IPC {
extern QueueHandle_t display_queue;
extern SemaphoreHandle_t buf1_sem;
extern SemaphoreHandle_t buf2_sem;

void init();

} // namespace IPC
