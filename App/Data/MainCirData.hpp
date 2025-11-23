#pragma once
#include "Utils/Units/Units.hpp"
#include <cstdint>

/**
 * @brief 主回路数据结构体（可嵌套结构体）
 */
struct MainCirDataRaw {
    struct Sample {
        uint16_t V_bus;
        uint16_t V_cap_up;
        uint16_t V_cap_dn;
        uint16_t I_bus;
        uint16_t ch4_resv;
        uint16_t ch5_resv;
    } sample;

    struct Control {
        uint16_t I_ref;
        uint16_t Duty_IGBT_up;
        uint16_t Duty_IGBT_dn;
    } control;

    struct State {
        uint8_t Fault;
        uint32_t code;
    } state;

    uint32_t timestamp;
} __attribute__((packed, aligned(4)));

/**
 * @brief 内部高精度数据结构体（供算法使用）
 */
struct MainCirData {
    struct Sample {
        Volt V_bus;
        Volt V_cap_up;
        Volt V_cap_dn;
        Curr I_bus;
        uint16_t ch4_resv = 0;
        uint16_t ch5_resv = 0;
    } sample;

    struct Control {
        Curr I_ref;
        Duty Duty_IGBT_up;
        Duty Duty_IGBT_dn;
    } control;

    struct State {
        bool Fault = false;
        uint32_t code = 0x0000'0000;
    } state;
};
