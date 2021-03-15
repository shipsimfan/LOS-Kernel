#pragma once

#include <memory/defs.h>

#include <mutex.h>

namespace Memory { namespace Virtual {
    void Allocate(VirtualAddress virt, PhysicalAddress phys);
    void Allocate(VirtualAddress virt);

    void Free(VirtualAddress virt);

    PhysicalAddress CreateAddressSpace();
    void DeletePagingStructure(PhysicalAddress structure);

    void SetCurrentAddressSpace(PhysicalAddress addr, Mutex* mutex);

    PhysicalAddress GetKernelPagingStructure();
}} // namespace Memory::Virtual