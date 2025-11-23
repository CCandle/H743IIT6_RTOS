#pragma once

#include "Fixed.hpp"
#include "Scale.hpp"

using Volt = Fixed<uint16_t, Scale::VOLTAGE>;
using Curr = Fixed<uint16_t, Scale::CURRENT>;
using Duty = Fixed<uint16_t, Scale::DUTY>;
using Temp = Fixed<uint16_t, Scale::TEMP, 1, 60>;