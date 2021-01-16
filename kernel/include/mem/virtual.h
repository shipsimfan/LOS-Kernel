#pragma once

#include <mem/physical.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_SUPERVISOR (1 << 2)

namespace MemoryManager {
#pragma pack(push)
#pragma pack(1)

    template <class T> struct PageTableBase {
        uint64_t entries[512];

        inline T GetEntry(int i) { return (T)((entries[i] & ~(PAGE_SIZE - 1)) + KERNEL_VMA); }
        void SetEntry(int index, physAddr_t addr, bool write, bool supervisor);
        void ClearEntry(int index);
    };

    struct PT : public PageTableBase<uint64_t> {};

    struct PD : public PageTableBase<PT*> {};

    struct PDPT : public PageTableBase<PD*> {};

    struct PML4 : public PageTableBase<PDPT*> {};

#pragma pack(pop)

    extern "C" PML4* GetCurrentPML4();
    extern "C" void SetCurrentPML4(physAddr_t pml4);
    void InvalidatePage(virtAddr_t addr);

    class Virtual {
    public:
        bool Init(Physical* physicalMem);

        virtAddr_t AllocNextKPage();
        virtAddr_t AllocNextKPage(physAddr_t& physAddr);

        void AllocatePage(virtAddr_t virtAddr, physAddr_t physAddr, bool write);
        void FreePage(virtAddr_t addr);

    private:
        void VirtualToIndex(virtAddr_t addr, int& pml4Index, int& pdptIndex, int& pdIndex, int& ptIndex, int& offset);
        virtAddr_t IndexToVirtual(uint64_t pml4Index, uint64_t pdptIndex, uint64_t pdIndex, uint64_t ptIndex, uint64_t offset);

        Physical* physicalMem;

        PML4* kernelPML4;

        PML4* currentPML4 = nullptr;
    };
} // namespace MemoryManager