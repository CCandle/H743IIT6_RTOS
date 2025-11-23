#pragma once
#include "Config/SystemConfig.hpp"
#include "Data/MainCirData.hpp"
#include "Utils/Units/Units.hpp"
#include "tim.h"
#include <algorithm>

#define ITCM_FUNC_CTRL __attribute__((section(".itcm.text.controller")))

class Controller {
public:
  Controller() = default;

  ITCM_FUNC_CTRL void apply(MainCirData& data) {
    const auto& ctrl = data.control;
    float duty_up = ctrl.Duty_IGBT_up.toFloat();
    float duty_dn = 1.0f - ctrl.Duty_IGBT_dn.toFloat();

    const uint16_t ccr_up = static_cast<uint16_t>(std::clamp(duty_up, SystemConfig::PWM_MIN_DUTY, SystemConfig::PWM_MAX_DUTY) * SystemConfig::PWM_PERIOD);
    const uint16_t ccr_dn = static_cast<uint16_t>(std::clamp(duty_dn, SystemConfig::PWM_MIN_DUTY, SystemConfig::PWM_MAX_DUTY) * SystemConfig::PWM_PERIOD);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_up);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr_dn);
  }

  ITCM_FUNC_CTRL void shutdown() {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
  }

  ITCM_FUNC_CTRL void start() {
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, SystemConfig::PWM_PERIOD - 1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  }
};
