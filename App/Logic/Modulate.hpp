#pragma once
#include "Config/SystemConfig.hpp"
#include "Data/MainCirData.hpp"
#include "Logic/Algo/PI.hpp"
#include "Logic/Algo/Vec.hpp"
#include <cmath>

#define ITCM_FUNC_MODULATE __attribute__((section(".itcm.text.modulate")))

class Modulate {
public:
  Modulate()
      : pi_(SystemConfig::PI_KP, SystemConfig::PI_KI,
            SystemConfig::PI_LIMIT_MAX, SystemConfig::PI_LIMIT_MIN,
            SystemConfig::TS_VALUE),
        vec_(SystemConfig::CAP_VALUE, SystemConfig::TS_VALUE),
        i_ref_(SystemConfig::IREF_START),
        iref_slope_ts_(SystemConfig::IREF_SLOPE * SystemConfig::TS_VALUE),
        soft_started_(false) {}

  ITCM_FUNC_MODULATE void compute(MainCirData& data) {
    auto& sample = data.sample;
    auto& control = data.control;

    // 防止 I_ref 过小导致除零
    constexpr float kIRefMin = 1e-3f;
    if (std::fabs(i_ref_) < kIRefMin) {
      i_ref_ = (i_ref_ >= 0.0f) ? kIRefMin : -kIRefMin;
    }

    // ---- 电流软启动 ----
    if (!soft_started_) {
      if (i_ref_ < SystemConfig::IREF_TARGET) {
        i_ref_ += iref_slope_ts_;
        control.I_ref = Curr::fromFloat(i_ref_);
      } else {
        soft_started_ = true;
      }
    }

    // ---- PI计算（直接通过引用设置输入） ----
    PI::Input& pi_input = pi_.input();
    pi_input.ref = i_ref_;
    pi_input.fdb = sample.I_bus.toFloat();

    pi_.compute();
    const float v_ref = sample.V_bus.toFloat() - pi_.output().out;

    // ---- Vec计算（直接通过引用设置输入） ----
    Vec::Input& vec_input = vec_.input();
    vec_input.V_ref = v_ref;
    vec_input.I_ref = i_ref_;
    vec_input.V_cap_up = sample.V_cap_up.toFloat();
    vec_input.V_cap_dn = sample.V_cap_dn.toFloat();

    if (vec_.compute()) {
      const auto& out = vec_.output();
      control.Duty_IGBT_up = Duty::fromFloat(out.Duty_up);
      control.Duty_IGBT_dn = Duty::fromFloat(out.Duty_dn);
    }
  }

  void setIRef(float i_ref) { i_ref_ = i_ref; }
  void setTs(float ts) {
    pi_.setTs(ts);
    vec_.setTs(ts);
  }

private:
  PI pi_;
  Vec vec_;
  float i_ref_;
  const float iref_slope_ts_;
  bool soft_started_;
};
