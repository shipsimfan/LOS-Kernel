#include <memory/heap.h>

#include <memory/defs.h>
#include <panic.h>
#include <stddef.h>

#include "physical.h"

void* operator new(size_t size) { return Memory::Heap::Allocate(size); }

void* operator new[](size_t size) { return Memory::Heap::Allocate(size); }

void operator delete(void* p) { Memory::Heap::Free(p); }

void operator delete[](void* p) { Memory::Heap::Free(p); }

extern "C" void __cxa_throw_bad_array_new_length() { panic("Bad Array Length!"); }

namespace Memory { namespace Heap {
    uint64_t top;

    extern "C" void InitHeap() { top = KERNEL_VMA + 0x100000000000; }

    void* Allocate(uint64_t size) {
        void* ret = (void*)top;
        top += size;
        return ret;
    }

    void* AllocateAligned(uint64_t size, uint64_t alignment) {
        if (top % alignment == 0)
            return Allocate(size);

        void* ret = (void*)top;
        top = top - (top % alignment) + alignment;
        return ret;
    }

    void Free(void* ptr) {}
}} // namespace Memory::Heap