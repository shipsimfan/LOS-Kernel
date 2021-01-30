#pragma once

#include <dev.h>

namespace VirtualFileSystem {
    struct FileSystemDriver;

    struct FileSystem {
        DeviceManager::Device* device;
        FileSystemDriver* driver;

        void* driverInfo;

        const char* volumeName = "";
        uint64_t startLBA;
        uint64_t length;
    };

    struct FileSystemDriver {
        const char* name;

        FileSystemDriver* next = nullptr;

        bool (*VerifyFileSystem)(FileSystem*);
        void (*InitFileSystem)(FileSystem*) = nullptr;
    };

    void RegisterDrive(DeviceManager::Device* drive, uint64_t driveSize);
} // namespace VirtualFileSystem