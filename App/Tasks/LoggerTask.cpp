#include "Tasks/LoggerTask.hpp"
#include <cmath>

void LoggerTask::inject(RingBuffer<MainCirDataRaw, 64>* ringBuffer,
                        SnapshotStore* snapshot_store) {
  ringBuffer_ = ringBuffer;
  snapshot_store_ = snapshot_store;
}

void LoggerTask::Run() {
  for (;;) {
    size_t popped = 0;
    MainCirDataRaw frame{};
    while (ringBuffer_ && ringBuffer_->tryPop(frame)) {
      accumulate(frame);
      ++popped;
    }

    if (popped == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void LoggerTask::accumulate(const MainCirDataRaw& frame) {
  // 转为物理量（浮点）做统计
  const float v_bus = static_cast<float>(frame.sample.V_bus) / static_cast<float>(Scale::VOLTAGE);
  const float v_cap_up = static_cast<float>(frame.sample.V_cap_up) / static_cast<float>(Scale::VOLTAGE);
  const float v_cap_dn = static_cast<float>(frame.sample.V_cap_dn) / static_cast<float>(Scale::VOLTAGE);
  const float i_bus = static_cast<float>(frame.sample.I_bus) / static_cast<float>(Scale::CURRENT);
  const float duty_up = static_cast<float>(frame.control.Duty_IGBT_up) / static_cast<float>(Scale::DUTY);
  const float duty_dn = static_cast<float>(frame.control.Duty_IGBT_dn) / static_cast<float>(Scale::DUTY);

  acc_v_bus_.add(v_bus);
  acc_v_cap_up_.add(v_cap_up);
  acc_v_cap_dn_.add(v_cap_dn);
  acc_i_bus_.add(i_bus);
  acc_duty_up_.add(duty_up);
  acc_duty_dn_.add(duty_dn);

  if (frame.state.Fault) {
    last_fault_flag_ = frame.state.Fault;
    last_fault_code_ = frame.state.code;
  }

  ++window_count_;
  if (window_count_ >= WINDOW_SIZE) {
    finalizeSnapshot();
  }
}

void LoggerTask::finalizeSnapshot() {
  Snapshot snap{};
  snap.seq = ++seq_;
  snap.count = window_count_;
  snap.v_bus = acc_v_bus_.finalize();
  snap.v_cap_up = acc_v_cap_up_.finalize();
  snap.v_cap_dn = acc_v_cap_dn_.finalize();
  snap.i_bus = acc_i_bus_.finalize();
  snap.duty_up = acc_duty_up_.finalize();
  snap.duty_dn = acc_duty_dn_.finalize();
  snap.last_fault_flag = last_fault_flag_;
  snap.last_fault_code = last_fault_code_;

  if (snapshot_store_) {
    snapshot_store_->update(snap);
  }

  // 重置窗口
  window_count_ = 0;
  acc_v_bus_.reset();
  acc_v_cap_up_.reset();
  acc_v_cap_dn_.reset();
  acc_i_bus_.reset();
  acc_duty_up_.reset();
  acc_duty_dn_.reset();
  last_fault_flag_ = 0;
}
