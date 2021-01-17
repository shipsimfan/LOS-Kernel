#pragma once

#include <types.h>

#pragma pack(push)
#pragma pack(1)

struct multiboot2BootInformation {
    uint32_t totalSize;
    uint32_t reserved;
};

struct multiboot2TagHeader {
    uint32_t type;
    uint32_t size;
};

struct mutliboot2TagBasicMemory {
    multiboot2TagHeader header;
    uint32_t memLower;
    uint32_t memUpper;
};

struct multiboot2TagBIOSBootDevice {
    multiboot2TagHeader header;
    uint32_t BIOSDevice;
    uint32_t partition;
    uint32_t subPartition;
};

struct multiboot2TagCommandLine {
    multiboot2TagHeader header;
    char string;
};

struct multiboot2MemoryMapEntry {
    uint64_t baseAddr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct multiboot2TagMemoryMap {
    multiboot2TagHeader header;
    uint32_t entrySize;
    uint32_t entryVersion;
    multiboot2MemoryMapEntry entries;
};

#pragma pack(pop)