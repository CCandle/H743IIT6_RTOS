#pragma once
#include "main.h"

namespace UartLogger {

void init(UART_HandleTypeDef* huart);
void putChar(char c);

} // namespace UartLogger

extern "C" {
void uartLoggerInit(UART_HandleTypeDef* huart);
void uartLoggerPutChar(char c);
}
