#pragma once
#include "Tasks/LCDTask.hpp"
#include "Tasks/UDPTask.hpp"

namespace Tasks {

static UDPTask udp_task;
static LCDTask lcd_task;

void TaskStartFailHandle();

void init();

void startTasks();
} // namespace Tasks
