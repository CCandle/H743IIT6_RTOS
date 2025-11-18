#include "Memory/memory.hpp"
#include "UartLogger.hpp"
#include <cstdio>
#include "usart.h"
#include "Tasks/Tasks.hpp"

extern "C" void sysInit() {
  Memory::Init();
  
}

extern "C" void osInit(){
  uartLoggerInit(&huart1);
  printf("USART1 printf ready!\r\n");
  Tasks::startTasks();
}
