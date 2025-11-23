#pragma once
#include "FreeRTOS.h"
#include "Memory/memory.hpp"
#include "task.h"
#include <concepts>
#include <cstring>
#include <type_traits>

// ===============================
// 内存区域定义
// ===============================
enum class MemoryRegion {
  DTCM,   // 最快内存，适合高频任务
  AXI,    // 通用内存
  RAM_D2, // 外设相关内存
  RAM_D3  // 低功耗内存
};

// ===============================
// 模板任务基类
// ===============================
template <typename Derived, MemoryRegion Region, size_t StackSize>
class TaskBase {
public:
  TaskBase() {
    // 初始化栈与TCB
    stackBuffer_ = getStackBuffer();
    tcbBuffer_ = getTcbBuffer();
  }

  // 禁止拷贝
  TaskBase(const TaskBase&) = delete;
  TaskBase& operator=(const TaskBase&) = delete;

  virtual ~TaskBase() {
    // 防止在任务自身上下文中自删
    if (handle_ && xTaskGetCurrentTaskHandle() != handle_) {
      vTaskDelete(handle_);
    }
  }

  // ===============================
  // 启动任务
  // ===============================
  bool Start(const char* name, UBaseType_t priority) {
    if (!stackBuffer_ || !tcbBuffer_)
      return false;

    // 清空任务栈与TCB
    std::memset(tcbBuffer_, 0, sizeof(StaticTask_t));
    std::memset(stackBuffer_, 0, StackSize * sizeof(StackType_t));

    handle_ = xTaskCreateStatic(
        &TaskEntryPoint,
        name,
        StackSize,
        this,
        priority,
        stackBuffer_,
        tcbBuffer_);

    return handle_ != nullptr;
  }

  // ===============================
  // 控制接口
  // ===============================
  void Suspend() {
    if (handle_)
      vTaskSuspend(handle_);
  }

  void Resume() {
    if (handle_)
      vTaskResume(handle_);
  }

  TaskHandle_t GetHandle() const { return handle_; }
  MemoryRegion GetMemoryRegion() const { return Region; }
  size_t GetStackSize() const { return StackSize; }

protected:
  // 安全延时
  void SafeDelay(TickType_t ticks) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
      vTaskDelay(ticks);
  }

private:
  // ===============================
  // 静态存储分配
  // 每个任务类唯一一份（模板 + Derived 区分）
  // ===============================

  // 为每个任务类生成唯一变量名
  template <typename T>
  struct UniqueTag {
    static inline T value;
  };

  StackType_t* getStackBuffer() {
    if constexpr (Region == MemoryRegion::DTCM) {
      __attribute__((section(".dtcm.data"))) static StackType_t buffer[StackSize];
      return buffer;
    } else if constexpr (Region == MemoryRegion::AXI) {
      __attribute__((section(".axi.data"))) static StackType_t buffer[StackSize];
      return buffer;
    } else if constexpr (Region == MemoryRegion::RAM_D2) {
      __attribute__((section(".d2.data"))) static StackType_t buffer[StackSize];
      return buffer;
    } else if constexpr (Region == MemoryRegion::RAM_D3) {
      __attribute__((section(".d3.data"))) static StackType_t buffer[StackSize];
      return buffer;
    } else {
      static_assert(true, "Invalid memory region");
      return nullptr;
    }
  }

  StaticTask_t* getTcbBuffer() {
    if constexpr (Region == MemoryRegion::DTCM) {
      __attribute__((section(".dtcm.data"))) static StaticTask_t tcb;
      return &tcb;
    } else if constexpr (Region == MemoryRegion::AXI) {
      __attribute__((section(".axi.data"))) static StaticTask_t tcb;
      return &tcb;
    } else if constexpr (Region == MemoryRegion::RAM_D2) {
      __attribute__((section(".d2.data"))) static StaticTask_t tcb;
      return &tcb;
    } else if constexpr (Region == MemoryRegion::RAM_D3) {
      __attribute__((section(".d3.data"))) static StaticTask_t tcb;
      return &tcb;
    } else {
      static_assert(true, "Invalid memory region");
      return nullptr;
    }
  }

  // ===============================
  // 任务入口包装
  // ===============================
  static void TaskEntryPoint(void* pvParameters) {
    auto* task = static_cast<TaskBase*>(pvParameters);
    static_cast<Derived*>(task)->Run();
    vTaskDelete(nullptr);
  }

  // ===============================
  // 成员变量
  // ===============================
  TaskHandle_t handle_ = nullptr;
  StackType_t* stackBuffer_ = nullptr;
  StaticTask_t* tcbBuffer_ = nullptr;
};

// ===============================
// 概念：有效任务类型
// ===============================
template <typename T>
concept ValidTask = requires(T t) {
  t.Run();
};

// ===============================
// 便捷别名
// ===============================
template <typename Derived, size_t Size = 128>
using DTCMTask = TaskBase<Derived, MemoryRegion::DTCM, Size>;

template <typename Derived, size_t Size = 512>
using AXITask = TaskBase<Derived, MemoryRegion::AXI, Size>;

template <typename Derived, size_t Size = 256>
using RAM_D2Task = TaskBase<Derived, MemoryRegion::RAM_D2, Size>;

template <typename Derived, size_t Size = 128>
using RAM_D3Task = TaskBase<Derived, MemoryRegion::RAM_D3, Size>;
