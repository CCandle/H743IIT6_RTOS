#pragma once

#include "Config/SystemConfig.hpp"
#include "Data/MainCirData.hpp"
#include "IPC/SysEvents.hpp"
#include <cmath>

class Protection {
public:
  Protection() = default;

  void detect(MainCirData& data) {
    const float v_out = data.sample.V_cap_up.toFloat() +
                  data.sample.V_cap_dn.toFloat();
    float i_bus = data.sample.I_bus.toFloat();
    auto& state = data.state;

    state.Fault = false;
    state.code = 0x00;

    // 硬件保护优先级：电流 > 单电容 > 总压 > 母线 > 其他
    if (std::fabs(i_bus) >= SystemConfig::I_BUS_OC_LIMIT) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_I_OC;
      return;
    }

    if (data.sample.V_cap_up.toFloat() >= SystemConfig::V_CAP_UP_MAX) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_V_CAP_UP_OV;
      return;
    }

    if (data.sample.V_cap_dn.toFloat() >= SystemConfig::V_CAP_DN_MAX) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_V_CAP_DN_OV;
      return;
    }

    if (v_out >= SystemConfig::V_CAP_SUM_MAX) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_V_CAP_SUM_OV;
      return;
    }

    const float v_bus = data.sample.V_bus.toFloat();
    if (v_bus <= SystemConfig::V_BUS_UV_MIN) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_V_BUS_UV;
      return;
    }
    if (v_bus >= SystemConfig::V_BUS_OV_MAX) {
      state.Fault = true;
      state.code = SysEvent::SYS_CRITICAL_V_BUS_OV;
      return;
    }
  }
};
