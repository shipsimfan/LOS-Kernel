#pragma once

#include <memory/defs.h>

#define MAXIMUM_SYSTEM_MEMORY (128 * GIGABYTE)
#define PHYSICAL_BITMAP_SIZE (MAXIMUM_SYSTEM_MEMORY / (PAGE_SIZE * 64))