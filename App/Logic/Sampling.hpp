#pragma once
#include "Config/SystemConfig.hpp"
#include "Data/MainCirData.hpp"
#include "Utils/Units/Units.hpp"

class Sampling {

public:
  Sampling() = default;

  void attachBuffer(const uint16_t* buf, size_t len) {
    adcBuffer_ = buf;
    adcLen_ = len;
  }

  void update(MainCirData& data) {
    // 映射 DMA 缓冲 -> 物理量，按 CubeMX Rank 顺序配置
    if (!adcBuffer_ || adcLen_ < 4) {
      return;
    }

    auto& s = data.sample;

    const auto& cal = SystemConfig::ADC_CALIB;

    s.V_bus = Volt::fromFloat(adcBuffer_[0] * cal[0].gain + cal[0].offset);
    s.V_cap_up = Volt::fromFloat(adcBuffer_[1] * cal[1].gain + cal[1].offset);
    s.V_cap_dn = Volt::fromFloat(adcBuffer_[2] * cal[2].gain + cal[2].offset);
    s.I_bus = Curr::fromFloat(adcBuffer_[3] * cal[3].gain + cal[3].offset);

    // 备用通道原始值暂存
    if (adcLen_ > 4) s.ch4_resv = adcBuffer_[4];
    if (adcLen_ > 5) s.ch5_resv = adcBuffer_[5];
  }

private:
  const uint16_t* adcBuffer_ = nullptr;
  size_t adcLen_ = 0;
};
