#include <elf.h>

#include <fs.h>
#include <stdlib.h>
#include <string.h>

namespace ELF {
    int VerifyElfExecHeader(int fd) {
        void* header = malloc(sizeof(Elf64_Ehdr));
        VirtualFileSystem::Seek(fd, 0, SEEK_SET);
        VirtualFileSystem::Read(fd, header, sizeof(Elf64_Ehdr));

        Elf64_Ehdr* hdr = (Elf64_Ehdr*)header;

        int ret = 0;

        // Verify MAG
        if (hdr->ident[EI_MAG0] != ELFMAG0 || hdr->ident[EI_MAG1] != ELFMAG1 || hdr->ident[EI_MAG2] != ELFMAG2 || hdr->ident[EI_MAG3] != ELFMAG3)
            ret = 1;

        // Verify Class
        else if (hdr->ident[EI_CLASS] != ELFCLASS64)
            ret = 2;

        // Verify Data
        else if (hdr->ident[EI_DATA] != ELFDATA2LSB)
            ret = 3;

        // Verify Type
        else if (hdr->type != ET_EXEC)
            ret = 4;

        // Verify Machine
        else if (hdr->machine != EM_AMD64)
            ret = 5;

        // Verify Version
        else if (hdr->version < EV_CURRENT)
            ret = 6;

        free(header);
        return ret;
    }

    void* LoadExecutableIntoUserspace(int fd) {
        size_t fileSize = VirtualFileSystem::Seek(fd, 0, SEEK_END);
        void* file = malloc(fileSize);

        VirtualFileSystem::Seek(fd, 0, SEEK_SET);
        VirtualFileSystem::Read(fd, file, fileSize);

        Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)file;

        Elf64_Phdr* pHdr = (Elf64_Phdr*)(elfHeader->phOff + (uint64_t)elfHeader);
        for (int i = 0; i < elfHeader->phNum; i++) {
            if (pHdr->type == PT_LOAD) {
                memcpy((void*)pHdr->vAddr, (void*)(pHdr->offset + (uint64_t)elfHeader), pHdr->fileSz);
                memset((void*)(pHdr->vAddr + pHdr->fileSz), 0, pHdr->memSz - pHdr->fileSz);
            }

            pHdr = (Elf64_Phdr*)((uint64_t)pHdr + elfHeader->phEntSize);
        }

        void* entry = (void*)elfHeader->entry;

        free(file);

        return entry;
    }
} // namespace ELF