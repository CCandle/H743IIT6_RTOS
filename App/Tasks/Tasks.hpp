#pragma once
#include "Tasks/Screen/LVGLTask.hpp"
#include "Tasks/Screen/KeyInputTask.hpp"
#include "Tasks/Screen/UIOutputTask.hpp"
#include "Tasks/Screen/TouchInputTask.hpp"
#include "Tasks/UDPTask.hpp"

namespace Tasks {

static UDPTask udp_task;
static LVGLTask lvgl_task;
static UIOutputTask UI_output_task;
static TouchInputTask touch_input_task;
static KeyInputTask key_input_task;

void TaskStartFailHandle();

void init();

void startTasks();
} // namespace Tasks
