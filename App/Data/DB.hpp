#pragma once
#include "Data/RingBuffer.hpp"
#include "MainCirData.hpp"
#include "memory.h"
namespace DB {
__attribute__((section(".axi.data"))) inline RingBuffer<MainCirDataRaw, 64> MainCirBuffer;
} // namespace DB
