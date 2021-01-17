#include <mem/heap.h>

MemoryManager::Heap* instance;

void* malloc(size_t size) { return instance->malloc(size); }
void free(void* ptr) { instance->free(ptr); }

namespace MemoryManager {
    void Heap::Init(Physical* physicalMem) {
        instance = this;
        top = physicalMem->GetNumPages() * PAGE_SIZE + KERNEL_VMA;
    }

    void* Heap::malloc(size_t size) {
        void* ret = (void*)top;
        top += size;
        return ret;
    }

    void Heap::free(void* ptr) {}
} // namespace MemoryManager