#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void* malloc(size_t size);
extern void free(void* ptr);

#ifdef __cplusplus
}
#endif