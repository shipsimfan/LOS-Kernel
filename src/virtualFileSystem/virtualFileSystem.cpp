#include <fs.h>

#include <logger.h>
#include <stdlib.h>
#include <string.h>

#define ISO9660_SECTOR_SIZE 2048

namespace VirtualFileSystem {
    extern FileSystemDriver ISO9660Driver;

    FileSystemDriver* driverHead = nullptr;

    FileSystem** fileSystems = nullptr;
    size_t fileSystemsSize = 0;

    size_t InsertFileSystem(FileSystem* fs) {
        for (size_t i = 0; i < fileSystemsSize; i++) {
            if (fileSystems[i] == nullptr) {
                fileSystems[i] = fs;
                return i;
            }
        }

        // No filesystems available
        // Resize the filesystem array
        size_t newFSIndex = fileSystemsSize;
        fileSystemsSize *= 2;

        if (fileSystemsSize == 0)
            fileSystemsSize = 8;

        FileSystem** newArray = (FileSystem**)malloc(sizeof(FileSystem*) * fileSystemsSize * 2);
        for (size_t i = 0; i < newFSIndex; i++)
            newArray[i] = fileSystems[i];

        for (size_t i = newFSIndex; i < fileSystemsSize; i++)
            newArray[i] = nullptr;

        newArray[newFSIndex] = fs;

        if (fileSystems != nullptr)
            free(fileSystems);

        fileSystems = newArray;
        return newFSIndex;
    }

    void RegisterDrive(DeviceManager::Device* drive, uint64_t driveSize) {
        // TODO: Check for GPT

        // Assuming whole disk volume
        FileSystem* newFS = (FileSystem*)malloc(sizeof(FileSystem));
        newFS->device = drive;
        newFS->length = driveSize / ISO9660_SECTOR_SIZE;
        newFS->startLBA = 0;

        if (ISO9660Driver.VerifyFileSystem(newFS)) {
            newFS->driver = &ISO9660Driver;
            ISO9660Driver.InitFileSystem(newFS);
            InsertFileSystem(newFS);
        } else {
            errorLogger.Log("Unable to find GPT or ISO9660 file system!");
            free(newFS);
        }
    }

    File* GetFile(const char* filepath) {
        char* ptr = (char*)filepath;

        // Find the starting directory
        Directory* baseDirectory = nullptr;

        if (filepath[0] == ':') {
            ptr++;
            // Relative to root of drive
            // Parse drive number
            uint64_t driveNumber = 0;
            while (*ptr != '/') {
                if (*ptr >= '0' && *ptr <= '9')
                    driveNumber = driveNumber * 10 + (*ptr - '0');
                else {
                    errorLogger.Log("Invalid filepath!");
                    return nullptr;
                }

                ptr++;
            }

            ptr++;

            if (driveNumber >= fileSystemsSize || fileSystems[driveNumber] == nullptr) {
                errorLogger.Log("Invalid drive number! (%i)", driveNumber);
                return nullptr;
            }

            baseDirectory = fileSystems[driveNumber]->rootDir;
        } else {
            errorLogger.Log("Unable to handle relative directories yet!");
            return nullptr;
        }

        // Count the number of '/' characters
        uint64_t numDirectories = 0;
        char* ptr2 = ptr;
        char* extension = nullptr;
        while (*ptr2 != 0) {
            if (*ptr2 == '/')
                numDirectories++;
            else if (*ptr2 == '.') {
                *ptr2 = 0;
                extension = ptr2 + 1;
                break;
            }

            ptr2++;
        }

        // Parse the names
        const char** directoryNames = (const char**)malloc(numDirectories * sizeof(const char**));
        for (uint64_t i = 0; i < numDirectories; i++) {
            directoryNames[i] = ptr;

            while (*ptr != '/')
                ptr++;

            *ptr = 0;
            ptr++;
        }

        // Open the directories
        Directory* currentDirectory = baseDirectory;
        for (uint64_t i = 0; i < numDirectories; i++) {
            const char* directoryName = directoryNames[i];
            bool foundDir = false;
            for (Directory* dir = currentDirectory->subDirectories; dir != nullptr; dir = dir->next) {
                if (strcmp(directoryName, dir->name) == 0) {
                    currentDirectory = dir;
                    foundDir = true;
                    break;
                }
            }

            if (!foundDir) {
                errorLogger.Log("Unable to find path %s", filepath);
                return nullptr;
            }
        }

        // Find the file
        const char* filename = ptr;
        for (File* file = currentDirectory->subFiles; file != nullptr; file = file->next) {
            if (strcmp(filename, file->name) == 0 && strcmp(extension, file->extension) == 0)
                return file;
        }

        errorLogger.Log("Unable to find file %s", filepath);
        return nullptr;
    }

    void* ReadFile(File* file) {
        void* filePtr = malloc(file->fileSize + 1);

        uint64_t bytesRead = file->fileSystem->driver->ReadFile(file, 0, filePtr, file->fileSize);
        if (bytesRead == 0) {
            errorLogger.Log("Failed to read file!");
            return nullptr;
        }

        ((char*)filePtr)[file->fileSize] = 0;

        return filePtr;
    }
}; // namespace VirtualFileSystem