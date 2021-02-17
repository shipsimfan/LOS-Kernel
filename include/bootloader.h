#pragma once

#include <stdint.h>

enum MemoryType : uint32_t { RESERVED, LOADER_CODE, LOADER_DATA, BOOT_SERVICES_CODE, BOOT_SERVICES_DATA, RUNTIME_SERVICES_CODE, RUNTIME_SERVICES_DATA, CONVENTIONAL, UNUSABLE, ACPI_RECLAIM, ACPI_NVS, MMIO, MMIO_PORT, PAL_CODE, PERSISTENT, MAX };

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

struct GOPInfo {
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t pixelFormat;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t reserved;
    uint32_t pixelsPerScanline;
    uint64_t frameBufferBase;
    uint64_t frameBufferSize;
};

#pragma pack(pop)

struct MemoryDescriptor {
    MemoryType type;
    uint64_t physicalAddress;
    uint64_t virtualAddress;
    uint64_t numPages;
    uint64_t attribute;
};

extern "C" MemoryMap* mmap;
extern "C" GOPInfo* gopInfo;
