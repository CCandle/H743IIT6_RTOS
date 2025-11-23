#pragma once

#include "lvgl/lvgl.h"

/**
 * @brief 构建测试动画界面（渐变背景 + 弹跳球）。
 *
 * 仅在编译开关允许时调用，用于验证刷新管线与 DMA 同步。
 */
void CreateTestAnimation();

/**
 * @brief 更新按键状态显示。
 * @param status 文本（short/long/repeat/release/cancel 等）
 * @param repeat_count 如为 repeat 可附带计数。
 */
void UpdateKeyStatus(const char* status, uint32_t repeat_count);
