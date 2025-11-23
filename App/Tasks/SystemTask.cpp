#include "Tasks/SystemTask.hpp"

void SystemTask::inject(MainCirTask* main_task,
                        SnapshotStore* snapshot_store,
                        QueueHandle_t control_queue) {
  main_task_ = main_task;
  snapshot_store_ = snapshot_store;
  control_queue_ = control_queue;
}

void SystemTask::handleCommand(const IPC::Control::Command& cmd) {
  switch (cmd.type) {
  case IPC::Control::CommandType::Start:
    state_ = State::Run;
    if (main_task_) {
      main_task_->requestReset();
    }
    break;
  case IPC::Control::CommandType::Stop:
    state_ = State::Stop;
    break;
  case IPC::Control::CommandType::Toggle:
    state_ = (state_ == State::Run) ? State::Stop : State::Run;
    if (state_ == State::Run && main_task_) {
      main_task_->requestReset();
    }
    break;
  case IPC::Control::CommandType::ResetFault:
    last_fault_code_ = 0;
    state_ = State::Stop;
    if (main_task_) {
      main_task_->requestReset();
    }
    break;
  }

  // apply optional settings
  if (cmd.iref != 0.0f || cmd.ts != 0.0f) {
    applySettings(cmd);
  }
}

void SystemTask::applyState() {
  if (!main_task_) return;
  if (state_ == State::Run && last_fault_code_ == 0) {
    main_task_->setRunEnabled(true);
  } else {
    main_task_->setRunEnabled(false);
  }
}

void SystemTask::applySettings(const IPC::Control::Command& cmd) {
  if (cmd.iref != 0.0f) {
    iref_cmd_ = cmd.iref;
    // TODO: forward to controller/modulate if interface added
  }
  if (cmd.ts != 0.0f) {
    ts_cmd_ = cmd.ts;
    // TODO: forward to modulate/PI if interface added
  }
}

void SystemTask::Run() {
  for (;;) {
    IPC::Control::Command cmd;
    if (control_queue_ && xQueueReceive(control_queue_, &cmd, pdMS_TO_TICKS(50)) == pdPASS) {
      handleCommand(cmd);
    }

    if (snapshot_store_) {
      Snapshot snap;
      if (snapshot_store_->read(snap, 0)) {
        if (snap.last_fault_flag) {
          last_fault_code_ = snap.last_fault_code;
          state_ = State::Fault;
        } else if (state_ == State::Fault) {
          // remain faulted until reset command
          main_task_->setRunEnabled(false);
        }
      }
    }

    applyState();
  }
}
