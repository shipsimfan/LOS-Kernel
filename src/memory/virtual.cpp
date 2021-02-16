#include <memory/virtual.h>

namespace Memory { namespace Virtual {
    extern "C" void InitVirtualMemory() {}

    void Allocate(VirtualAddress virt, PhysicalAddress phys) {}
    void Allocate(VirtualAddress virt) {}

    void Free(VirtualAddress virt) {}

    PhysicalAddress CreateEmptyAddressSpace() { return 0; }
    void SetCurrentAddressSpace(PhysicalAddress addr) {}
}} // namespace Memory::Virtual