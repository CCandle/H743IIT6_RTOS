#pragma once
#include "FreeRTOS.h"
#include "queue.h"
#include <cstdint>

namespace IPC::Control {

enum class CommandType : uint8_t { Start, Stop, Toggle, ResetFault };

struct Command {
  CommandType type;
  float iref; // optional, used when setting reference
  float ts;   // optional, used when setting Ts
};

extern QueueHandle_t control_queue;

void init();
bool send(const Command& cmd, TickType_t to_ticks = 0);

} // namespace IPC::Control
