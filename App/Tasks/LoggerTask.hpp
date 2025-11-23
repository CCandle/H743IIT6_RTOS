#pragma once
#include "Data/MainCirData.hpp"
#include "Data/RingBuffer.hpp"
#include "Data/Snapshot.hpp"
#include "OS/TaskBase.hpp"
#include "Utils/Units/Scale.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <cfloat>
#include <cstdint>
#include <cmath>

class LoggerTask : public AXITask<LoggerTask, 512> {
public:
  LoggerTask() = default;

  void inject(RingBuffer<MainCirDataRaw, 64>* ringBuffer,
              SnapshotStore* snapshot_store);

  void Run();

private:
  struct Acc {
    float sum = 0.0f;
    float sumsq = 0.0f;
    float minv = FLT_MAX;
    float maxv = -FLT_MAX;
    uint32_t n = 0;

    inline void reset() {
      sum = 0.0f; sumsq = 0.0f;
      minv = FLT_MAX; maxv = -FLT_MAX;
      n = 0;
    }

    inline void add(float v) {
      sum += v;
      sumsq += v * v;
      if (v < minv) minv = v;
      if (v > maxv) maxv = v;
      ++n;
    }

    inline SnapshotStat finalize() const {
      SnapshotStat s{};
      if (n == 0) return s;
      const float inv = 1.0f / static_cast<float>(n);
      s.avg = sum * inv;
      s.min = minv;
      s.max = maxv;
      s.rms = sqrtf(sumsq * inv);
      return s;
    }
  };

  void accumulate(const MainCirDataRaw& frame);
  void finalizeSnapshot();

private:
  RingBuffer<MainCirDataRaw, 64>* ringBuffer_{nullptr};
  SnapshotStore* snapshot_store_{nullptr};
  uint32_t window_count_{0};
  uint32_t seq_{0};
  Acc acc_v_bus_;
  Acc acc_v_cap_up_;
  Acc acc_v_cap_dn_;
  Acc acc_i_bus_;
  Acc acc_duty_up_;
  Acc acc_duty_dn_;
  uint32_t last_fault_code_{0};
  uint8_t last_fault_flag_{0};

  static constexpr uint32_t WINDOW_SIZE = 1000; // 50ms @20kHz
};
