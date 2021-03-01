#pragma once

#include <stdint.h>

#define MADT_SIGNATURE "APIC"

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

    struct MADT : public TableHeader {
        enum EntryType : uint8_t { LOCAL_APIC = 0, IO_APIC, INTERRUPT_SOURCE_OVERRIDE, NON_MASKABLE_INTERRUPT = 4, LOCAL_APIC_ADDRESS_OVERRIDE };

        struct EntryHeader {
            EntryType type;
            uint8_t length;
        };

        struct LocalAPICEntry : public EntryHeader {
            uint8_t ACPIProcessorID;
            uint8_t ID;
            uint32_t flags;
        };

        struct IOAPICEntry : public EntryHeader {
            uint8_t ID;
            uint8_t reserved;
            uint32_t address;
            uint32_t interruptBase;
        };

        struct InterruptSourceOverrideEntry : public EntryHeader {
            uint8_t busSource;
            uint8_t irqSource;
            uint32_t interrupt;
            uint16_t flags;
        };

        struct NonMaskableInterruptEntry : public EntryHeader {
            uint8_t ACPIProcessorID;
            uint16_t flags;
            uint8_t LINTNumber;
        };

        struct LocalAPICAddressOverrideEntry : public EntryHeader {
            uint16_t reserved;
            uint64_t address;
        };

        uint32_t localAPICAddress;
        uint32_t flags;
        EntryHeader entries[1];
    };

#pragma pack(pop)

    TableHeader* GetTable(const char* tableSignature);
} // namespace ACPI