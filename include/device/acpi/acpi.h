#pragma once

#include <stdint.h>

#define MADT_SIGNATURE "APIC"
#define HPET_SIGNATURE "HPET"

namespace ACPI {
#pragma pack(push)
#pragma pack(1)

    struct AddressStructure {
        uint8_t space;
        uint8_t bitWidth;
        uint8_t bitOffset;
        uint8_t reserved;
        uint64_t address;
    };

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

    struct HPET : public TableHeader {
        uint8_t hardwareRevisionID;
        uint8_t comparatorCount : 5;
        uint8_t counterSize : 1;
        uint8_t reserved : 1;
        uint8_t legacyReplacement : 1;
        uint16_t PCIVendorID;
        AddressStructure address;
        uint8_t hpetNumber;
        uint16_t minimumTick;
        uint8_t pageProtection;
    };

#pragma pack(pop)

    TableHeader* GetTable(const char* tableSignature);
} // namespace ACPI