#pragma once

#include <stdint.h>

namespace ACPI {
#pragma pack(push)
#pragma pack(1)

    struct TableHeader {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char OEMID[6];
        char OEMTableID[8];
        uint32_t OEMRevision;
        char ASLCompilerID[4];
        uint32_t ASLCompilerRevision;
    };

#pragma pack(pop)

    TableHeader* GetTable(const char* tableSignature);
} // namespace ACPI