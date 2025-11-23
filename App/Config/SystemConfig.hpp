#pragma once
#include <cstdint>

// ==========================
// 控制系统配置参数
// ==========================

namespace SystemConfig {

// 预留 6 通道采样（实际使用 4，额外 2 备用）
constexpr uint8_t ADC_CHANNEL = 6;

// 电容、周期（20 kHz -> 50 us）
constexpr float CAP_VALUE = 50e-6f;   // [F]
constexpr float TS_VALUE  = 50e-6f;   // [s]

// ADC 标定（按实际硬件更新），每通道独立
struct AdcCalib {
  float gain;
  float offset;
};
// 通道含义：0-Vbus, 1-Vcap_up, 2-Vcap_dn, 3-Ibus, 4-RESV, 5-RESV
inline constexpr AdcCalib ADC_CALIB[ADC_CHANNEL] = {
    {1.0f, 0.0f}, // Vbus
    {1.0f, 0.0f}, // Vcap_up
    {1.0f, 0.0f}, // Vcap_dn
    {1.0f, 0.0f}, // Ibus
    {1.0f, 0.0f}, // Resv
    {1.0f, 0.0f}, // Resv
};

// PI 参数
constexpr float PI_KP = 2.09f;
constexpr float PI_KI = 0.00746f;
constexpr float PI_LIMIT_MIN = -250.0f;
constexpr float PI_LIMIT_MAX = 250.0f;

// 电流参考值及软启动
constexpr float IREF_TARGET = 10.0f;      // 最终目标电流 [A]
constexpr float IREF_START  = 0.5f;      // 起始电流 [A]
constexpr float IREF_SLOPE  = 1.0f;      // 斜率 [A/s]，按 TS 计算增量

// PWM 频率等其他配置（TIM1: PSC=23, ARR=249, center aligned）
constexpr uint16_t PWM_PERIOD   = 249; 
constexpr float PWM_MAX_DUTY = 1.0f;
constexpr float PWM_MIN_DUTY = 0.0f;

// 调试开环模式（示波器观察 PWM）
constexpr bool PWM_OPEN_LOOP = false;
constexpr float PWM_OPEN_LOOP_DUTY_UP = 0.1f;
constexpr float PWM_OPEN_LOOP_DUTY_DN = 0.1f;

// 硬件保护阈值（按实际硬件更新）
constexpr float I_BUS_OC_LIMIT = 80.0f;    // A
constexpr float V_CAP_UP_MAX = 820.0f;     // V
constexpr float V_CAP_DN_MAX = 820.0f;     // V
constexpr float V_CAP_SUM_MAX = 1600.0f;   // V
constexpr float V_BUS_UV_MIN = 200.0f;     // V
constexpr float V_BUS_OV_MAX = 1200.0f;    // V
constexpr float TEMP_IGBT_MAX = 90.0f;     // C （若有温度通道）
constexpr float TEMP_HEATSINK_MAX = 85.0f; // C （若有温度通道）

} // namespace SystemConfig
