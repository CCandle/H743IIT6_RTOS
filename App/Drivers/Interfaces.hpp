#pragma once

#include <cstdint>

/**
 * @brief 通用输入/输出设备接口枚举与结构，便于后续扩展。
 */

/// 触摸状态
enum class TouchState {
  None,
  Press,
  Move,
  Release
};

/// 触摸事件
struct TouchEvent {
  uint16_t x = 0;
  uint16_t y = 0;
  TouchState state = TouchState::None;
};

/// 按键状态
enum class KeyState {
  None,
  Press,
  Release,
  LongRelease,
  LongRepeat,
  Cancel
};

/// 按键事件（键值沿用 LVGL key 定义）
struct KeyEvent {
  uint32_t key = 0;
  KeyState state = KeyState::None;
};

/**
 * @brief 触摸控制器抽象接口，便于更换/扩展具体芯片。
 */
class ITouchDevice {
public:
  virtual ~ITouchDevice() = default;

  /**
   * @brief 初始化触摸控制器。
   * @return true 表示成功。
   */
  virtual bool Init() = 0;

  /**
   * @brief 执行硬件复位。
   */
  virtual void ResetChip() = 0;

  /**
   * @brief 状态寄存器指示数据就绪。
   * @return true 表示有数据可读。
   */
  virtual bool DataReady() = 0;

  /**
   * @brief 读取触摸事件。
   * @param evt 输出触摸事件（x/y/state）。
   * @return true 表示本次读取获得了有效事件。
   */
  virtual bool ReadTouch(TouchEvent& evt) = 0;
};

/**
 * @brief 按键输入抽象接口。
 */
class IKeyDevice {
public:
  virtual ~IKeyDevice() = default;

  /// 初始化设备
  virtual bool Init() = 0;
};

/**
 * @brief 输出设备抽象接口（预留），如 LED、蜂鸣器等。
 */
class IOutputDevice {
public:
  virtual ~IOutputDevice() = default;
  virtual void Init() = 0;
};
