#pragma once
#include "Tasks/UDPTask.hpp"
#include "Tasks/LCDTask.hpp"

namespace Tasks {

static UDPTask udp_task;
static LCDTask lcd_task;

void TaskStartFailHandle();

void init();

void startTasks();
} // namespace Tasks
