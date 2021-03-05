#pragma once

#include <stdint.h>

typedef uint64_t time_t;

time_t GetCurrentTime();

void Sleep(time_t milliseconds);