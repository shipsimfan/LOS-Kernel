#include <filesystem/drivers/fat.h>

#include "fat.h"

#include <console.h>
#include <errno.h>
#include <string.h>

FATDriver fatDriver;

void InitializeFAT() { RegisterFilesystemDriver(&fatDriver); }

int64_t FATDriver::DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t _size) {
    uint8_t buffer[512];
    if (drive->ReadStream(startLBA + BOOT_RECORD_SECTOR, buffer, 512) < 0)
        return -1;

    // Verify bootcode
    if (buffer[BPB_BOOTCODE] != 0x55 || buffer[BPB_BOOTCODE + 1] != 0xAA)
        return 1;

    // Verify EBPB signature
    if (buffer[EBPB_SIGNATURE] != 0x28 && buffer[EBPB_SIGNATURE] != 0x29)
        return 1;

    // Get the sector size
    uint16_t sectorSize = *(uint16_t*)(buffer + BPB_BYTES_PER_SECTOR);

    // Get filesystem size
    int64_t size = *(uint16_t*)(buffer + BPB_TOTAL_SECTORS);
    if (size == 0)
        size = *(uint32_t*)(buffer + BPB_LARGE_TOTAL_SECTORS);

    size *= sectorSize;

    // Get volume name
    char volumeName[12];
    memcpy(volumeName, buffer + EBPB_VOLUME_NAME, 11);
    volumeName[12] = 0;

    // Get sectors per cluster
    uint8_t sectorsPerCluster = buffer[BPB_SECTORS_PER_CLUSTER];

    // Get first usable cluster
    uint32_t firstFATSector = startLBA + *(uint16_t*)(buffer + BPB_NUMBER_OF_RESERVED_SECTORS);
    uint32_t firstUsableCluster = firstFATSector + *(uint32_t*)(buffer + EBPB_SECTORS_PER_FAT) * buffer[BPB_NUMBER_OF_FATS];

    // Get root directory size and sector
    uint32_t rootDirFirstCluster = *(uint32_t*)(buffer + EBPB_ROOT_DIRECTORY_FIRST_CLUSTER);

    // Get fs info sector
    uint16_t fsInfoSector = *(uint16_t*)(buffer + EBPB_FSINFO);
    if (drive->ReadStream(startLBA + fsInfoSector, buffer, 512) < 0)
        return -1;

    // Verify FS info signatures
    if (*(uint32_t*)(buffer + FSINFO_SIGNATURE_1) != 0x41615252 || *(uint32_t*)(buffer + FSINFO_SIGNATURE_2) != 0x61417272)
        return -1;

    // Create the filesystem
    FATFilesystem* filesystem = new FATFilesystem(drive, this, startLBA, size, volumeName);
    filesystem->bytesPerSector = sectorSize;
    filesystem->sectorsPerCluster = sectorsPerCluster;
    filesystem->firstUsableCluster = firstUsableCluster;
    filesystem->firstFATSector = firstFATSector;

    // Create the root directory
    FATDirectory* rootDirectory = new FATDirectory("", rootDirectory, filesystem, rootDirFirstCluster);
    filesystem->SetRootDirectory(rootDirectory);

    if (!SetupDirectory(rootDirectory, filesystem)) {
        delete filesystem;
        delete rootDirectory;
        return -1;
    }

    RegisterFilesystem(filesystem);
    return 0;
}

bool FATDriver::SetupDirectory(FATDirectory* directory, FATFilesystem* filesystem) {
    // Get the cluster chain
    Queue<void>* clusterChain = GetClusterChain(directory->firstCluster, filesystem);
    if (clusterChain == nullptr)
        return false;

    // Allocate the buffer
    uint64_t bufferSize = clusterChain->count() * (filesystem->bytesPerSector * filesystem->sectorsPerCluster);
    uint8_t* buffer = new uint8_t[bufferSize];

    // Read the clusters
    uint64_t offset = 0;
    while ((uint64_t)clusterChain->front() != 0 && !(((uint64_t)clusterChain->front() & 0x0FFFFFFF) >= 0x0FFFFF8)) {
        uint32_t cluster = (uint64_t)clusterChain->front();
        clusterChain->pop();

        if (filesystem->Read(filesystem->ClusterToLBA(cluster), buffer + offset, filesystem->sectorsPerCluster * filesystem->bytesPerSector) < 0) {
            delete clusterChain;
            delete buffer;
            return false;
        }

        offset += filesystem->sectorsPerCluster * filesystem->bytesPerSector;
    }

    DirectoryEntry* entries = (DirectoryEntry*)buffer;
    for (uint32_t i = 0; i < bufferSize / sizeof(DirectoryEntry); i++) {
        if (entries[i].attributes == LONG_FILE_NAME) {
            Console::Println("[ FAT ] Long file name located");
            continue;
        }

        if (entries[i].name[0] == 0x00)
            continue;
        else if (memcmp(entries[i].name, ".       ", 8) == 0)
            continue;
        else if (memcmp(entries[i].name, "..      ", 8) == 0)
            continue;

        if (memcmp(entries[i].name, "USR", 3) == 0)
            Console::Println("AHHH!");

        uint32_t firstCluster = (entries[i].firstClusterHigh << 16) | entries[i].firstClusterLow;

        char name[9];
        int j;
        for (j = 0; j < 8 && entries[i].name[j]; j++)
            name[j] = entries[i].name[j];

        name[j] = 0;

        if (entries[i].attributes & DIRECTORY) {
            // Create directory
            FATDirectory* newDirectory = new FATDirectory(name, directory, filesystem, firstCluster);
            directory->AddSubDirectory(newDirectory);

            Console::Println("[ FAT ] New directory: %s", newDirectory->GetName());

            SetupDirectory(newDirectory, filesystem);
        } else {
            // Create file
            char extension[4];
            int k;
            for (k = 0; k < 3 && entries[i].extension[k]; k++)
                extension[k] = entries[i].extension[k];

            extension[k] = 0;

            uint64_t size = entries[i].size;

            File* newFile = new File(name, extension, size, directory, filesystem);
            directory->AddSubFile(newFile);

            Console::Println("[ FAT ] New file: %s.%s", newFile->GetName(), newFile->GetExtension());
        }
    }

    delete clusterChain;
    delete buffer;
    return true;
}

Queue<void>* FATDriver::GetClusterChain(uint32_t firstCluster, FATFilesystem* filesystem) {
    Queue<void>* ret = new Queue<void>();
    uint32_t cluster = firstCluster;
    uint32_t cchain = 0;

    uint32_t* buffer = new uint32_t[filesystem->bytesPerSector / 4];
    do {
        uint32_t FATSector = filesystem->firstFATSector + ((cluster * 4) / filesystem->bytesPerSector);
        uint32_t FATOffset = cluster % filesystem->bytesPerSector;

        if (filesystem->Read(FATSector, buffer, filesystem->bytesPerSector) < 0)
            return nullptr;

        cchain = buffer[FATOffset] & 0x0FFFFFFF;

        ret->push((void*)(uint64_t)cluster);

        cluster = cchain;
    } while (cchain != 0 && !((cchain & 0x0FFFFFFF) >= 0x0FFFFF8));

    delete buffer;

    return ret;
}

int64_t FATDriver::Read(File* file, int64_t offset, void* buffer, int64_t count) {
    errno = ERROR_NOT_IMPLEMENTED;
    return -1;
}

int64_t FATDriver::Write(File* file, int64_t offset, void* buffer, int64_t count) {
    errno = ERROR_NOT_IMPLEMENTED;
    return -1;
}

FATFilesystem::FATFilesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name) : Filesystem(drive, driver, startLBA, length, name) {}
uint64_t FATFilesystem::ClusterToLBA(uint32_t cluster) { return firstUsableCluster + cluster * sectorsPerCluster - 2 * sectorsPerCluster; }

FATDirectory::FATDirectory(const char* name, Directory* parent, Filesystem* filesystem, uint32_t firstCluster) : Directory(name, parent, filesystem), firstCluster(firstCluster) {}
