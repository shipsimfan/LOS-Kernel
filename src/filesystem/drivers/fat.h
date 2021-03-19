#pragma once

#include <filesystem/driver.h>

#define BOOT_RECORD_SECTOR 0

#define BPB_BYTES_PER_SECTOR 0x0B
#define BPB_SECTORS_PER_CLUSTER 0x0D
#define BPB_NUMBER_OF_RESERVED_SECTORS 0x0E
#define BPB_NUMBER_OF_FATS 0x10
#define BPB_TOTAL_SECTORS 0x13
#define BPB_LARGE_TOTAL_SECTORS 0x20

#define EBPB_SECTORS_PER_FAT 0x24
#define EBPB_ROOT_DIRECTORY_FIRST_CLUSTER 0x2C
#define EBPB_FSINFO 0x30
#define EBPB_SIGNATURE 0x42
#define EBPB_VOLUME_NAME 0x47

#define BPB_BOOTCODE 510

#define FSINFO_SIGNATURE_1 0x0
#define FSINFO_SIGNATURE_2 0x1E4

#define READ_ONLY 0x01
#define HIDDEN 0x02
#define SYSTEM 0x04
#define VOLUME_ID 0x08
#define DIRECTORY 0x10
#define ARCHIVE 0x20

#define LONG_FILE_NAME (READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID)

#pragma pack(push)
#pragma pack(1)

struct DirectoryEntry {
    char name[8];
    char extension[3];
    uint8_t attributes;
    uint8_t reserved;
    struct {
        uint8_t tenths;
        uint16_t hour : 5;
        uint16_t minutes : 6;
        uint16_t seconds : 5;
        uint16_t year : 7;
        uint16_t month : 4;
        uint16_t day : 5;
    } creation;
    struct {
        uint16_t year : 7;
        uint16_t month : 4;
        uint16_t day : 5;
    } lastAccess;
    uint16_t firstClusterHigh;
    struct {
        uint16_t hour : 5;
        uint16_t minutes : 6;
        uint16_t seconds : 5;
        uint16_t year : 7;
        uint16_t month : 4;
        uint16_t day : 5;
    } lastMoification;
    uint16_t firstClusterLow;
    uint32_t size;
};

struct LongFilenameEntry {
    uint8_t identifier;
    uint16_t first[5];
    uint8_t attribute;
    uint8_t type;
    uint8_t checksum;
    uint16_t second[6];
    uint16_t zero;
    uint16_t third[2];
};

#pragma pack(pop)

class FATFilesystem;
class FATDirectory;

class FATDriver : public FilesystemDriver {
public:
    int64_t DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) override;

    int64_t Read(File* file, int64_t offset, void* buffer, int64_t count) override;
    int64_t Write(File* file, int64_t offset, void* buffer, int64_t count) override;

    int64_t Truncate(File* file, int64_t newSize) override;

private:
    bool SetupDirectory(FATDirectory* directory, FATFilesystem* filesystem);

    Queue<void>* GetClusterChain(uint32_t firstCluster, FATFilesystem* filesystem);
};

class FATFilesystem : public Filesystem {
    friend FATDriver;

public:
    FATFilesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name);

private:
    uint64_t ClusterToLBA(uint32_t cluster);

    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint32_t firstFATSector;
    uint32_t firstUsableCluster;
};

class FATDirectory : public Directory {
    friend FATDriver;

public:
    FATDirectory(const char* name, Directory* parent, Filesystem* filesystem, uint32_t firstCluster);

private:
    uint32_t firstCluster;
};

class FATFile : public File {
    friend FATDriver;

public:
    FATFile(const char* name, int64_t size, Directory* directory, Filesystem* filesystem, uint32_t firstCluster, int flags);

private:
    uint32_t firstCluster;
};