#include "IPC/ControlIPC.hpp"

namespace IPC::Control {

QueueHandle_t control_queue = nullptr;

void init() {
  if (control_queue == nullptr) {
    control_queue = xQueueCreate(4, sizeof(Command));
  }
}

bool send(const Command& cmd, TickType_t to_ticks) {
  if (!control_queue) return false;
  return xQueueSend(control_queue, &cmd, to_ticks) == pdTRUE;
}

} // namespace IPC::Control
