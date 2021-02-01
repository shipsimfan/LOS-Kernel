#include <elf.h>

#include <string.h>

namespace ELF {
    int VerifyElfExecHeader(void* header) {
        Elf64_Ehdr* hdr = (Elf64_Ehdr*)header;

        // Verify MAG
        if (hdr->ident[EI_MAG0] != ELFMAG0 || hdr->ident[EI_MAG1] != ELFMAG1 || hdr->ident[EI_MAG2] != ELFMAG2 || hdr->ident[EI_MAG3] != ELFMAG3)
            return 1;

        // Verify Class
        if (hdr->ident[EI_CLASS] != ELFCLASS64)
            return 2;

        // Verify Data
        if (hdr->ident[EI_DATA] != ELFDATA2LSB)
            return 3;

        // Verify Type
        if (hdr->type != ET_EXEC)
            return 4;

        // Verify Machine
        if (hdr->machine != EM_AMD64)
            return 5;

        // Verify Version
        if (hdr->version < EV_CURRENT)
            return 6;

        return 0;
    }

    void* LoadExecutableIntoUserspace(void* file) {
        Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)file;

        Elf64_Phdr* pHdr = (Elf64_Phdr*)(elfHeader->phOff + (uint64_t)elfHeader);
        for (int i = 0; i < elfHeader->phNum; i++) {
            if (pHdr->type == PT_LOAD) {
                memcpy((void*)pHdr->vAddr, (void*)(pHdr->offset + (uint64_t)elfHeader), pHdr->fileSz);
                memset((void*)(pHdr->vAddr + pHdr->fileSz), 0, pHdr->memSz - pHdr->fileSz);
            }

            pHdr = (Elf64_Phdr*)((uint64_t)pHdr + elfHeader->phEntSize);
        }

        return (void*)elfHeader->entry;
    }
} // namespace ELF