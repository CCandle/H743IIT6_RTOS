#pragma once
#include <cstdint>

namespace LT7680Cmd {
    constexpr uint8_t CMD_PLL_1 = 0x05; // vendor uses 0x05/0x06..0x0A for PLL config
    // ... (we keep minimal names; concrete register writes are done by methods)
}
