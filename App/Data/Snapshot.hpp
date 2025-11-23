#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include <cstdint>

struct SnapshotStat {
  float avg = 0.0f;
  float min = 0.0f;
  float max = 0.0f;
  float rms = 0.0f;
};

struct Snapshot {
  uint32_t seq = 0;
  uint32_t count = 0;
  SnapshotStat v_bus;
  SnapshotStat v_cap_up;
  SnapshotStat v_cap_dn;
  SnapshotStat i_bus;
  SnapshotStat duty_up;
  SnapshotStat duty_dn;
  uint32_t last_fault_code = 0;
  uint8_t last_fault_flag = 0;
};

/**
 * @brief 线程安全的 Snapshot 存储
 */
struct SnapshotStore {
  Snapshot snapshot{};
  SemaphoreHandle_t mutex = nullptr;

  void init() {
    if (!mutex) {
      mutex = xSemaphoreCreateMutex();
    }
  }

  bool update(const Snapshot& src) {
    if (!mutex) return false;
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      snapshot = src;
      xSemaphoreGive(mutex);
      return true;
    }
    return false;
  }

  bool read(Snapshot& dst, TickType_t to_ticks = pdMS_TO_TICKS(1)) const {
    if (!mutex) return false;
    if (xSemaphoreTake(mutex, to_ticks) == pdTRUE) {
      dst = snapshot;
      xSemaphoreGive(mutex);
      return true;
    }
    return false;
  }
};
