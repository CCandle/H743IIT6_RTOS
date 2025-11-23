#pragma once
#include "Config/TaskConfig.hpp"
#include "Data/Snapshot.hpp"
#include "IPC/ControlIPC.hpp"
#include "OS/TaskBase.hpp"
#include "Tasks/MainCirTask.hpp"
#include "FreeRTOS.h"
#include "queue.h"

class SystemTask : public AXITask<SystemTask, TaskConfig::SYSTEM_TASK_STATCK_SIZE> {
public:
  SystemTask() = default;

  void inject(MainCirTask* main_task,
              SnapshotStore* snapshot_store,
              QueueHandle_t control_queue);

  void Run();

private:
  enum class State : uint8_t { Stop, Run, Fault };

  void handleCommand(const IPC::Control::Command& cmd);
  void applyState();
  void applySettings(const IPC::Control::Command& cmd);

private:
  MainCirTask* main_task_{nullptr};
  SnapshotStore* snapshot_store_{nullptr};
  QueueHandle_t control_queue_{nullptr};
  State state_{State::Stop};
  uint32_t last_fault_code_{0};
  float iref_cmd_{0.0f};
  float ts_cmd_{SystemConfig::TS_VALUE};
};
