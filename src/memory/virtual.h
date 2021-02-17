#pragma once

#include <memory/defs.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_SUPERVISOR (1 << 2)

template <class T> struct PageTableBase {
    uint64_t entries[512];

    inline T GetEntry(int i) { return (T)((entries[i] & ~(PAGE_SIZE - 1)) + KERNEL_VMA); }
    void SetEntry(int index, PhysicalAddress addr, bool write, bool supervisor);
    void ClearEntry(int index);
};

typedef PageTableBase<PhysicalAddress> PageTable;
typedef PageTableBase<PageTable*> PageDirectory;
typedef PageTableBase<PageDirectory*> PDPT;
typedef PageTableBase<PDPT*> PML4;

extern "C" void SetCurrentPML4(PhysicalAddress pml4);
extern "C" uint64_t GetCR2();