#pragma once

#include <stdint.h>

#define MADT_SIGNATURE "APIC"
#define HPET_SIGNATURE "HPET"
#define FADT_SIGNATURE "FACP"

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

    struct FADT : public TableHeader {
        uint32_t firmwareControl;
        uint32_t dsdt;
        uint8_t reserved;
        uint8_t preferredPowerManagementProfile;
        uint16_t SCIInterrupt;
        uint32_t SMICommandPort;
        uint8_t ACPIEnable;
        uint8_t ACPIDisable;
        uint8_t S4BIOSREQ;
        uint8_t PSTATEControl;
        uint32_t PM1aEventBlock;
        uint32_t PM1bEventBlock;
        uint32_t PM1aControlBlock;
        uint32_t PM1bControlBlock;
        uint32_t PM2ControlBlock;
        uint32_t PMTimerBlock;
        uint32_t GPE0Block;
        uint32_t GPE1Block;
        uint8_t PM1EventLength;
        uint8_t PM1ControlLength;
        uint8_t PM2ControlLength;
        uint8_t PMTimerLength;
        uint8_t GPE0Length;
        uint8_t GPE1Length;
        uint8_t CStateControl;
        uint16_t worstC2Latency;
        uint16_t worstC3Latency;
        uint16_t flushSize;
        uint16_t flushStride;
        uint8_t dutyOffset;
        uint8_t dutyWidth;
        uint8_t dayAlarm;
        uint8_t monthAlarm;
        uint8_t century;
        uint16_t bootArchitectureFlags;
        uint8_t reserved2;
        uint32_t flags;
        AddressStructure resetReg;
        uint8_t resetValue;
        uint8_t reserved3[3];
        uint64_t xFirmwareControl;
        uint64_t xDSDT;
        AddressStructure xPM1AEventBlock;
        AddressStructure xPM1BEventBlock;
        AddressStructure xPM1AControlBlock;
        AddressStructure xPM2ControlBlock;
        AddressStructure xPMTimerBlock;
        AddressStructure xGPE0Block;
        AddressStructure xGPE1Block;
    };

#pragma pack(pop)

    TableHeader* GetTable(const char* tableSignature);

    void Shutdown();
} // namespace ACPI