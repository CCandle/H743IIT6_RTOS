#include "UartLogger.hpp"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "task.h"
#include <cstdio>
// #include "mutex.h"

namespace UartLogger {

static constexpr uint16_t BUF_SIZE = 512;

static uint8_t buf[BUF_SIZE];
static volatile uint16_t head = 0;
static volatile uint16_t tail = 0;
static UART_HandleTypeDef* gUart = nullptr;
static SemaphoreHandle_t mutex = nullptr;

static void trySend() {
  if (!gUart)
    return;
  if (gUart->gState != HAL_UART_STATE_READY)
    return;
  if (head == tail)
    return;

  uint16_t chunk =
      (head > tail) ? (head - tail)
                    : (BUF_SIZE - tail);

  HAL_UART_Transmit_IT(gUart, &buf[tail], chunk);
}

void init(UART_HandleTypeDef* huart) {
  gUart = huart;
  mutex = xSemaphoreCreateMutex();
  setvbuf(stdout, NULL, _IONBF, 0);
}

void putChar(char c) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  uint16_t next = (head + 1) % BUF_SIZE;
  while (next == tail) {
    xSemaphoreGive(mutex);
    taskYIELD();
    xSemaphoreTake(mutex, portMAX_DELAY);
  }

  buf[head] = (uint8_t)c;
  head = next;

  trySend();
  xSemaphoreGive(mutex);
}

} // namespace UartLogger

extern "C" {

// ---- C Wrapper ----

void uartLoggerInit(UART_HandleTypeDef* huart) {
  UartLogger::init(huart);
}

void uartLoggerPutChar(char c) {
  UartLogger::putChar(c);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
  using namespace UartLogger;
  // extern volatile uint16_t tail, head;
  // extern uint8_t buf[];

  tail = (tail + huart->TxXferSize) % BUF_SIZE;
  trySend();
}

} // extern "C"
