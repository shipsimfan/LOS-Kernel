#include <mem.h>

#include <logger.h>
#include <mem/heap.h>
#include <mem/physical.h>
#include <mem/virtual.h>

uint64_t KERNEL_LMA;
uint64_t KERNEL_VMA;
uint64_t KERNEL_BOTTOM;
uint64_t KERNEL_TOP;
uint64_t KERNEL_SIZE;

bool MemoryManager::Init(MemoryMap* mmap) {
    infoLogger.Log("Initializing memory manager . . .");

    KERNEL_LMA = (uint64_t)&__KERNEL_LMA;
    KERNEL_VMA = (uint64_t)&__KERNEL_VMA;
    KERNEL_BOTTOM = (uint64_t)&__KERNEL_BOTTOM;
    KERNEL_TOP = (uint64_t)&__KERNEL_TOP;
    KERNEL_SIZE = KERNEL_TOP - KERNEL_BOTTOM;

    if (!Physical::Init(mmap)) {
        errorLogger.Log("Failed to initialize physical memory manager!");
        return false;
    }

    if (!Virtual::Init()) {
        errorLogger.Log("Failed to initialize virtual memory manager!");
        return false;
    }

    Heap::Init();

    infoLogger.Log("Memory manager initialized!");

    return true;
}