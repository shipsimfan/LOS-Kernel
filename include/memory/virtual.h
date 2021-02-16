#pragma once

#include <memory/defs.h>

namespace Memory { namespace Virtual {
    void Allocate(VirtualAddress virt, PhysicalAddress phys);
    void Allocate(VirtualAddress virt);

    void Free(VirtualAddress virt);

    PhysicalAddress CreateEmptyAddressSpace();
    void SetCurrentAddressSpace(PhysicalAddress addr);
}} // namespace Memory::Virtual