#include <memory/heap.h>

#include <memory/defs.h>
#include <mutex.h>
#include <panic.h>
#include <stddef.h>

#include "physical.h"

void* operator new(size_t size) { return Memory::Heap::Allocate(size); }
void* operator new[](size_t size) { return Memory::Heap::Allocate(size); }

void operator delete(void* p) { Memory::Heap::Free(p); }
void operator delete(void* p, unsigned long) { Memory::Heap::Free(p); }

void operator delete[](void* p) { Memory::Heap::Free(p); }
void operator delete[](void* p, unsigned long) { Memory::Heap::Free(p); }

namespace Memory { namespace Heap {
    uint64_t top;
    Mutex topMutex;

    extern "C" void InitHeap() { top = KERNEL_VMA + 0x100000000000; }

    void* Allocate(uint64_t size) {
        topMutex.Lock();
        void* ret = (void*)top;
        top += size;
        topMutex.Unlock();

        return ret;
    }

    void* AllocateAligned(uint64_t size, uint64_t alignment) {
        topMutex.Lock();
        if (top % alignment == 0) {
            topMutex.Unlock();
            return Allocate(size);
        }

        top = top - (top % alignment) + alignment;
        topMutex.Unlock();

        return Allocate(size);
    }

    void Free(void* ptr) {}
}} // namespace Memory::Heap