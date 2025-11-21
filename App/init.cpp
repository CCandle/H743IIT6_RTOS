#include "Drivers/Drivers.hpp"
#include "IPC/IPC.hpp"
#include "IPC/LogIPC.hpp"
#include "Memory/memory.hpp"
#include "Tasks/Tasks.hpp"
#include "UartLogger.hpp"
#include "usart.h"
#include <cstdio>

extern "C" void sysInit() {
  Memory::Init();
  Drivers::init();
}

extern "C" void osInit() {
  IPC::init();
  IPC::Log::init();
  Tasks::init();
  uartLoggerInit(&huart1);
  printf("USART1 printf ready!\r\n");
  Tasks::startTasks();
}

extern "C" void configureTimerForRunTimeStats(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

extern "C" unsigned long getRunTimeCounterValue(void) {
  return DWT->CYCCNT;
}
