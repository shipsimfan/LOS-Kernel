#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* format, ...);

int vprintf(const char* format, va_list arg);

#ifdef __cplusplus
}
#endif