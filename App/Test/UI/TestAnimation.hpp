#pragma once

#include "lvgl/lvgl.h"

/**
 * @brief 构建测试动画界面（渐变背景 + 弹跳球）。
 *
 * 仅在编译开关允许时调用，用于验证刷新管线与 DMA 同步。
 */
void CreateTestAnimation();
