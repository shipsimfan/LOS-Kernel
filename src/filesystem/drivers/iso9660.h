#pragma once

#include <filesystem/driver.h>

#define ISO9660_ERROR_INVALID_SIGNATURE 0xF0000
#define ISO9660_ERROR_NO_PRIMARY_VOLUME_DESCRIPTOR 0xF0001

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

class ISO9660Driver : public FilesystemDriver {
public:
    int64_t DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) override;

    int64_t Read(File* file, int64_t offset, void* buffer, int64_t count) override;
    int64_t Write(File* file, int64_t offset, void* buffer, int64_t count) override;

    int64_t Truncate(File* file, int64_t newSize) override;

private:
    bool SetupDirectory(Filesystem* filesystem, Directory* directory, void* bufferStart, uint64_t bufferLength);
};

class ISO9660File : public File {
public:
    ISO9660File(const char* name, int64_t size, Directory* directory, Filesystem* filesystem, uint32_t entryLBA);

    uint32_t entryLBA;
};