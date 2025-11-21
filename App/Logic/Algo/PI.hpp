#pragma once

class PI {
  public:
    struct Input {
        float ref;
        float fdb;
    };

    struct Internal {
        float err;
        float p_term;
        float i_inc;
        float out_pre;
        float integral;
    };

    struct Output {
        float out;
    };

  public:
    PI(float kp = 0.0f, float ki = 0.0f,
       float max = -1.0f, float min = 1.0f,
       float ts = 0.001f)
        : Kp(kp), Ki(ki),
          limit_max(max), limit_min(min),
          Ts(ts), invTs(1.0f / ts) {
        internal_.integral = 0.0f;
        internal_.out_pre = 0.0f;
    }

    inline void reset() {
        internal_.integral = 0.0f;
        internal_.out_pre = 0.0f;
    }

    // 提供输入输出的引用访问
    inline Input& input() noexcept { return input_; }
    inline const Output& output() const noexcept { return output_; }
    inline Internal& internal() noexcept { return internal_; }

    inline bool compute() {
        float& integral = internal_.integral;

        internal_.err = input_.ref - input_.fdb;
        internal_.p_term = Kp * internal_.err;
        internal_.i_inc = Ki * internal_.err * Ts;
        internal_.out_pre = internal_.p_term + integral + internal_.i_inc;

        if (internal_.out_pre <= limit_max && internal_.out_pre >= limit_min) {
            integral += internal_.i_inc;
        }

        output_.out = internal_.p_term + integral;

        if (output_.out > limit_max) {
            output_.out = limit_max;
        } else if (output_.out < limit_min) {
            output_.out = limit_min;
        }

        return true;
    }

    inline void setTs(float ts) { Ts = ts; }

  private:
    float Kp, Ki;
    float limit_max;
    float limit_min;
    float Ts;
    float invTs;

    Input input_;
    Internal internal_;
    Output output_;
};