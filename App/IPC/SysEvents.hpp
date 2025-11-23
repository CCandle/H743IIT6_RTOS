#pragma once

namespace SysEvent {
enum _SysEvntCat {
    STATE,
    INFO,
    WARNING,
    CRITICAL
};

template <_SysEvntCat cat>
constexpr uint32_t _cat_mask() { return 0xFFUL << (8 * static_cast<uint8_t>(cat)); }

template <_SysEvntCat cat, uint8_t bit>
constexpr uint32_t _event_val() {
    static_assert(bit < 8, "SYS EVENT SET BIT MUST BETWEEN 0-7;");
    return 0x01UL << (static_cast<uint8_t>(cat) * 8 + bit);
}

constexpr uint32_t SYS_STATE_MASK = _cat_mask<STATE>();
constexpr uint32_t SYS_INFO_MASK = _cat_mask<INFO>();
constexpr uint32_t SYS_WARNING_MASK = _cat_mask<WARNING>();
constexpr uint32_t SYS_CRITICAL_MASK = _cat_mask<CRITICAL>();

constexpr uint32_t SYS_STATE_BOOT = _event_val<STATE, 0>();
constexpr uint32_t SYS_STATE_INIT = _event_val<STATE, 1>();
constexpr uint32_t SYS_STATE_STANDBY = _event_val<STATE, 2>();
constexpr uint32_t SYS_STATE_RUNNING = _event_val<STATE, 3>();
constexpr uint32_t SYS_STATE_ERROR = _event_val<STATE, 4>();
constexpr uint32_t SYS_STATE_SHUTDOWN = _event_val<STATE, 5>();

constexpr uint32_t SYS_INFO_CONFIG_UNLOADED = _event_val<INFO, 0>();

constexpr uint32_t SYS_WARNING_TEMP_HIGH = _event_val<WARNING, 0>();
constexpr uint32_t SYS_WARNING_TEMP_LOW = _event_val<WARNING, 1>();
constexpr uint32_t SYS_WARNING_ETH_LINK_DOWN = _event_val<WARNING, 2>();
constexpr uint32_t SYS_WARNING_ETH_LOSS = _event_val<WARNING, 3>();

constexpr uint32_t SYS_CRITICAL_I_OC = _event_val<CRITICAL, 0>();
constexpr uint32_t SYS_CRITICAL_V_CAP_UP_OV = _event_val<CRITICAL, 1>();
constexpr uint32_t SYS_CRITICAL_V_CAP_DN_OV = _event_val<CRITICAL, 2>();
constexpr uint32_t SYS_CRITICAL_V_CAP_SUM_OV = _event_val<CRITICAL, 3>();
constexpr uint32_t SYS_CRITICAL_V_BUS_UV = _event_val<CRITICAL, 4>();
constexpr uint32_t SYS_CRITICAL_V_BUS_OV = _event_val<CRITICAL, 5>();
constexpr uint32_t SYS_CRITICAL_ADC_FAULT = _event_val<CRITICAL, 6>();
constexpr uint32_t SYS_CRITICAL_SYNC_FAULT = _event_val<CRITICAL, 7>();
} // namespace SysEvent
