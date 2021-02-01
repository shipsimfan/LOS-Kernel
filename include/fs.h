#pragma once

#include <dev.h>

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

    void RegisterDrive(DeviceManager::Device* drive, uint64_t driveSize);

    File* GetFile(const char* filepath);
    void* ReadFile(File* file);
} // namespace VirtualFileSystem