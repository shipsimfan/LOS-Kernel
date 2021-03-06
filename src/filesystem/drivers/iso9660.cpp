#include <filesystem/drivers/iso9660.h>

#include "iso9660.h"

#include <console.h>
#include <errno.h>
#include <string.h>

ISO9660Driver isoDriver;

void InitializeISO9660() { RegisterFilesystemDriver(&isoDriver); }

int64_t ISO9660Driver::DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) {
    drive->Open();

    // Look for 'CD001' signature of ISO 9660 volume descriptor
    uint8_t* sector = new uint8_t[2048];
    int64_t bytesRead = drive->ReadStream(startLBA + 0x10, sector, 2048);
    if (bytesRead < 0) {
        drive->Close();
        delete sector;
        return -1; // Drive error
    }

    if (memcmp(sector + 1, "CD001", 5)) {
        delete sector;
        drive->Close();
        return 1; // Not an ISO 9660 filesystem
    }

    // Is an ISO 9660 filesystem

    // Locate the primary volume descriptor
    uint64_t lba = startLBA + 0x10;
    while (1) {
        // Load the descriptor sector
        bytesRead = drive->ReadStream(lba, sector, 2048);
        if (bytesRead < 0) {
            delete sector;
            drive->Close();
            return -1;
        }

        // Verify identifier
        if (memcmp(sector + 1, "CD001", 5)) {
            delete sector;
            errno = ISO9660_ERROR_INVALID_SIGNATURE;
            return -1;
        }

        // Check descriptor type
        if (sector[0] == 1)
            break;

        if (sector[0] == 0xFF) {
            delete sector;
            errno = ISO9660_ERROR_NO_PRIMARY_VOLUME_DESCRIPTOR;
            drive->Close();
            return -1;
        }

        lba++;
    }

    // Get volume name
    char* volumeName = new char[32];
    strncpy(volumeName, (const char*)(sector + 40), 32);

    Console::Println("New volume name: %s", volumeName);

    // Create the filesystem
    Filesystem* filesystem = new Filesystem(drive, &isoDriver, startLBA, size, volumeName);
    delete volumeName;

    // Create the root directory
    Directory* rootDirectory = new Directory("", nullptr, filesystem);

    // Set the root directory
    filesystem->SetRootDirectory(rootDirectory);

    // Get the root directory entry
    DirectoryEntry* entry = (DirectoryEntry*)(sector + 156);
    uint32_t rootLBA = entry->lba;
    uint32_t rootLength = entry->dirLength;

    delete sector;

    // Load the root directory
    uint8_t* rootEntry = new uint8_t[rootLength];
    bytesRead = drive->ReadStream(rootLBA, rootEntry, rootLength);
    if (bytesRead < 0) {
        delete rootEntry;
        drive->Close();
        return -1;
    }

    if (!SetupDirectory(filesystem, rootDirectory, rootEntry, rootLength)) {
        delete rootEntry;
        drive->Close();
        return -1;
    }

    delete rootEntry;
    drive->Close();

    RegisterFilesystem(filesystem);

    Console::Println("Test");

    return 0;
}

bool ISO9660Driver::SetupDirectory(Filesystem* filesystem, Directory* directory, void* bufferStart, uint64_t bufferLength) { return true; }