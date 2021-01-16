#include <mem/physical.h>

#include <logger.h>
#include <mem/defs.h>

namespace MemoryManager {
    bool Physical::Init(multiboot2BootInformation* bootInfo) {
        infoLogger.Log("Initializing physical memory manager . . .");

        // Search for the memory map tag in the boot information
        uint64_t bootInfoTop = (uint64_t)bootInfo + bootInfo->totalSize;
        uint64_t cur = (uint64_t)bootInfo + 8;
        multiboot2TagHeader* currentTag = (multiboot2TagHeader*)cur;
        while ((currentTag->type != 0 || currentTag->size != 8) && cur < bootInfoTop) {
            if (currentTag->type == 6)
                break;

            cur += ((currentTag->size / 8) * 8) + 8;
            currentTag = (multiboot2TagHeader*)cur;
        }

        if (currentTag->type != 6) {
            errorLogger.Log("Unable to find memory map boot information tag!");
            return false;
        }

        multiboot2TagMemoryMap* memMapTag = (multiboot2TagMemoryMap*)currentTag;

        // Find the top memory block
        uint64_t memMapTop = (uint64_t)memMapTag + memMapTag->header.size;
        multiboot2MemoryMapEntry* entry = &(memMapTag->entries);
        cur = (uint64_t)entry;
        while (cur < memMapTop) {
            cur += memMapTag->entrySize;
            entry = (multiboot2MemoryMapEntry*)cur;
        }
        cur -= memMapTag->entrySize;
        multiboot2MemoryMapEntry* topEntry = (multiboot2MemoryMapEntry*)cur;

        // Verify we can hold all the tables correctly
        if (topEntry->baseAddr + topEntry->length > MAXIMUM_SYSTEM_MEMORY) {
            errorLogger.Log("Too much system memory for this operating system.");
            return false;
        }

        // Prepare the physical memory map (Reserve everything)
        bitmapSize = (topEntry->baseAddr + topEntry->length + 1) / PAGE_SIZE / 64;
        for (size_t i = 0; i < bitmapSize; i++)
            bitmap[i] = 0xFFFFFFFFFFFFFFFF;

        numTotalPages = bitmapSize * 64;
        numFreePages = 0;

        // Free the areas that need to be freed
        entry = &(memMapTag->entries);
        cur = (uint64_t)entry;
        while (cur < memMapTop) {
            if (entry->type == 1) {
                size_t i = entry->baseAddr / PAGE_SIZE / 64;
                if (entry->baseAddr < nextFreePage)
                    nextFreePage = entry->baseAddr;

                for (size_t j = 0; j < entry->length; j += PAGE_SIZE * 64, i++) {
                    bitmap[i] = 0;
                    numFreePages += 64;
                }
            }

            cur += memMapTag->entrySize;
            entry = (multiboot2MemoryMapEntry*)cur;
        }

        // Reserve Kernel memory
        size_t i = KERNEL_LMA / PAGE_SIZE / 64;
        for (size_t j = 0; j < KERNEL_SIZE; j += PAGE_SIZE * 64, i++) {
            bitmap[i] = 0xFFFFFFFFFFFFFFFF;
            numFreePages -= 64;
        }

        // Reserve Video Memory
        i = 0xA0000 / PAGE_SIZE / 64;
        for (; i < 0xC0000 / PAGE_SIZE / 64; i++) {
            bitmap[i] = 0xFFFFFFFFFFFFFFFF;
            numFreePages -= 64;
        }

        // Physical bitmap is now ready
        infoLogger.Log("Total Free Memory: %i KB (%i MB)", (numFreePages * PAGE_SIZE) / KILOBYTE, (numFreePages * PAGE_SIZE) / MEGABYTE);
        infoLogger.Log("Physical Memory Manager Ready!");

        return true;
    }

    physAddr_t Physical::AllocNextFreePage() {
        physAddr_t nfp = nextFreePage;

        AllocPage(nextFreePage * PAGE_SIZE);

        return nfp * PAGE_SIZE;
    }

    bool Physical::IsPageFree(physAddr_t addr) {
        size_t i = addr / PAGE_SIZE / 64;
        size_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return false;

        return (bitmap[i] >> b) & 1 ? false : true;
    }

    void Physical::AllocPage(physAddr_t addr) {
        if (!IsPageFree(addr))
            return;

        size_t i = addr / PAGE_SIZE / 64;
        size_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return;

        numFreePages--;

        bitmap[i] |= 1 << b;

        if (addr / PAGE_SIZE == nextFreePage) {
            for (i = nextFreePage; i < bitmapSize * 64; i++) {
                if (IsPageFree(i * PAGE_SIZE)) {
                    nextFreePage = i;
                    break;
                }
            }
        }
    }

    void Physical::FreePage(physAddr_t addr) {
        if (IsPageFree(addr))
            return;

        size_t i = addr / PAGE_SIZE / 64;
        size_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return;

        numFreePages++;

        if (addr / PAGE_SIZE < nextFreePage)
            nextFreePage = addr / PAGE_SIZE;

        bitmap[i] &= ~(1 << b);
    }

    uint64_t Physical::GetNumFreePages() { return numFreePages; }

    uint64_t Physical::GetNumUsedPages() { return numTotalPages - numFreePages; }

    uint64_t Physical::GetNumPages() { return numTotalPages; }

} // namespace MemoryManager