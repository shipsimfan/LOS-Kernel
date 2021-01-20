#pragma once

#include <types.h>

namespace DeviceManager { namespace ACPI {
#pragma pack(push)
#pragma pack(1)

    struct RDSP {
        uint8_t signature[8];
        uint8_t checksum;
        uint8_t OEMID[6];
        uint8_t revision;
        uint32_t rsdtAddr;
        uint32_t length;
        uint64_t xsdtAddr;
        uint8_t xChecksum;
        uint8_t reserved[3];
    };

    struct SDTHeader {
        uint8_t signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        uint8_t OEMID[6];
        uint8_t OEMTableID[8];
        uint32_t OEMRevision;
        uint32_t creatorID;
        uint32_t creatorRevision;
    };

    struct RSDT : public SDTHeader {
        uint32_t entries;

        SDTHeader* GetEntry(int i);
        int GetNumEntries();
    };

    struct XSDT : public SDTHeader {
        uint64_t entries;

        SDTHeader* GetEntry(int i);
        int GetNumEntries();
    };

    enum PowerManagementProfile : uint8_t { UNSPECIFIED, DESKTOP, MOBILE, WORKSTATION, ENTERPRISE_SERVER, SOHO_SERVER, APPLIANCE_PC, PERFORMANCE_SERVER, TABLET };

    enum AddressSpace : uint8_t { SYSTEM_MEMORY, SYSTEM_IO, PCI_CONFIG_SPACE, EMBEDDED_CONTROLLER, SYSTEM_MANAGEMENT_BUS, SYSTEM_CMOS, PCI_DEVICE_BAR_TARGET, INTELLIGENT_PLATFORM_MANAGEMENT_INFRASTRUCTURE, GENERAL_PURPOSE_IO, GENERIC_SERIAL_BUS, PLATFORM_COMMUNICATION_CHANNEL };

    struct GenericAddressStructure {
        AddressSpace addressSpace;
        uint8_t bitWidth;
        uint8_t bitOffset;
        uint8_t accessSize;
        uint64_t address;
    };

    struct FADT : public SDTHeader {
        uint32_t firmwareCtrl;
        uint32_t dsdt;
        uint8_t reserved;
        PowerManagementProfile preferredPowerManagementProfile;
        uint16_t SCIInterrupt;
        uint32_t SMICommandPort;
        uint8_t acpiEnable;
        uint8_t acpiDisable;
        uint8_t S4BIOSReq;
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
        uint8_t GPE1Base;
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
        GenericAddressStructure resetReg;
        uint8_t resetValue;
        uint8_t reserved3[3];
        uint64_t XFirmwareControl;
        uint64_t Xdsdt;
        GenericAddressStructure XPM1aEventBlock;
        GenericAddressStructure XPM1bEventBlock;
        GenericAddressStructure XPM1aControlBlock;
        GenericAddressStructure XPM1bControlBlock;
        GenericAddressStructure XPM2ControlBlock;
        GenericAddressStructure XPMTimerBlock;
        GenericAddressStructure XGPE0Block;
        GenericAddressStructure XGPE1Block;
    };

    struct DSDT : public SDTHeader {
        uint8_t definitionBlock;
    };

#pragma pack(pop)

    bool Init(RDSP* rdsp);
}} // namespace DeviceManager::ACPI