#pragma once
#include "FreeRTOS.h"

// #define SECTION_ITCM __attribute__((section(".itcm")))
// #define SECTION_ITCM_FUNC __attribute__((noinline, section(".itcm.code"), used))
// #define SECTION_DTCM __attribute__((section(".dtcm")))
// #define SECTION_AXI __attribute__((section(".axi")))
// #define SECTION_D2 __attribute__((section(".d2")))
// #define SECTION_D3 __attribute__((section(".d3")))

// extern uint8_t __start_itcm_code[], __end_itcm_code[];
// extern uint8_t __start_dtcm_bss[], __end_dtcm_bss[];
// extern uint8_t __start_axi_bss[], __end_axi_bss[];
// extern uint8_t __start_d2_bss[], __end_d2_bss[];
// extern uint8_t __start_d3_bss[], __end_d3_bss[];

namespace Memory {
__attribute__((section(".dtcm.bss"))) static uint8_t DTCMHeap[16 * 1024];
__attribute__((section(".axi.bss"))) static uint8_t AXIHeap[32 * 1024];
__attribute__((section(".d2.bss"))) static uint8_t D2Heap[8 * 1024];
__attribute__((section(".d3.bss"))) static uint8_t D3Heap[16 * 1024];

static HeapRegion_t MemoryRegions[] =
    {
        {DTCMHeap, sizeof(DTCMHeap)},
        {AXIHeap, sizeof(AXIHeap)},
        {D2Heap, sizeof(D2Heap)},
        {D3Heap, sizeof(D3Heap)},
        {nullptr, 0} // Terminator
};

// void ZeroCustomBss() {
//   auto clear = [](uint8_t* start, uint8_t* end) {
//     for (; start < end; ++start)
//       *start = 0;
//   };
//   clear(__start_dtcm_bss, __end_dtcm_bss);
//   clear(__start_axi_bss, __end_axi_bss);
//   clear(__start_d2_bss, __end_d2_bss);
//   clear(__start_d3_bss, __end_d3_bss);
// }

inline void Init() {
  vPortDefineHeapRegions(MemoryRegions);
}

} // namespace Memory
