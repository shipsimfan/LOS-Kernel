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
    FATDirectory* rootDirectory = new FATDirectory("", nullptr, filesystem, rootDirFirstCluster);
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
    char* longFilenameBuffer = new char[1024];
    longFilenameBuffer[0] = 0;
    for (uint32_t i = 0; i < bufferSize / sizeof(DirectoryEntry); i++) {
        if (entries[i].attributes == LONG_FILE_NAME) {
            LongFilenameEntry* lfe = (LongFilenameEntry*)&entries[i];
            if (lfe->type != 0)
                continue;

            uint8_t index = lfe->identifier & 0x3F - 1;
            for (int i = 0; i < 5; i++)
                longFilenameBuffer[index + i] = lfe->first[i] & 0xFF;

            for (int i = 0; i < 6; i++)
                longFilenameBuffer[index + i + 5] = lfe->second[i] & 0xFF;

            for (int i = 0; i < 2; i++)
                longFilenameBuffer[index + i + 11] = lfe->third[i] & 0xFF;

            continue;
        }

        if (entries[i].name[0] == 0x00)
            continue;
        else if (memcmp(entries[i].name, ".       ", 8) == 0)
            continue;
        else if (memcmp(entries[i].name, "..      ", 8) == 0)
            continue;

        uint32_t firstCluster = (entries[i].firstClusterHigh << 16) | entries[i].firstClusterLow;

        char* name;

        int j;
        if (longFilenameBuffer[0] == 0) {
            char nameBuffer[13];
            for (j = 0; j < 8; j++)
                nameBuffer[j] = tolower(entries[i].name[j]);

            nameBuffer[8] = 0;
            for (j = 7; j >= 0 && nameBuffer[j] == ' '; j--)
                nameBuffer[j] = 0;

            name = nameBuffer;
        } else
            name = longFilenameBuffer;

        if (entries[i].attributes & DIRECTORY) {
            // Create directory
            FATDirectory* newDirectory = new FATDirectory(name, directory, filesystem, firstCluster);
            directory->AddSubDirectory(newDirectory);

            SetupDirectory(newDirectory, filesystem);
        } else {
            // Create file
            if (name != longFilenameBuffer) {
                int k;
                j++;
                name[j] = '.';
                j++;
                for (k = 0; k < 3 && entries[i].extension[k] != ' '; k++)
                    name[k + j] = tolower(entries[i].extension[k]);

                name[k + j] = 0;
            }

            uint64_t size = entries[i].size;

            FATFile* newFile = new FATFile(name, size, directory, filesystem, firstCluster);
            directory->AddSubFile(newFile);
        }

        longFilenameBuffer[0] = 0;
    }

    delete clusterChain;
    delete longFilenameBuffer;
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
        uint32_t FATOffset = ((cluster * 4) % filesystem->bytesPerSector) / 4;

        if (filesystem->Read(FATSector, buffer, filesystem->bytesPerSector) < 0)
            return nullptr;

        cchain = buffer[FATOffset] & 0x0FFFFFFF;

        ret->push((void*)(uint64_t)(cluster & 0x0FFFFFFF));

        cluster = cchain;
    } while (cchain != 0 && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));

    delete buffer;

    return ret;
}

int64_t FATDriver::Read(File* file, int64_t offset, void* buffer, int64_t count) {
    FATFilesystem* fs = (FATFilesystem*)file->GetFilesystem();
    FATFile* f = (FATFile*)file;

    uint32_t startClusterOffset = offset / (fs->bytesPerSector * fs->sectorsPerCluster);
    uint32_t endClusterOffset = (offset + count) / (fs->bytesPerSector * fs->sectorsPerCluster);

    if ((offset + count) % (fs->bytesPerSector * fs->sectorsPerCluster) != 0)
        endClusterOffset++;

    uint32_t numClusters = endClusterOffset - startClusterOffset;

    Queue<void>* clusterChain = GetClusterChain(f->firstCluster, fs);
    if (clusterChain == nullptr) {
        errno = ERROR_DEVICE_ERROR;
        return -1;
    }

    uint8_t* bufferToUse = new uint8_t[numClusters * fs->bytesPerSector * fs->sectorsPerCluster];
    uint64_t bufferOffset = 0;
    while ((uint64_t)clusterChain->front() != 0 && !(((uint64_t)clusterChain->front() & 0x0FFFFFFF) >= 0x0FFFFF8)) {
        uint32_t cluster = (uint64_t)clusterChain->front();
        clusterChain->pop();

        if (startClusterOffset > 0) {
            startClusterOffset--;
            endClusterOffset--;
            continue;
        }

        if (fs->Read(fs->ClusterToLBA(cluster), bufferToUse + bufferOffset, fs->bytesPerSector * fs->sectorsPerCluster) < 0)
            return -1;

        bufferOffset += fs->bytesPerSector * fs->sectorsPerCluster;

        endClusterOffset--;
        if (endClusterOffset == 0)
            break;
    }

    memcpy(buffer, bufferToUse + (offset % (fs->bytesPerSector * fs->sectorsPerCluster)), count);

    return count;
}

int64_t FATDriver::Write(File* file, int64_t offset, void* buffer, int64_t count) {
    errno = ERROR_NOT_IMPLEMENTED;
    return -1;
}

FATFilesystem::FATFilesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name) : Filesystem(drive, driver, startLBA, length, name) {}
uint64_t FATFilesystem::ClusterToLBA(uint32_t cluster) { return firstUsableCluster + cluster * sectorsPerCluster - 2 * sectorsPerCluster; }

FATDirectory::FATDirectory(const char* name, Directory* parent, Filesystem* filesystem, uint32_t firstCluster) : Directory(name, parent, filesystem), firstCluster(firstCluster) {}

FATFile::FATFile(const char* name, int64_t size, Directory* directory, Filesystem* filesystem, uint32_t firstCluster) : File(name, size, directory, filesystem), firstCluster(firstCluster) {}
