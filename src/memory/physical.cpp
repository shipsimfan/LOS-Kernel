#include <memory/physical.h>

#include <mutex.h>
#include <panic.h>

#include "physical.h"

extern uint64_t __KERNEL_TOP;
extern uint64_t __KERNEL_BOTTOM;

uint64_t KERNEL_TOP;
uint64_t KERNEL_BOTTOM;
uint64_t KERNEL_SIZE;

namespace Memory { namespace Physical {
    uint64_t bitmapSize;
    uint64_t bitmap[PHYSICAL_BITMAP_SIZE];
    Mutex bitmapMutex;

    PhysicalAddress nextFreePage;

    uint64_t numFreePages;
    uint64_t numTotalPages;

    extern "C" MemoryMap* mmap;

    extern "C" void InitPhysicalMemory() {
        // Setup linker symbols
        KERNEL_TOP = (uint64_t)&__KERNEL_TOP;
        KERNEL_BOTTOM = (uint64_t)&__KERNEL_BOTTOM;
        KERNEL_SIZE = KERNEL_TOP - KERNEL_BOTTOM;

        // Prepare the bitmap
        MemoryDescriptor* topEntry = (MemoryDescriptor*)(mmap->mapAddr + mmap->size - mmap->descSize);

        bitmapSize = ((topEntry->physicalAddress / PAGE_SIZE) + topEntry->numPages) / 64;
        if (bitmapSize > PHYSICAL_BITMAP_SIZE)
            panic("Too much memory!");

        // Reserve everything
        for (uint64_t i = 0; i < bitmapSize; i++)
            bitmap[i] = 0xFFFFFFFFFFFFFFFF;

        bitmapMutex.Unlock();

        uint64_t usable = 0;
        uint64_t unusable = 0;

        MemoryDescriptor* desc;
        for (uint64_t ptr = mmap->mapAddr; ptr < mmap->mapAddr + mmap->size; ptr += mmap->descSize) {
            desc = (MemoryDescriptor*)ptr;

            switch (desc->type) {
            case MemoryType::RESERVED:
            case MemoryType::RUNTIME_SERVICES_CODE:
            case MemoryType::RUNTIME_SERVICES_DATA:
            case MemoryType::UNUSABLE:
            case MemoryType::ACPI_RECLAIM:
            case MemoryType::ACPI_NVS:
            case MemoryType::PAL_CODE:
                unusable += desc->numPages;

                break;

            case MemoryType::LOADER_CODE:
            case MemoryType::LOADER_DATA:
            case MemoryType::BOOT_SERVICES_CODE:
            case MemoryType::BOOT_SERVICES_DATA:
            case MemoryType::CONVENTIONAL:
            case MemoryType::PERSISTENT: {
                usable += desc->numPages;

                uint64_t phys = desc->physicalAddress;
                for (uint64_t i = 0; i < desc->numPages; i++, phys += PAGE_SIZE)
                    Free(phys);

                break;
            }

            default:
                break;
            }
        }

        numTotalPages = usable + unusable;
        numFreePages = usable;

        // Allocate the kernel
        for (uint64_t addr = KERNEL_LMA; addr < KERNEL_LMA + KERNEL_SIZE; addr += PAGE_SIZE)
            Allocate(addr);
    }

    void Allocate(PhysicalAddress addr) {
        uint64_t i = addr / (PAGE_SIZE * 64);
        uint64_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return;

        bitmapMutex.Lock();
        if ((bitmap[i] >> b) & 1) {
            bitmapMutex.Unlock();
            return;
        }

        numFreePages--;

        bitmap[i] |= 1 << b;

        if (addr == nextFreePage) {
            bool found = false;
            for (; i < bitmapSize && !found; i++) {
                for (; b < 64 && !found; b++) {
                    if (((bitmap[i] >> b) & 1) == 0) {
                        nextFreePage = (i * 64 + b) * PAGE_SIZE;
                        found = true;
                    }
                }

                b = 0;
            }
        }

        bitmapMutex.Unlock();
    }

    PhysicalAddress Allocate() {
        bitmapMutex.Lock();
        PhysicalAddress ret = nextFreePage;
        bitmapMutex.Unlock();

        Allocate(nextFreePage);

        return ret;
    }

    void Free(PhysicalAddress addr) {
        uint64_t i = addr / (PAGE_SIZE * 64);
        uint64_t b = 64 - ((addr / PAGE_SIZE) % 64);

        if (i >= bitmapSize)
            return;

        bitmapMutex.Lock();
        if (((bitmap[i] >> b) & 1) == 0) {
            bitmapMutex.Unlock();
            return;
        }

        numFreePages++;

        bitmap[i] &= ~(1 << b);

        bitmapMutex.Unlock();
    }

    uint64_t GetTotalPages() { return numTotalPages; }
    uint64_t GetFreePages() { return numFreePages; }
}} // namespace Memory::Physical