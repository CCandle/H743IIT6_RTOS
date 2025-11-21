#pragma once
#include <cstddef>

#include "Drivers/Interfaces.hpp"
#include "GT1151Q.hpp"
#include "GpioKeys.hpp"
#include "KeyDriver.hpp"
#include "main.h"

namespace Drivers {
/**
 * @brief 获取主触摸设备，当前为 GT1151Q，实现 ITouchDevice。
 */
ITouchDevice& GetPrimaryTouch();

/**
 * @brief 获取主按键设备（最多 6 个 GPIO 按键）。
 */
IKeyDevice& GetPrimaryKeys();

/**
 * @brief 获取按键驱动实例。
 */
KeyDriver& GetKeyDriver();

/**
 * @brief 配置单个按键的引脚与键值。
 * @param idx     序号 0..5
 * @param port    GPIO 端口
 * @param pin     GPIO 引脚
 * @param lvgl_key 对应 LVGL 键值（如 LV_KEY_ENTER 等）
 * @param active_low 是否低电平有效
 */
void ConfigureKey(std::size_t idx, GPIO_TypeDef* port, uint16_t pin, uint32_t lvgl_key, bool active_low = true);

/**
 * @brief 初始化所有已注册设备（当前仅触摸）。
 */
void init();
} // namespace Drivers
