#include "IPC.hpp"

namespace IPC {
QueueHandle_t display_queue = nullptr;
SemaphoreHandle_t buf1_sem = nullptr;
SemaphoreHandle_t buf2_sem = nullptr;
SemaphoreHandle_t lvgl_ready_sem = nullptr;
SemaphoreHandle_t lvgl_mutex = nullptr;
void init() {
  display_queue = xQueueCreate(4, sizeof(DisplayMessage)); // 深度按需
  buf1_sem = xSemaphoreCreateBinary();
  buf2_sem = xSemaphoreCreateBinary();
  lvgl_ready_sem = xSemaphoreCreateBinary();
  lvgl_mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(buf1_sem);
  xSemaphoreGive(buf2_sem);
}
} // namespace IPC
