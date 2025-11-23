#pragma once
#include "Config/TaskConfig.hpp"
#include "Data/DB.hpp"
#include "IPC/LogIPC.hpp"
#include "IPC/ControlIPC.hpp"
#include "Tasks/Screen/LVGLTask.hpp"
#include "Tasks/Screen/KeyInputTask.hpp"
#include "Tasks/Screen/UIOutputTask.hpp"
#include "Tasks/Screen/TouchInputTask.hpp"
#include "Tasks/UDPTask.hpp"
#include "Tasks/MainCirTask.hpp"
#include "Tasks/LoggerTask.hpp"
#include "Tasks/SystemTask.hpp"

namespace Tasks {

static UDPTask udp_task;
static MainCirTask maincir_task;
static LoggerTask logger_task;
static SystemTask system_task;
static LVGLTask lvgl_task;
static UIOutputTask UI_output_task;
static TouchInputTask touch_input_task;
static KeyInputTask key_input_task;

void TaskStartFailHandle();

void init();

void startTasks();
} // namespace Tasks
