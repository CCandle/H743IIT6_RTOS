#pragma once
#include "Tasks/UDPTask.hpp"

namespace Tasks {

static UDPTask udp_task;

void TaskStartFailHandle();

void init();

void startTasks();
} // namespace Tasks
