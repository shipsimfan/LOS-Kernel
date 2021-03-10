#include <filesystem/drivers/iso9660.h>

#include "iso9660.h"

#include <console.h>
#include <errno.h>
#include <string.h>

ISO9660Driver isoDriver;

void InitializeISO9660() { RegisterFilesystemDriver(&isoDriver); }

int64_t ISO9660Driver::DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) {
    // Look for 'CD001' signature of ISO 9660 volume descriptor
    uint8_t* sector = new uint8_t[2048];
    int64_t bytesRead = drive->ReadStream(startLBA + 0x10, sector, 2048);
    if (bytesRead < 0) {
        delete sector;
        return -1; // Drive error
    }

    if (memcmp(sector + 1, "CD001", 5)) {
        delete sector;
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
            return -1;
        }

        lba++;
    }

    // Get volume name
    char* volumeName = new char[32];
    strncpy(volumeName, (const char*)(sector + 40), 32);

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
        return -1;
    }

    if (!SetupDirectory(filesystem, rootDirectory, rootEntry, rootLength)) {
        delete rootEntry;
        return -1;
    }

    delete rootEntry;

    RegisterFilesystem(filesystem);

    return 0;
}

bool ISO9660Driver::SetupDirectory(Filesystem* filesystem, Directory* directory, void* bufferStart, uint64_t bufferLength) {
    DirectoryEntry* entry = (DirectoryEntry*)bufferStart;
    uint64_t entryI = (uint64_t)entry;

    // Skip first two entries (self & parent)
    entryI += entry->length;
    entry = (DirectoryEntry*)entryI;
    entryI += entry->length;
    entry = (DirectoryEntry*)entryI;

    // Read the entries
    while (entry->length > 0) {
        if (entry->flags & 2) {
            char* name = new char[entry->filenameLength + 1];
            memcpy(name, &entry->fileIdentifier, entry->filenameLength);
            name[entry->filenameLength] = 0;
            Directory* newDir = new Directory(name, directory, filesystem);
            delete name;

            directory->AddSubDirectory(newDir);

            uint8_t* buffer = new uint8_t[entry->dirLength];
            uint64_t bytesRead = filesystem->Read(entry->lba, buffer, entry->dirLength);
            if (bytesRead < 0) {
                delete buffer;
                return false;
            }

            if (!SetupDirectory(filesystem, newDir, buffer, entry->dirLength)) {
                delete buffer;
                return false;
            }

            delete buffer;
        } else {
            // Find file name
            int64_t nameLength = 0;
            int64_t strLength;
            char* fileIdentifier = (char*)(&entry->fileIdentifier);
            for (strLength = 0; fileIdentifier[strLength] != ';'; strLength++) {
                if (fileIdentifier[strLength] == '.')
                    nameLength = strLength;
            }

            if (nameLength == 0)
                nameLength = strLength;

            char* filename = new char[nameLength + 1];
            memcpy(filename, fileIdentifier, nameLength);
            filename[nameLength] = 0;

            // Find file extension
            char* extension;
            if (nameLength != strLength) {
                nameLength++;
                extension = new char[strLength - nameLength + 1];
                memcpy(extension, fileIdentifier + nameLength, strLength - nameLength);
                extension[strLength - nameLength] = 0;
            } else {
                extension = new char[1];
                extension[0] = 0;
            }

            File* newFile = new ISO9660File(filename, extension, entry->dirLength, directory, filesystem, entry->lba);
            delete filename;
            delete extension;

            directory->AddSubFile(newFile);
        }

        entryI += entry->length;
        entry = (DirectoryEntry*)entryI;
    }

    return true;
}

int64_t ISO9660Driver::Read(File* file, int64_t offset, void* buffer, int64_t count) {
    ISO9660File* isoFile = (ISO9660File*)file;

    int64_t readStart = offset & ~(2047);
    int64_t readEnd = count + offset;
    if (readEnd & 2047) {
        readEnd = readEnd & ~(2047);
        readEnd += 2048;
    }

    int64_t bufToUseSize = readEnd - readStart;
    uint8_t* bufToUse = new uint8_t[bufToUseSize];

    int64_t ret = file->GetFilesystem()->Read(isoFile->entryLBA + (readStart / 2048), bufToUse, bufToUseSize);
    if (ret < 0)
        return -1;

    memcpy(buffer, (void*)((uint64_t)bufToUse + (offset & 2047)), count);

    delete bufToUse;

    return count;
}

int64_t ISO9660Driver::Write(File* file, int64_t offset, void* buffer, int64_t count) {
    errno = ERROR_NOT_IMPLEMENTED;
    return -1;
}

ISO9660File::ISO9660File(const char* name, const char* extension, int64_t size, Directory* directory, Filesystem* filesystem, uint32_t entryLBA) : File(name, extension, size, directory, filesystem), entryLBA(entryLBA) {}
