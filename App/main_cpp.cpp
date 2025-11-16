#include "memory.hpp"
#include "UartLogger.hpp"
#include <cstdio>
#include "usart.h"
#include "Tasks/Tasks.hpp"

void TaskStartFailHandle();

/**
 * @brief 提供C接口主函数，在FREERTOS启动之前统一完成初始化
 * @details 初始化顺序：MEM -> TASKS -> SCHEDULER
 */
extern "C" void main_cpp(void) {
  MEM::Init();
  uartLoggerInit(&huart1);
  printf("USART1 printf ready!\r\n");
  HAL_UART_Transmit_IT(&huart1, (uint8_t*)"USART1 HAL_UART_Transmit_IT ready!\r\n", 36);
  StartMemRegionTasks();
}

void TaskStartFailHandle() {
  for (;;) {
  }
}