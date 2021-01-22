#include <mem/heap.h>

#include <mem/physical.h>
#include <stdio.h>

extern "C" void* malloc(size_t size) { return MemoryManager::Heap::malloc(size); }
extern "C" void free(void* ptr) { MemoryManager::Heap::free(ptr); }

namespace MemoryManager { namespace Heap {
    uint64_t top;

    void Init() { top = Physical::GetNumPages() * PAGE_SIZE + KERNEL_VMA; }

    void* malloc(size_t size) {
        void* ret = (void*)top;
        top += size;
        return ret;
    }

    void free(void* ptr) {}
}} // namespace MemoryManager::Heap