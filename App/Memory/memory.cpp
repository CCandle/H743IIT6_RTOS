#include "memory.hpp"

namespace Memory {

void CopySram2Data() {
  uint8_t* src = __sram2_data_source;
  uint8_t* dst = __sram2_data_start;

  while (dst < __sram2_data_end) {
    *dst++ = *src++;
  }
}

} // namespace Memory
