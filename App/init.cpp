#include "Drivers/Drivers.hpp"
#include "IPC/IPC.hpp"
#include "IPC/LogIPC.hpp"
#include "Memory/memory.hpp"
#include "Tasks/Tasks.hpp"
#include "UartLogger.hpp"
#include "core_cm7.h"
#include "usart.h"
#include <cstdio>

extern "C" void EnableTCM() {
  // Enable ITCM(64KB)/DTCM(128KB) with RMW+RETEN; cache handled in main.c
  const uint32_t itcm_sz = (0x7u << SCB_ITCMCR_SZ_Pos);  // 64KB
  const uint32_t dtcm_sz = (0x8u << SCB_DTCMCR_SZ_Pos);  // 128KB
  SCB->ITCMCR = SCB_ITCMCR_EN_Msk | SCB_ITCMCR_RMW_Msk | SCB_ITCMCR_RETEN_Msk | itcm_sz;
  SCB->DTCMCR = SCB_DTCMCR_EN_Msk | SCB_DTCMCR_RMW_Msk | SCB_DTCMCR_RETEN_Msk | dtcm_sz;
  __DSB();
  __ISB();
}

extern "C" void AppMemoryInit() {
  Memory::Init();
}

extern "C" void AppDriversInit() {
  Drivers::init();
}

extern "C" void AppOsObjectsInit() {
  IPC::init();
  Tasks::init();
  uartLoggerInit(&huart1);
  printf("USART1 printf ready!\r\n");
}

extern "C" void StartAppTasks() {
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
