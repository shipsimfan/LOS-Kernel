#pragma once

#include <memory/defs.h>

#define MAXIMUM_SYSTEM_MEMORY (128 * GIGABYTE)
#define PHYSICAL_BITMAP_SIZE (MAXIMUM_SYSTEM_MEMORY / (PAGE_SIZE * 64))

#pragma pack(push)
#pragma pack(1)

struct MemoryMap {
    uint64_t size;
    uint64_t key;
    uint64_t descSize;
    uint32_t descVersion;
    uint32_t reserved;
    uint64_t mapAddr;
};

#pragma pack(pop)

enum MemoryType : uint32_t { RESERVED, LOADER_CODE, LOADER_DATA, BOOT_SERVICES_CODE, BOOT_SERVICES_DATA, RUNTIME_SERVICES_CODE, RUNTIME_SERVICES_DATA, CONVENTIONAL, UNUSABLE, ACPI_RECLAIM, ACPI_NVS, MMIO, MMIO_PORT, PAL_CODE, PERSISTENT, MAX };

struct MemoryDescriptor {
    MemoryType type;
    uint64_t physicalAddress;
    uint64_t virtualAddress;
    uint64_t numPages;
    uint64_t attribute;
};
