#include <memory/virtual.h>

#include <bootloader.h>
#include <interrupt/exception.h>
#include <memory/physical.h>
#include <panic.h>
#include <string.h>

#include "virtual.h"

template <class T> void PageTableBase<T>::SetEntry(int index, PhysicalAddress addr, bool write, bool supervisor) {
    if (index >= 512)
        return;

    uint64_t entry = addr & ~(PAGE_SIZE - 1);
    entry |= PAGE_PRESENT;
    if (write)
        entry |= PAGE_WRITE;
    if (supervisor)
        entry |= PAGE_SUPERVISOR;

    entries[index] = entry;
}

template <class T> void PageTableBase<T>::ClearEntry(int index) {
    if (index >= 512)
        return;

    entries[index] = 0;
}

namespace Memory { namespace Virtual {
    PML4* kernelPML4;
    PML4* currentPML4;

    void PageFaultHandler(Interrupt::Registers regs, Interrupt::ExceptionInfo info) {
        uint64_t cr2 = GetCR2();

        if ((info.errorCode & 1) == 0) {
            // Using the first page of virtual memory to detect null pointer exceptions
            // Meaning you can't use the first page of virtual memory. Hopefully you didn't need those 4 kilobytes
            if (cr2 < PAGE_SIZE)
                panic("Null Pointer Exception at %#llx (Faulting Address: %#llx)", info.rip, cr2);
            else {
                if (currentPML4 != kernelPML4 || cr2 >= KERNEL_VMA)
                    Allocate((VirtualAddress)cr2, Physical::Allocate());
                else
                    panic("Page fault for access in user address space!\n    Fault Address: %#llX\n     Fault Instruction: %#llX\n    Error Code: %#X\n", cr2, info.rip, info.errorCode);
            }
        } else
            panic("Page Protection Fault!\n    Fault Address: %#llX\n    Fault Instruction: %#llX\n    Error Code: %#X", cr2, info.rip, info.errorCode);
    }

    extern "C" void InitVirtualMemory() {
        kernelPML4 = (PML4*)(Physical::Allocate() + KERNEL_VMA);
        currentPML4 = kernelPML4;
        memset(kernelPML4, 0, PAGE_SIZE);

        // Allocate the PDPTs
        for (uint64_t i = 256; i < 512; i++) {
            kernelPML4->SetEntry(i, Physical::Allocate(), true, true);
            memset(kernelPML4->GetEntry(i), 0, PAGE_SIZE);
        }

        // Allocate pages
        MemoryDescriptor* desc;
        for (uint64_t ptr = mmap->mapAddr; ptr < mmap->mapAddr + mmap->size; ptr += mmap->descSize) {
            desc = (MemoryDescriptor*)ptr;

            PhysicalAddress paddr = desc->physicalAddress;
            for (uint64_t i = 0; i < desc->numPages; i++, paddr += PAGE_SIZE)
                Allocate((VirtualAddress)(paddr + KERNEL_VMA), paddr);
        }

        // Allocate framebuffer
        PhysicalAddress paddr = gopInfo->frameBufferBase;
        for (uint64_t i = 0; paddr < gopInfo->frameBufferBase + gopInfo->frameBufferSize; i++, paddr += PAGE_SIZE)
            Allocate((VirtualAddress)(paddr + KERNEL_VMA), paddr);

        // Set the pml4
        SetCurrentPML4((uint64_t)kernelPML4 - KERNEL_VMA);

        // Set page fault handler
        if (!Interrupt::InstallExceptionHandler(Interrupt::ExceptionType::PAGE_FAULT, PageFaultHandler))
            panic("Unable to set page fault handler!");
    }

    void InvalidatePage(VirtualAddress addr) { asm volatile("invlpg (%0)" ::"r"((uint64_t)addr) : "memory"); }

    void VirtualToIndex(VirtualAddress addr, int& pml4Index, int& pdptIndex, int& pdIndex, int& ptIndex, int& offset) {
        uint64_t corAddr = (uint64_t)addr & 0x0000FFFFFFFFFFFF;
        offset = corAddr & 0xFFF;
        ptIndex = (corAddr >> 12) & 0x1FF;
        pdIndex = (corAddr >> 21) & 0x1FF;
        pdptIndex = (corAddr >> 30) & 0x1FF;
        pml4Index = (corAddr >> 39) & 0x1FF;
    }

    void Allocate(VirtualAddress virt, PhysicalAddress phys) {
        int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
        VirtualToIndex(virt, pml4Index, pdptIndex, pdIndex, ptIndex, offset);

        bool supervisor = (uint64_t)virt < KERNEL_VMA;
        // Check PDPT
        if ((currentPML4->entries[pml4Index] & 1) == 0) {
            // Allocate new PDPT
            currentPML4->SetEntry(pml4Index, Physical::Allocate(), true, supervisor);

            // Clear the new PDPT
            memset(currentPML4->GetEntry(pml4Index), 0, PAGE_SIZE);
        }
        PDPT* pdpt = currentPML4->GetEntry(pml4Index);

        // Check PD
        if ((pdpt->entries[pdptIndex] & 1) == 0) {
            // Allocate new PD
            pdpt->SetEntry(pdptIndex, Physical::Allocate(), true, supervisor);

            // Clear the new PD
            memset(pdpt->GetEntry(pdptIndex), 0, PAGE_SIZE);
        }
        PageDirectory* pd = pdpt->GetEntry(pdptIndex);

        // Check PT
        if ((pd->entries[pdIndex] & 1) == 0) {
            // Allocate new PT
            pd->SetEntry(pdIndex, Physical::Allocate(), true, supervisor);
            // Clear the new PDPT
            memset(pd->GetEntry(pdIndex), 0, PAGE_SIZE);
        }
        PageTable* pt = pd->GetEntry(pdIndex);

        // Allocate Page
        if ((pt->entries[ptIndex] & 1) == 0)
            pt->SetEntry(ptIndex, phys, true, supervisor);
    }

    void Allocate(VirtualAddress virt) { Allocate(virt, Physical::Allocate()); }

    void Free(VirtualAddress virt) {
        int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
        VirtualToIndex(virt, pml4Index, pdptIndex, pdIndex, ptIndex, offset);
        if (currentPML4->entries[pml4Index] != 0) {
            PDPT* pdpt = currentPML4->GetEntry(pml4Index);
            if (pdpt->entries[pdptIndex] != 0) {
                PageDirectory* pd = pdpt->GetEntry(pdIndex);
                if (pd->entries[ptIndex] != 0) {
                    PageTable* pt = pd->GetEntry(ptIndex);
                    Physical::Free(pt->entries[ptIndex] & 0xFFFFFFFFFFFFF000);
                    pt->ClearEntry(ptIndex);
                }
            }
        }
    }

    PhysicalAddress CreateEmptyAddressSpace() { return 0; }
    void SetCurrentAddressSpace(PhysicalAddress addr) {}
}} // namespace Memory::Virtual