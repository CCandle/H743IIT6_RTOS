#pragma once
#include "Core/rtos/EventGroup.hpp"
#include <cstdint>

enum class DeviceMode : uint8_t {
    Boot,
    Init,
    Standby,
    Running,
    Error,
    Shutdown
};

enum class Severity : uint8_t {
    None,
    Info,
    Warning,
    Critical
};

class SystemState {
  private:
    explicit SystemState(EventGroup& sysEvent) : sysEvent_(sysEvent), mode_(DeviceMode::Boot), severity_(Severity::None) {}

  public:
    void update() {
        EventBits_t bits = sysEvent_.getBits();

        if (bits & SYS_CRITICAL_MASK) {
            mode_ = DeviceMode::Error;
            severity_ = Severity::Critical;
        } else if (bits & SYS_WARNING_MASK) {
            severity_ = Severity::Warning;
        } else if (bits & SYS_INFO_MASK) {
            severity_ = Severity::Info;
        } else {
            severity_ = Severity::None;
        }
    }
    void setMode(DeviceMode mode) { mode_ = mode; }
    void setSeverity(Severity severity) { severity_ = severity; }
    DeviceMode mode() const { return mode_; }

    Severity severity() const { return severity_; }
    bool isCritical() { return severity_ == Severity::Critical; }

    static inline& instance() {
        static SystemState ss;
        return ss;
    }

  private:
    EventGroup& sysEvent_;
    DeviceMode mode_;
    Severity severity_;
};
