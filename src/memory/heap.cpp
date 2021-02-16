#include <memory/heap.h>

namespace Memory { namespace Heap {
    extern "C" void InitHeap() {}

    void* Allocate(uint64_t size) { return nullptr; }
    void* AllocateAligned(uint64_t size, uint64_t alignment) { return nullptr; }

    void Free(void* ptr) {}
}} // namespace Memory::Heap