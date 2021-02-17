#pragma once

#include <stdint.h>

extern "C" bool CompareExchange(void* val, uint64_t compare, uint64_t newValue);