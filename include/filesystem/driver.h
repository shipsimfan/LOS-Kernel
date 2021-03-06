#pragma once

#include <device/device.h>

class Filesystem;
class Directory;

class FilesystemDriver {
public:
    virtual int64_t DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) = 0;

private:
    char* name;
};

class File {
private:
    char* name;
    char* extension;

    int64_t size;

    Directory* directory;
    Filesystem* filesystem;

    int64_t lock;
};

class Directory {
public:
    Directory(const char* name, Directory* parent, Filesystem* filesystem);

private:
    char* name;

    Directory* parent;
    Filesystem* filesystem;

    Queue<Directory> subDirectories;
    Queue<File> files;
};

class Filesystem {
public:
    Filesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name);

    void SetRootDirectory(Directory* newRootDir);

private:
    Device::Device* drive;
    FilesystemDriver* driver;

    uint64_t startLBA;
    int64_t length;

    char* volumeName;

    Directory* rootDir;
};

void RegisterDrive(Device::Device* drive, int64_t driveSize);
void RegisterFilesystem(Filesystem* filesystem);
void RegisterFilesystemDriver(FilesystemDriver* driver);