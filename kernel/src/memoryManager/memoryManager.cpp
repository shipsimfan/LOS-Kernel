#include <mem.h>

#include <logger.h>
#include <mem/heap.h>
#include <mem/virtual.h>

MemoryManager::Physical physicalMem;
MemoryManager::Virtual virtualMem;
MemoryManager::Heap heap;

uint64_t KERNEL_LMA;
uint64_t KERNEL_VMA;
uint64_t KERNEL_BOTTOM;
uint64_t KERNEL_TOP;
uint64_t KERNEL_SIZE;

bool MemoryManager::Init(multiboot2BootInformation* bootInfo) {
    infoLogger.Log("Initializing memory manager . . .");

    KERNEL_LMA = (uint64_t)&__KERNEL_LMA;
    KERNEL_VMA = (uint64_t)&__KERNEL_VMA;
    KERNEL_BOTTOM = (uint64_t)&__KERNEL_BOTTOM;
    KERNEL_TOP = (uint64_t)&__KERNEL_TOP;
    KERNEL_SIZE = KERNEL_TOP - KERNEL_BOTTOM;

    if (!physicalMem.Init(bootInfo)) {
        errorLogger.Log("Failed to initialize physical memory manager!");
        return false;
    }

    if (!virtualMem.Init(&physicalMem)) {
        errorLogger.Log("Failed to initialize virtual memory manager!");
        return false;
    }

    heap.Init(&physicalMem);

    return true;
}