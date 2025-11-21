#pragma once
#include <algorithm>
#include <cmath>

class Vec {
  public:
    struct Input {
        float V_ref = 0.0f;
        float I_ref = 0.0f;
        float V_cap_up = 0.0f;
        float V_cap_dn = 0.0f;
    };

    struct Internal {
        float vec_large, vec_mid, vec_small;
        float t_large, t_mid, t_small;
        float t_delta;
        float t_up, t_dn;
    };

    struct Output {
        float Duty_up = 0.0f;
        float Duty_dn = 0.0f;
    };

    constexpr Vec(float cap = 50e-6f, float ts = 50e-6f)
        : Cap_(cap), Ts_(ts), invTs_(1.0f / ts) {}

    // 提供输入输出的引用访问
    inline Input& input() noexcept { return input_; }
    inline const Output& output() const noexcept { return output_; }
    inline Internal& internal() noexcept { return internal_; }

    inline bool compute() noexcept {
        const float inv_I_ref = 1.0f / input_.I_ref;

        internal_.vec_large = input_.V_cap_up + input_.V_cap_dn;
        internal_.vec_mid = 0.5f * internal_.vec_large;

        const float V_ref = input_.V_ref;

        if (V_ref >= internal_.vec_mid) {
            const float denom = internal_.vec_large - internal_.vec_mid;
            const float inv_denom = 1.0f / denom;
            internal_.t_large = Ts_ * (internal_.vec_large - V_ref) * inv_denom;
            internal_.t_mid = Ts_ * (V_ref - internal_.vec_mid) * inv_denom;
            internal_.t_small = 0.0f;
        } else {
            const float denom = internal_.vec_mid;
            const float inv_denom = 1.0f / denom;
            internal_.t_mid = Ts_ * V_ref * inv_denom;
            internal_.t_small = Ts_ * (internal_.vec_mid - V_ref) * inv_denom;
            internal_.t_large = 0.0f;
        }

        float& t_large = internal_.t_large;
        float& t_mid = internal_.t_mid;
        float& t_small = internal_.t_small;

        clamp(t_large, 0.0f, Ts_);
        clamp(t_mid, 0.0f, Ts_);
        clamp(t_small, 0.0f, Ts_);

        internal_.t_delta = (input_.V_cap_up - input_.V_cap_dn) * Cap_ * inv_I_ref;

        const float t_mid_small_sum = t_mid + t_small;
        internal_.t_up = 0.5f * (t_mid_small_sum + internal_.t_delta);
        internal_.t_dn = 0.5f * (t_mid_small_sum - internal_.t_delta);

        clamp(internal_.t_up, 0.0f, Ts_);
        clamp(internal_.t_dn, 0.0f, Ts_);

        output_.Duty_up = internal_.t_up * invTs_;
        output_.Duty_dn = internal_.t_dn * invTs_;

        return true;
    }

    inline void setTs(float ts) {
        Ts_ = ts;
        invTs_ = 1.0f / ts;
    }

  private:
    float Cap_;
    float Ts_;
    float invTs_;
    Input input_;
    Output output_;
    Internal internal_;

    static constexpr inline void clamp(float& v, float lo, float hi) noexcept {
        v = std::clamp(v, lo, hi);
    }
};
