#include <fs.h>

#include <logger.h>
#include <stdlib.h>

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
            ISO9660Driver.InitFileSystem(newFS);
            InsertFileSystem(newFS);
        } else {
            errorLogger.Log("Unable to find GPT or ISO9660 file system!");
            free(newFS);
        }
    }
}; // namespace VirtualFileSystem