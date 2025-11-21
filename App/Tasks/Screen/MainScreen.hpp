#pragma once
#include "Data/Snapshot.hpp"
#include "IPC/ControlIPC.hpp"
#include "lvgl/lvgl.h"
#include "FreeRTOS.h"
#include "queue.h"

namespace UI {

void CreateMainScreen(QueueHandle_t control_queue, SnapshotStore* snapshot_store);
void UpdateMainScreenDiag(uint32_t fault_code, bool fault_flag);

} // namespace UI
