#pragma once

#include <stdint.h>

namespace Memory { namespace Heap {
    void* Allocate(uint64_t size);
    void* AllocateAligned(uint64_t size, uint64_t alignment);

    void Free(void* ptr);
}} // namespace Memory::Heap