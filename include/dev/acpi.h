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

#pragma pack(pop)

    bool Init(RDSP* rdsp);
}} // namespace DeviceManager::ACPI