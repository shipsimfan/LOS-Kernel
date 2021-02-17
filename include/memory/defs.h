#pragma once

#include <stdint.h>

#define KILOBYTE (uint64_t)1024
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)
#define TERABYTE (1024 * GIGABYTE)

#define PAGE_SIZE (4 * KILOBYTE)

#define KERNEL_LMA 0x100000
#define KERNEL_VMA 0xFFFF800000000000

extern uint64_t KERNEL_BOTTOM;
extern uint64_t KERNEL_TOP;
extern uint64_t KERNEL_SIZE;

typedef void* VirtualAddress;
typedef uint64_t PhysicalAddress;