#pragma once
/**
 * @file Fixed.hpp
 * @brief 通用定点物理量封装模板，适用于嵌入式 C++ 工程（STM32 等）
 *
 * 本模板用于在内存中以整数形式（uint16_t / int16_t 等）存储
 * 物理量的定点表示，同时保持算法阶段的浮点计算精度。
 *
 * 示例：
 *   using Voltage = Fixed<uint16_t, 10>;             // 电压：V × 10
 *   using Current = Fixed<int16_t, 100>;             // 电流：A × 100，可为负
 *   using CurrentOffset = Fixed<uint16_t, 100, 1, 32768>; // 电流：A × 100，0A 对应32768
 *
 *   Voltage v = Voltage::fromFloat(230.5f); // -> raw=2305
 *   float vf = v.toFloat();                  // -> 230.5
 *
 * 特点：
 *  - 可选偏移 Offset：支持负数映射（如双向电流）
 *  - 全constexpr实现，可被编译器优化为零运行时开销
 *  - 类型安全，避免单位混用
 *  - 可配置比例分母（ScaleDen）支持非整数比例
 *
 * 作者：ChatGPT（2025）
 */

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

/**
 * @tparam RawType    存储类型，通常为 uint16_t 或 int16_t
 * @tparam ScaleNum   缩放分子。例如 "×100" 则写100
 * @tparam ScaleDen   缩放分母。默认为1，可用于非整数比例，如360°映射65535
 * @tparam Offset     可选偏移（原始整数域中的零点）。默认0
 *
 * @details
 *  物理量与存储整数之间的关系：
 *  raw = physical * (ScaleNum / ScaleDen) + Offset
 *  physical = (raw - Offset) / (ScaleNum / ScaleDen)
 */
template <typename RawType, int ScaleNum, int ScaleDen = 1, int Offset = 0>
class Fixed {
    static_assert(std::is_integral_v<RawType>, "RawType must be integer type");
    static_assert(ScaleDen > 0 && ScaleNum > 0, "Scale must be positive");

  public:
    using raw_t = RawType;
    static constexpr float scale = static_cast<float>(ScaleNum) / static_cast<float>(ScaleDen);
    static constexpr int offset = Offset;

    /// @brief 默认构造：值为0（对应 raw=Offset）
    constexpr Fixed() noexcept : raw_(Offset) {}

    /// @brief 从原始整数构造（不做比例换算）
    static constexpr Fixed fromRaw(raw_t raw) noexcept {
        Fixed r;
        r.raw_ = raw;
        return r;
    }

    /// @brief 从物理量（浮点）构造，自动做比例换算与偏移
    static constexpr Fixed fromFloat(float physical) noexcept {
        Fixed r;
        // physical -> raw (含偏移)
        const float raw_val = physical * scale + static_cast<float>(Offset);

        if constexpr (std::is_unsigned_v<raw_t>) {
            if (raw_val < 0.0f)
                r.raw_ = 0;
            else if (raw_val > static_cast<float>(std::numeric_limits<raw_t>::max()))
                r.raw_ = std::numeric_limits<raw_t>::max();
            else
                r.raw_ = static_cast<raw_t>(raw_val + 0.5f);
        } else {
            const float maxv = static_cast<float>(std::numeric_limits<raw_t>::max());
            const float minv = static_cast<float>(std::numeric_limits<raw_t>::min());
            r.raw_ = static_cast<raw_t>(std::clamp(raw_val, minv, maxv));
        }
        return r;
    }

    /// @brief 转换为浮点物理量（反比例换算 + 去偏移）
    constexpr float toFloat() const noexcept {
        return (static_cast<float>(raw_) - static_cast<float>(Offset)) / scale;
    }

    /// @brief 返回原始整数值
    constexpr raw_t raw() const noexcept { return raw_; }

    /// @brief 原地加上Δ值（单位：raw），带饱和
    constexpr void addRawSaturate(int32_t delta) noexcept {
        int32_t tmp = static_cast<int32_t>(raw_) + delta;
        const int32_t maxv = static_cast<int32_t>(std::numeric_limits<raw_t>::max());
        const int32_t minv = static_cast<int32_t>(std::numeric_limits<raw_t>::min());
        if (tmp > maxv)
            tmp = maxv;
        if (tmp < minv)
            tmp = minv;
        raw_ = static_cast<raw_t>(tmp);
    }

    /// @brief 运算符支持（相同模板类型）
    constexpr Fixed operator+(const Fixed& rhs) const noexcept {
        return Fixed::fromFloat(toFloat() + rhs.toFloat());
    }
    constexpr Fixed operator-(const Fixed& rhs) const noexcept {
        return Fixed::fromFloat(toFloat() - rhs.toFloat());
    }
    constexpr bool operator<(const Fixed& rhs) const noexcept { return toFloat() < rhs.toFloat(); }
    constexpr bool operator>(const Fixed& rhs) const noexcept { return toFloat() > rhs.toFloat(); }

    /// @brief 取负号（仅当RawType为有符号时有效）
    constexpr Fixed operator-() const noexcept
        requires(std::is_signed_v<raw_t>)
    {
        return Fixed::fromFloat(-toFloat());
    }

  private:
    raw_t raw_;
};
