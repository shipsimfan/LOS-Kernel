#pragma once

#include <dev.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

namespace VirtualFileSystem {
    struct FileSystem;
    struct FileSystemDriver;
    struct Directory;

    struct File {
        const char* name;
        const char* extension;
        size_t fileSize;

        Directory* directory;

        File* next;

        int64_t lock;

        FileSystem* fileSystem;
        void* driverInfo;
    };

    struct Directory {
        const char* name;

        Directory* parent;
        Directory* next;

        Directory* subDirectories;

        File* subFiles;

        FileSystem* fileSystem;
        void* driverInfo;
    };

    struct FileSystem {
        // Setup by VFS
        DeviceManager::Device* device;
        FileSystemDriver* driver;
        uint64_t startLBA;
        uint64_t length;

        // Setup by driver
        const char* volumeName = "";
        Directory* rootDir;

        void* driverInfo; // Optional
    };

    struct FileSystemDriver {
        const char* name;

        FileSystemDriver* next = nullptr;

        bool (*VerifyFileSystem)(FileSystem*);
        void (*InitFileSystem)(FileSystem*);

        // uint64_t ReadFile(File* file, size_t offset, void* buffer, size_t bufferSize)
        size_t (*ReadFile)(File*, size_t, void*, size_t);
    };

    struct FileDescriptor {
        File* file;
        uint64_t offset;
    };

    void RegisterDrive(DeviceManager::Device* drive, uint64_t driveSize);

    int Open(const char* filename);
    size_t Read(int fd, void* buf, size_t count);
    int Close(int fd);

    size_t Seek(int fd, size_t offset, int whence);
} // namespace VirtualFileSystem