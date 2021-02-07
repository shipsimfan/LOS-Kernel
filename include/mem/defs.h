#pragma once

#include <stdint.h>

// Sizes
#define KILOBYTE (size_t)1024
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)
#define TERABYTE (1024 * GIGABYTE)

#define PAGE_SIZE (4 * KILOBYTE)

extern uint64_t KERNEL_SIZE;

// Addresses
extern uint64_t KERNEL_LMA;
extern uint64_t KERNEL_VMA;
extern uint64_t KERNEL_BOTTOM;
extern uint64_t KERNEL_TOP;

// Types
typedef uint64_t physAddr_t;
typedef void* virtAddr_t;

// Linker Symbols
extern uint64_t __KERNEL_LMA;
extern uint64_t __KERNEL_VMA;
extern uint64_t __KERNEL_BOTTOM;
extern uint64_t __KERNEL_TOP;