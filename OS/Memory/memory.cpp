#include "memory.hpp"

namespace Memory {

extern "C" uint8_t __itcm_code_load_start;
extern "C" uint8_t __itcm_code_start;
extern "C" uint8_t __itcm_code_end;

void CopySram2Data() {
  uint8_t* src = __sram2_data_source;
  uint8_t* dst = __sram2_data_start;

  while (dst < __sram2_data_end) {
    *dst++ = *src++;
  }
}

void CopyItcmCode() {
  uint8_t* src = &__itcm_code_load_start;
  uint8_t* dst = &__itcm_code_start;
  uint8_t* end = &__itcm_code_end;

  while (dst < end) {
    *dst++ = *src++;
  }
}

} // namespace Memory
