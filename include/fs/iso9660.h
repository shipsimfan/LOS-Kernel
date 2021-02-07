#pragma once

#include <stdint.h>

namespace VirtualFileSystem { namespace ISO9660 {
#pragma pack(push)
#pragma pack(1)

    struct DirectoryEntry {
        uint8_t length;
        uint8_t xLength;
        uint32_t lba;
        uint32_t lbaMSB;
        uint32_t dirLength;
        uint32_t dirLengthMSB;
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        int8_t timeZone;
        uint8_t flags;
        uint8_t interleaveSize;
        uint8_t interleaveGap;
        uint16_t volumeSequenceNumber;
        uint16_t volumeSequenceNumberMSB;
        uint8_t filenameLength;
        char fileIdentifier;
    };

#pragma pack(pop)

}} // namespace VirtualFileSystem::ISO9660