#pragma once

#include <memory/defs.h>

namespace Memory { namespace Physical {
    void Allocate(PhysicalAddress addr);
    PhysicalAddress Allocate();

    void Free(PhysicalAddress addr);

    bool IsFree(PhysicalAddress addr);

    uint64_t GetTotalPages();
    uint64_t GetFreePages();
}} // namespace Memory::Physical