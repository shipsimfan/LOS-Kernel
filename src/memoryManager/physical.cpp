#include <mem/physical.h>

#include <logger.h>
#include <mem/defs.h>

namespace MemoryManager { namespace Physical {
    size_t bitmapSize;
    uint64_t bitmap[PHYSICAL_BITMAP_SIZE];

    size_t nextFreePage = 0xFFFFFFFFFFFFFFFF;

    uint64_t numFreePages;
    uint64_t numTotalPages;

    bool Init(MemoryMap* mmap) {
        infoLogger.Log("Initializing physical memory manager . . .");

        // Prepare the physical memory map (Reserve everything)
        MemoryDescriptor* topEntry = (MemoryDescriptor*)(mmap->mapAddr + mmap->size - mmap->descSize);

        bitmapSize = ((topEntry->physicalAddress / PAGE_SIZE) + topEntry->numberOfPages) / 64;
        if (bitmapSize > MAXIMUM_SYSTEM_MEMORY / PAGE_SIZE / 64) {
            errorLogger.Log("Too much memory!");
            return false;
        }

        for (size_t i = 0; i < bitmapSize; i++)
            bitmap[i] = 0xFFFFFFFFFFFFFFFF;

        numTotalPages = bitmapSize * 64;
        numFreePages = 0;

        // Free the areas that need to be freed
        MemoryDescriptor* desc;
        for (uint64_t ptr = mmap->mapAddr; ptr < mmap->mapAddr + mmap->size; ptr += mmap->descSize) {
            desc = (MemoryDescriptor*)ptr;

            if (desc->type == MemoryType::BOOT_SERVICES_CODE || desc->type == MemoryType::BOOT_SERVICES_DATA || desc->type == MemoryType::CONVENTIONAL || desc->type == MemoryType::PERSISTENT) {
                size_t i = desc->physicalAddress / PAGE_SIZE / 64;
                if (desc->physicalAddress < nextFreePage)
                    nextFreePage = desc->physicalAddress;

                for (size_t j = 0; j < desc->numberOfPages; j += 64, i++) {
                    bitmap[i] = 0;
                    numFreePages += 64;
                }
            }
        }

        // Allocate the kernel
        uint64_t base = KERNEL_LMA / PAGE_SIZE / 64;
        for (uint64_t i = 0; i <= KERNEL_SIZE / PAGE_SIZE / 64; i++)
            bitmap[i + base] = 0xFFFFFFFFFFFFFFFF;

        // Physical bitmap is now ready
        infoLogger.Log("Total Free Memory: %i KB (%i MB)", (numFreePages * PAGE_SIZE) / KILOBYTE, (numFreePages * PAGE_SIZE) / MEGABYTE);
        infoLogger.Log("Physical memory manager initialized!");

        return true;
    }

    physAddr_t AllocNextFreePage() {
        physAddr_t nfp = nextFreePage;

        AllocPage(nextFreePage * PAGE_SIZE);

        return nfp * PAGE_SIZE;
    }

    bool IsPageFree(physAddr_t addr) {
        size_t i = addr / PAGE_SIZE / 64;
        size_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return false;

        return (bitmap[i] >> b) & 1 ? false : true;
    }

    void AllocPage(physAddr_t addr) {
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

    void FreePage(physAddr_t addr) {
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

    uint64_t GetNumFreePages() { return numFreePages; }

    uint64_t GetNumUsedPages() { return numTotalPages - numFreePages; }

    uint64_t GetNumPages() { return numTotalPages; }

}} // namespace MemoryManager::Physical