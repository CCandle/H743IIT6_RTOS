#pragma once
#include "FreeRTOS.h"

extern uint8_t __sram2_data_source[];
extern uint8_t __sram2_data_start[];
extern uint8_t __sram2_data_end[];

namespace Memory {
__attribute__((section(".dtcm.heap"))) static uint8_t DTCMHeap[16 * 1024];
__attribute__((section(".axi.heap"))) static uint8_t AXIHeap[64 * 1024];
__attribute__((section(".sram2.heap"))) static uint8_t D2Heap[64 * 1024];
__attribute__((section(".d3.heap"))) static uint8_t D3Heap[16 * 1024];

static HeapRegion_t MemoryRegions[] =
    {
        {DTCMHeap, sizeof(DTCMHeap)},
        {AXIHeap, sizeof(AXIHeap)},
        {D2Heap, sizeof(D2Heap)},
        {D3Heap, sizeof(D3Heap)},
        {nullptr, 0} // Terminator
};

void CopySram2Data(void);

inline void Init() {
  CopySram2Data();
  vPortDefineHeapRegions(MemoryRegions);
}

} // namespace Memory
