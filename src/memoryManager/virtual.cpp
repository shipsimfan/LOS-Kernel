#include <mem/virtual.h>

#include <console.h>
#include <interrupt.h>
#include <logger.h>
#include <mem/physical.h>
#include <stdlib.h>
#include <string.h>

namespace MemoryManager {
    extern "C" PML4* GetCurrentPML4();
    extern "C" void SetCurrentPML4(physAddr_t pml4);

    void InvalidatePage(virtAddr_t addr) { asm volatile("invlpg (%0)" ::"r"((uint64_t)addr) : "memory"); }

    template <class T> void PageTableBase<T>::SetEntry(int index, physAddr_t addr, bool write, bool supervisor) {
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

    namespace Virtual {
        PML4* kernelPML4;
        PML4* currentPML4;

        bool PageFaultHandler(InterruptHandler::CPUState cpu, InterruptHandler::StackState stack) {
            if ((stack.errorCode & 1) == 0) {
                // Using the first page of virtual memory to detect null pointer exceptions
                // Meaning you can't use the first page of virtual memory. Hopefully you didn't need those 4 kilobytes
                if (cpu.cr2 < PAGE_SIZE)
                    errorLogger.Log("Null Pointer Exception at %#llx", stack.rip);
                else {
                    if (currentPML4 != kernelPML4 || cpu.cr2 >= KERNEL_VMA) {
                        physAddr_t physAddr = Physical::AllocNextFreePage();
                        AllocatePage((virtAddr_t)cpu.cr2, physAddr, true);
                        return true;
                    }

                    errorLogger.Log("Page fault for access in user address space!");
                    errorLogger.Log("Fault address: %#llx", cpu.cr2);
                    errorLogger.Log("Error Code: %#x", stack.errorCode);
                }
            } else {
                errorLogger.Log("Page Protection fault!");
                errorLogger.Log("Fault address: %#llx", cpu.cr2);
                errorLogger.Log("Error Code: %#x", stack.errorCode);
            }

            return false;
        }

        bool Init() {
            infoLogger.Log("Initializing virtual memory manager . . .");

            // Identity map all of memory
            uint64_t numPages = Physical::GetNumPages();
            uint64_t numPageTables = (numPages + 511) / 512;
            uint64_t numPageDirectories = (numPageTables + 511) / 512;

            InterruptHandler::SetExceptionHandler(0xE, PageFaultHandler);

            // Allocate Kernel PML4
            kernelPML4 = (PML4*)(Physical::AllocNextFreePage() + KERNEL_VMA);
            memset(kernelPML4, 0, PAGE_SIZE);

            // PML4* bootPML4 = GetCurrentPML4();
            // for (int i = 256; i < 512; i++)
            //    kernelPML4->entries[i] = bootPML4->entries[i];

            // Allocate Kernel PDPTs
            for (int pml4i = 256; pml4i < 512; pml4i++) {
                kernelPML4->SetEntry(pml4i, Physical::AllocNextFreePage(), true, true);
                memset(kernelPML4->GetEntry(pml4i), 0, PAGE_SIZE);

                if (pml4i == 256) {
                    PDPT* pdpt = kernelPML4->GetEntry(pml4i);

                    // Allocate page directories
                    bool done = false;
                    uint64_t totalPageTables = 0;
                    uint64_t totalPages = 0;
                    uint64_t addr = 3;
                    for (uint64_t pdpti = 0; pdpti < numPageDirectories && !done; pdpti++) {
                        pdpt->SetEntry(pdpti, Physical::AllocNextFreePage(), true, true);
                        PD* pd = pdpt->GetEntry(pdpti);
                        memset(pd, 0, PAGE_SIZE);

                        // Allocate page tables
                        for (uint64_t pdi = 0; pdi < 512 && !done; pdi++) {
                            pd->SetEntry(pdi, Physical::AllocNextFreePage(), true, true);
                            PT* pt = pd->GetEntry(pdi);
                            memset(pt, 0, PAGE_SIZE);

                            // Allocate pages
                            for (uint64_t pti = 0; pti < 512 && !done; pti++) {
                                pt->SetEntry(pti, addr, true, true);

                                addr += PAGE_SIZE;
                                totalPages++;

                                if (totalPages >= numPages)
                                    done = true;
                            }

                            totalPageTables++;
                            if (totalPageTables >= numPageTables)
                                done = true;
                        }
                    }
                }
            }

            SetCurrentPML4((uint64_t)kernelPML4 - KERNEL_VMA);
            currentPML4 = kernelPML4;

            // Allocate framebuffer
            uint64_t virtAddr = Console::GetFramebuffer();
            uint64_t physAddr = virtAddr - KERNEL_VMA;

            uint64_t framebufferPageSize = Console::GetFramebufferSize() / PAGE_SIZE;

            for (uint64_t i = 0; i < framebufferPageSize; i++, virtAddr += PAGE_SIZE, physAddr += PAGE_SIZE)
                AllocatePage((virtAddr_t)virtAddr, physAddr, true);

            infoLogger.Log("Virtual memory manager initialized!");

            return true;
        }

        void VirtualToIndex(virtAddr_t addr, int& pml4Index, int& pdptIndex, int& pdIndex, int& ptIndex, int& offset) {
            uint64_t corAddr = (uint64_t)addr & 0x0000FFFFFFFFFFFF;
            offset = corAddr & 0xFFF;
            ptIndex = (corAddr >> 12) & 0x1FF;
            pdIndex = (corAddr >> 21) & 0x1FF;
            pdptIndex = (corAddr >> 30) & 0x1FF;
            pml4Index = (corAddr >> 39) & 0x1FF;
        }

        virtAddr_t IndexToVirtual(uint64_t pml4Index, uint64_t pdptIndex, uint64_t pdIndex, uint64_t ptIndex, uint64_t offset) { return (virtAddr_t)((offset & 0xFFF) | ((ptIndex & 0x1FF) << 12) | ((pdIndex & 0x1FF) << 21) | ((pdptIndex & 0x1FF) << 30) | ((pml4Index & 0x1FF) << 39)); }

        void AllocatePage(virtAddr_t virtAddr, physAddr_t physAddr, bool write) {
            int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
            VirtualToIndex(virtAddr, pml4Index, pdptIndex, pdIndex, ptIndex, offset);

            bool supervisor = (uint64_t)virtAddr < KERNEL_VMA;

            // Check PDPT
            if ((currentPML4->entries[pml4Index] & 1) == 0) {

                // Allocate new PDPT
                currentPML4->SetEntry(pml4Index, Physical::AllocNextFreePage(), true, supervisor);

                // Clear the new PDPT
                memset(currentPML4->GetEntry(pml4Index), 0, PAGE_SIZE);
            }
            PDPT* pdpt = currentPML4->GetEntry(pml4Index);

            // Check PD
            if ((pdpt->entries[pdptIndex] & 1) == 0) {
                // Allocate new PD
                pdpt->SetEntry(pdptIndex, Physical::AllocNextFreePage(), true, supervisor);

                // Clear the new PD
                memset(pdpt->GetEntry(pdptIndex), 0, PAGE_SIZE);
            }
            PD* pd = pdpt->GetEntry(pdptIndex);

            // Check PT
            if ((pd->entries[pdIndex] & 1) == 0) {

                // Allocate new PT
                pd->SetEntry(pdIndex, Physical::AllocNextFreePage(), true, supervisor);
                // Clear the new PDPT
                memset(pd->GetEntry(pdIndex), 0, PAGE_SIZE);
            }
            PT* pt = pd->GetEntry(pdIndex);

            // Allocate Page
            pt->SetEntry(ptIndex, physAddr, write, supervisor);
            MemoryManager::InvalidatePage(virtAddr);
        }

        void FreePage(virtAddr_t addr) {
            int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
            VirtualToIndex(addr, pml4Index, pdptIndex, pdIndex, ptIndex, offset);
            if (currentPML4->entries[pml4Index] != 0) {
                PDPT* pdpt = currentPML4->GetEntry(pml4Index);
                if (pdpt->entries[pdptIndex] != 0) {
                    PD* pd = pdpt->GetEntry(pdIndex);
                    if (pd->entries[ptIndex] != 0) {
                        PT* pt = pd->GetEntry(ptIndex);
                        Physical::FreePage(pt->GetEntry(ptIndex));
                        pt->ClearEntry(ptIndex);
                    }
                }
            }
        }

        physAddr_t VirtualToPhysical(virtAddr_t virtAddr) {
            int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
            VirtualToIndex(virtAddr, pml4Index, pdptIndex, pdIndex, ptIndex, offset);

            if (currentPML4->entries[pml4Index] == 0)
                return 0;

            PDPT* pdpt = currentPML4->GetEntry(pml4Index);
            if (pdpt->entries[pdptIndex] == 0)
                return 0;

            PD* pd = pdpt->GetEntry(pdptIndex);
            if (pd->entries[pdIndex] == 0)
                return 0;

            PT* pt = pd->GetEntry(pdIndex);

            return (pt->entries[ptIndex] & ~(PAGE_SIZE - 1)) + offset;
        }

        uint64_t CreateNewPagingStructure() {
            physAddr_t phys = Physical::AllocNextFreePage();
            PML4* newPML4 = (PML4*)(phys + KERNEL_VMA);
            for (int i = 0; i < 256; i++)
                newPML4->entries[i] = 0;

            for (int i = 256; i < 512; i++)
                newPML4->entries[i] = kernelPML4->entries[i];

            return phys;
        }

        void SetPageStructure(uint64_t cr3) {
            currentPML4 = (PML4*)(cr3 + KERNEL_VMA);
            SetCurrentPML4(cr3);
        }
    } // namespace Virtual
} // namespace MemoryManager