#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sleep(uint64_t milliseconds);
uint64_t getCurrentTimeMillis();

#ifdef __cplusplus
}
#endif