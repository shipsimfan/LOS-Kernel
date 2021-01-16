#include <mem/virtual.h>

#include <interrupt.h>
#include <logger.h>
#include <string.h>

MemoryManager::Physical* physInstance;
MemoryManager::Virtual* virtInstance;
bool PageFaultHandler(InterruptHandler::CPUState cpu, InterruptHandler::StackState stack) {
    if ((stack.errorCode & 1) == 0) {
        if (cpu.cr2 >= KERNEL_VMA) {
            physAddr_t physAddr = physInstance->AllocNextFreePage();
            virtInstance->AllocatePage((virtAddr_t)cpu.cr2, physAddr, true);
            return true;
        }

        errorLogger.Log("Page fault for access in user address space!");
        errorLogger.Log("Fault address: %#llx", cpu.cr2);
        errorLogger.Log("Error Code: %#x", stack.errorCode);
    } else {
        errorLogger.Log("Page Protection fault!");
        errorLogger.Log("Fault address: %#llx", cpu.cr2);
        errorLogger.Log("Error Code: %#x", stack.errorCode);
    }

    return false;
}

namespace MemoryManager {
    void InvalidatePage(virtAddr_t addr) { asm volatile("invlpg (%0)" ::"r"((uint64_t)addr) : "memory"); }

    template <class T> void PageTableBase<T>::SetEntry(int index, physAddr_t addr, bool write, bool supervisor) {
        if (index >= 512)
            return;

        uint64_t entry = addr & ~(PAGE_SIZE - 1);
        entry |= PAGE_PRESENT;
        if (write)
            entry |= PAGE_WRITE;
        if (addr >= KERNEL_VMA)
            entry |= PAGE_SUPERVISOR;

        entries[index] = entry;
    }

    template <class T> void PageTableBase<T>::ClearEntry(int index) {
        if (index >= 512)
            return;

        entries[index] = 0;
    }

    bool Virtual::Init(Physical* physicalMem) {
        infoLogger.Log("Initializing virtual memory manager . . .");

        physInstance = physicalMem;
        virtInstance = this;

        this->physicalMem = physicalMem;

        // Identity map all of memory
        uint64_t numPages = physicalMem->GetNumPages();
        uint64_t numPageTables = numPages / 512;
        uint64_t numPageDirectories = numPageTables / 512;
        // Allocate Kernel PML4
        kernelPML4 = (PML4*)(physicalMem->AllocNextFreePage() + KERNEL_VMA);
        memset(kernelPML4, 0, PAGE_SIZE);

        // Allocate 256 PDPTs
        for (int pml4i = 256; pml4i < 512; pml4i++) {
            kernelPML4->SetEntry(pml4i, physicalMem->AllocNextFreePage(), true, true);
            memset(kernelPML4->GetEntry(pml4i), 0, PAGE_SIZE);

            if (pml4i == 256) {
                PDPT* pdpt = kernelPML4->GetEntry(pml4i);
                uint64_t addr = 0;
                uint64_t pages = 0;
                uint64_t pageTables = 0;
                bool done = false;

                // Allocate page directories
                for (uint64_t pdpti = 0; pdpti < numPageDirectories && !done; pdpti++) {
                    pdpt->SetEntry(pdpti, physicalMem->AllocNextFreePage(), true, true);
                    PD* pd = pdpt->GetEntry(pdpti);
                    memset(pd, 0, PAGE_SIZE);

                    // Allocate page tables
                    for (int pdi = 0; pdi < 512 && !done; pdi++) {
                        pd->SetEntry(pdi, physicalMem->AllocNextFreePage(), true, true);
                        PT* pt = pd->GetEntry(pdi);

                        // Allocate pages
                        for (int pti = 0; pti < 512 && !done; pti++) {
                            pt->SetEntry(pti, addr, true, true);
                            addr += PAGE_SIZE;
                            pages++;

                            if (pages >= numPages)
                                done = true;
                        }

                        pageTables++;
                        if (pageTables >= numPageTables)
                            done = true;
                    }
                }
            }
        }

        SetCurrentPML4((uint64_t)kernelPML4 - KERNEL_VMA);
        currentPML4 = kernelPML4;

        InterruptHandler::SetExceptionHandler(0xE, PageFaultHandler);

        infoLogger.Log("Virtual memory manager ready!");

        return true;
    }

    virtAddr_t Virtual::AllocNextKPage() {
        physAddr_t phys;
        return AllocNextKPage(phys);
    }

    virtAddr_t Virtual::AllocNextKPage(physAddr_t& physAddr) {
        // Locate a page
        virtAddr_t addr = nullptr;
        for (int pml4i = 256; pml4i < 512; pml4i++) {
            PDPT* pdpt = currentPML4->GetEntry(pml4i);
            for (int pdpti = 0; pdpti < 512; pdpti++) {
                if (pdpt->entries[pdpti] == 0) {
                    uint64_t newPD = physicalMem->AllocNextFreePage();
                    pdpt->SetEntry(pdpti, newPD, true, true);
                    memset(pdpt->GetEntry(pdpti), 0, 4096);
                }

                PD* pd = pdpt->GetEntry(pdpti);
                for (int pdi = 0; pdi < 512; pdi++) {
                    if (pd->entries[pdi] == 0) {
                        uint64_t newPT = physicalMem->AllocNextFreePage();
                        pd->SetEntry(pdi, newPT, true, true);
                        memset(pd->GetEntry(pdi), 0, 4096);
                    }

                    PT* pt = pd->GetEntry(pdi);
                    for (int pti = 0; pti < 512; pti++) {
                        if (pt->entries[pti] == 0) {
                            physAddr = physicalMem->AllocNextFreePage();
                            pt->SetEntry(pti, physAddr, true, true);
                            addr = IndexToVirtual(pml4i, pdpti, pdi, pti, 0);
                            break;
                        }
                    }

                    if (addr != nullptr)
                        break;
                }

                if (addr != nullptr)
                    break;
            }

            if (addr != nullptr)
                break;
        }

        return addr;
    }

    void Virtual::AllocatePage(virtAddr_t virtAddr, physAddr_t physAddr, bool write) {
        int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
        VirtualToIndex(virtAddr, pml4Index, pdptIndex, pdIndex, ptIndex, offset);

        bool supervisor = (uint64_t)virtAddr >= KERNEL_VMA;

        // Check PDPT
        if ((currentPML4->entries[pml4Index] & 1) == 0) {

            // Allocate new PDPT
            currentPML4->SetEntry(pml4Index, physicalMem->AllocNextFreePage(), true, supervisor);

            // Clear the new PDPT
            memset(currentPML4->GetEntry(pml4Index), 0, PAGE_SIZE);
        }
        PDPT* pdpt = currentPML4->GetEntry(pml4Index);

        // Check PD
        if ((pdpt->entries[pdptIndex] & 1) == 0) {
            // Allocate new PD
            pdpt->SetEntry(pdptIndex, physicalMem->AllocNextFreePage(), true, supervisor);

            // Clear the new PD
            memset(pdpt->GetEntry(pdptIndex), 0, PAGE_SIZE);
        }
        PD* pd = pdpt->GetEntry(pdptIndex);

        // Check PT
        if ((pd->entries[pdIndex] & 1) == 0) {

            // Allocate new PT
            pd->SetEntry(pdIndex, physicalMem->AllocNextFreePage(), true, supervisor);
            // Clear the new PDPT
            memset(pd->GetEntry(pdIndex), 0, PAGE_SIZE);
        }
        PT* pt = pd->GetEntry(pdIndex);

        // Allocate Page
        pt->SetEntry(ptIndex, physicalMem->AllocNextFreePage(), write, supervisor);
        MemoryManager::InvalidatePage(virtAddr);
    }

    void Virtual::FreePage(virtAddr_t addr) {
        int pml4Index, pdptIndex, pdIndex, ptIndex, offset;
        VirtualToIndex(addr, pml4Index, pdptIndex, pdIndex, ptIndex, offset);
        if (currentPML4->entries[pml4Index] != 0) {
            PDPT* pdpt = currentPML4->GetEntry(pml4Index);
            if (pdpt->entries[pdptIndex] != 0) {
                PD* pd = pdpt->GetEntry(pdIndex);
                if (pd->entries[ptIndex] != 0) {
                    PT* pt = pd->GetEntry(ptIndex);
                    physicalMem->FreePage(pt->GetEntry(ptIndex));
                    pt->ClearEntry(ptIndex);
                }
            }
        }
    }

    void Virtual::VirtualToIndex(virtAddr_t addr, int& pml4Index, int& pdptIndex, int& pdIndex, int& ptIndex, int& offset) {
        uint64_t corAddr = (uint64_t)addr & 0x0000FFFFFFFFFFFF;
        offset = corAddr & 0xFFF;
        ptIndex = (corAddr >> 12) & 0x1FF;
        pdIndex = (corAddr >> 21) & 0x1FF;
        pdptIndex = (corAddr >> 30) & 0x1FF;
        pml4Index = (corAddr >> 39) & 0x1FF;
    }

    virtAddr_t Virtual::IndexToVirtual(uint64_t pml4Index, uint64_t pdptIndex, uint64_t pdIndex, uint64_t ptIndex, uint64_t offset) { return (virtAddr_t)((offset & 0xFFF) | ((ptIndex & 0x1FF) << 12) | ((pdIndex & 0x1FF) << 21) | ((pdptIndex & 0x1FF) << 30) | ((pml4Index & 0x1FF) << 39)); }

} // namespace MemoryManager