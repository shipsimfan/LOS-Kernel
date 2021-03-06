#pragma once

#include <stdint.h>

// Defines
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9
#define EI_NIDENT 16

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

#define EM_NONE 0
#define EM_SPARC 2
#define EM_386 3
#define EM_SPARC32PLUS 18
#define EM_SPARCV9 43
#define EM_AMD64 62

#define EV_NONE 0
#define EV_CURRENT 1

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7

// Types
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#pragma pack(push)
#pragma pack(1)

// ELF Header
struct Elf64_Ehdr {
    uint8_t ident[EI_NIDENT];
    Elf64_Half type;
    Elf64_Half machine;
    Elf64_Word version;
    Elf64_Addr entry;
    Elf64_Off phOff;
    Elf64_Off shOff;
    Elf64_Word flags;
    Elf64_Half ehSize;
    Elf64_Half phEntSize;
    Elf64_Half phNum;
    Elf64_Half shEntSize;
    Elf64_Half shNum;
    Elf64_Half shstrndx;
};

struct Elf64_Phdr {
    Elf64_Word type;
    Elf64_Word flags;
    Elf64_Off offset;
    Elf64_Addr vAddr;
    Elf64_Addr pAddr;
    Elf64_Xword fileSz;
    Elf64_Xword memSz;
    Elf64_Xword align;
};

#pragma pack(pop)

bool VerifyELFExecutable(int fd);
uint64_t LoadELFExecutable(int fd);