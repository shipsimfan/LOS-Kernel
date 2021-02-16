#include <memory/physical.h>

namespace Memory { namespace Physical {
    extern "C" void InitPhysicalMemory() {}

    void Allocate(PhysicalAddress addr) {}
    void Allocate() {}

    void Free(PhysicalAddress addr) {}

    uint64_t GetTotalPages() { return 0; }
    uint64_t GetFreePages() { return 0; }
}} // namespace Memory::Physical