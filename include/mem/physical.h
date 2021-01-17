#pragma once

#include <mem/defs.h>
#include <multiboot2.h>

#define MAXIMUM_SYSTEM_MEMORY (4 * GIGABYTE)
#define PHYSICAL_BITMAP_SIZE ((MAXIMUM_SYSTEM_MEMORY / PAGE_SIZE) / 8) / 8

namespace MemoryManager { namespace Physical {
    bool Init(multiboot2BootInformation* bootInfo);

    physAddr_t AllocNextFreePage();

    bool IsPageFree(physAddr_t addr);
    void AllocPage(physAddr_t addr);
    void FreePage(physAddr_t addr);

    uint64_t GetNumFreePages();
    uint64_t GetNumUsedPages();
    uint64_t GetNumPages();
}; } // namespace MemoryManager::Physical