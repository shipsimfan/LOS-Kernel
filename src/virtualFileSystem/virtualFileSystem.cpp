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

    void RegisterDrive(DeviceManager::Device* drive, uint64_t driveSize);

    int Open(const char* filename) {
        char* ptr = (char*)filename;

        // Find the starting directory
        Directory* baseDirectory = nullptr;

        if (*ptr == ':') {
            ptr++;
            // Relative to root of drive
            // Parse drive number
            uint64_t driveNumber = 0;
            while (*ptr != '/') {
                if (*ptr >= '0' && *ptr <= '9')
                    driveNumber = driveNumber * 10 + (*ptr - '0');
                else {
                    errorLogger.Log("Invalid filepath!");
                    return -1;
                }

                ptr++;
            }

            ptr++;

            if (driveNumber >= fileSystemsSize || fileSystems[driveNumber] == nullptr) {
                errorLogger.Log("Invalid drive number! (%i)", driveNumber);
                return -1;
            }

            baseDirectory = fileSystems[driveNumber]->rootDir;
        } else {
            errorLogger.Log("Unable to handle relative directories yet!");
            return -1;
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
                errorLogger.Log("Unable to find path %s", filename);
                return -1;
            }
        }

        // Find the file
        const char* filepath = ptr;
        File* file = nullptr;
        for (File* f = currentDirectory->subFiles; f != nullptr; f = f->next) {
            if (strcmp(filepath, f->name) == 0 && strcmp(extension, f->extension) == 0) {
                file = f;
                break;
            }
        }

        if (file == nullptr) {
            errorLogger.Log("Unable to find file %s", filepath);
            return -1;
        }

        // See if we can get the file
        if (file->lock < 0) {
            errorLogger.Log("Unable to open file, already opened for writing");
            return -1;
        }

        file->lock++;

        // Create the file descriptor
        FileDescriptor* newFD = (FileDescriptor*)malloc(sizeof(FileDescriptor));
        newFD->file = file;
        newFD->offset = 0;

        // Insert new file descriptor
        ProcessManager::Process* p = ProcessManager::GetCurrentProcess();
        FileDescriptor** fd = (FileDescriptor**)p->fd;
        for (int i = 0; i < p->fdSize; i++) {
            if (fd[i] == nullptr) {
                fd[i] = newFD;
                return i;
            }
        }

        // FD is full, we need to allocate
        size_t newSize = p->fdSize * 2;
        if (newSize == 0)
            newSize = 1;

        FileDescriptor** newProcFD = (FileDescriptor**)malloc(sizeof(FileDescriptor*) * newSize);
        for (int i = 0; i < p->fdSize; i++)
            newProcFD[i] = fd[i];

        newProcFD[p->fdSize] = newFD;
        int ret = p->fdSize;
        p->fdSize = newSize;
        p->fd = (void**)newProcFD;

        return ret;
    }

    size_t Read(int fd, void* buf, size_t count) {
        ProcessManager::Process* p = ProcessManager::GetCurrentProcess();
        if (fd >= p->fdSize) {
            errorLogger.Log("Invalid file descriptor");
            return 0;
        }

        FileDescriptor* f = ((FileDescriptor**)p->fd)[fd];
        if (f->offset >= f->file->fileSize) {
            memset(buf, 0, count);
            return count;
        }

        return f->file->fileSystem->driver->ReadFile(f->file, f->offset, buf, count);
    }

    int Close(int fd) {
        ProcessManager::Process* p = ProcessManager::GetCurrentProcess();
        if (fd >= p->fdSize) {
            errorLogger.Log("Invalid file descriptor");
            return 0;
        }

        FileDescriptor* f = ((FileDescriptor**)p->fd)[fd];
        ((FileDescriptor**)p->fd)[fd] = nullptr;

        f->file->lock--;
        free(f);

        return 0;
    }

    size_t Seek(int fd, size_t offset, int whence) {
        ProcessManager::Process* p = ProcessManager::GetCurrentProcess();
        if (fd >= p->fdSize) {
            errorLogger.Log("Invalid file descriptor");
            return 0;
        }

        FileDescriptor* f = ((FileDescriptor**)p->fd)[fd];

        if (whence == SEEK_SET)
            f->offset = offset;
        else if (whence == SEEK_CUR)
            f->offset += offset;
        else if (whence == SEEK_END)
            f->offset = f->file->fileSize + offset;

        return f->offset;
    }
}; // namespace VirtualFileSystem