#pragma once

#include <mem/defs.h>

#define MAXIMUM_SYSTEM_MEMORY (4 * GIGABYTE)
#define PHYSICAL_BITMAP_SIZE ((MAXIMUM_SYSTEM_MEMORY / PAGE_SIZE) / 8) / 8

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

namespace MemoryManager {
    struct MemoryDescriptor {
        uint32_t type;
        uint64_t physicalAddress;
        uint64_t virtualAddress;
        uint64_t numberOfPages;
        uint64_t attribute;
    };

    enum MemoryType { RESERVED, LOADER_CODE, LOADER_DATA, BOOT_SERVICES_CODE, BOOT_SERVICES_DATA, RUNTIME_SERVICES_CODE, RUNTIME_SERVICES_DATA, CONVENTIONAL, UNUSABLE, ACPI_RECLAIM, ACPI_NVS, MMIO, MMIO_PORT, PAL_CODE, PERSISTENT, MAX };

    namespace Physical {
        bool Init(MemoryMap* mmap);

        physAddr_t AllocNextFreePage();

        bool IsPageFree(physAddr_t addr);
        void AllocPage(physAddr_t addr);
        void FreePage(physAddr_t addr);

        uint64_t GetNumFreePages();
        uint64_t GetNumUsedPages();
        uint64_t GetNumPages();
    }; // namespace Physical
} // namespace MemoryManager