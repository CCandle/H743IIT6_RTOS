#pragma once
namespace Scale {

// 存储比例（raw = value * SCALE）
inline constexpr int VOLTAGE = 10;
inline constexpr int CURRENT = 100;
inline constexpr int CAP = 1e6;
inline constexpr int DUTY = 10000;
inline constexpr int TEMP = 10;

// 物理范围（用于校验 / 限幅）
inline constexpr float VOLTAGE_MIN = 0.0f;
inline constexpr float VOLTAGE_MAX = 1600.0f;
inline constexpr float CURRENT_MIN = -2.0f;
inline constexpr float CURRENT_MAX = 80.0f;
inline constexpr float DUTY_MIN = 0.0f;
inline constexpr float DUTY_MAX = 1.0f;
inline constexpr float TEMP_MIN = -40.0f;
inline constexpr float TEMP_MAX = 120.0f;

} // namespace Scale