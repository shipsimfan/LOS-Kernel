#include "elf.h"

#include <fs.h>
#include <string.h>

bool VerifyELFExecutable(int fd) {
    Elf64_Ehdr* header = new Elf64_Ehdr;
    Seek(fd, 0, SEEK_SET);
    Read(fd, header, sizeof(Elf64_Ehdr));

    bool ret = true;

    // Verify MAG
    if (header->ident[EI_MAG0] != ELFMAG0 || header->ident[EI_MAG1] != ELFMAG1 || header->ident[EI_MAG2] != ELFMAG2 || header->ident[EI_MAG3] != ELFMAG3)
        ret = false;

    // Verify Class
    else if (header->ident[EI_CLASS] != ELFCLASS64)
        ret = false;

    // Verify Data
    else if (header->ident[EI_DATA] != ELFDATA2LSB)
        ret = false;

    // Verify Type
    else if (header->type != ET_EXEC)
        ret = false;

    // Verify Machine
    else if (header->machine != EM_AMD64)
        ret = false;

    // Verify Version
    else if (header->version < EV_CURRENT)
        ret = false;

    delete header;
    return ret;
}

uint64_t LoadELFExecutable(int fd) {
    Elf64_Ehdr* elfHeader = new Elf64_Ehdr;
    Seek(fd, 0, SEEK_SET);
    if (Read(fd, elfHeader, sizeof(Elf64_Ehdr)) < 0) {
        delete elfHeader;
        return ~0;
    }

    uint8_t* programHeaders = new uint8_t[elfHeader->phNum * elfHeader->phEntSize];
    Seek(fd, elfHeader->phOff, SEEK_SET);
    if (Read(fd, programHeaders, elfHeader->phNum * elfHeader->phEntSize) < 0) {
        delete elfHeader;
        delete programHeaders;
        return ~0;
    }

    Elf64_Phdr* pHdr = (Elf64_Phdr*)programHeaders;
    for (int i = 0; i < elfHeader->phNum; i++) {
        if (pHdr->type == PT_LOAD) {
            Seek(fd, pHdr->offset, SEEK_SET);
            if (Read(fd, (void*)pHdr->vAddr, pHdr->fileSz) < 0) {
                delete elfHeader;
                delete programHeaders;
                return ~0;
            }
            memset((void*)(pHdr->vAddr + pHdr->fileSz), 0, pHdr->memSz - pHdr->fileSz);
        }

        pHdr = (Elf64_Phdr*)((uint64_t)pHdr + elfHeader->phEntSize);
    }

    uint64_t entry = elfHeader->entry;

    delete elfHeader;
    delete programHeaders;

    return entry;
}