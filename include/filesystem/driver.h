#pragma once

#include <device/device.h>

#define DIRENT_TYPE_FILE 0
#define DIRENT_TYPE_DIRECTORY 1

struct Dirent {
    char name[128];
    uint8_t type;
    uint64_t size;
};

class Filesystem;
class Directory;
class File;

class FilesystemDriver {
public:
    virtual int64_t DetectFilesystem(Device::Device* drive, uint64_t startLBA, int64_t size) = 0;

    virtual int64_t Read(File* file, int64_t offset, void* buffer, int64_t count) = 0;
    virtual int64_t Write(File* file, int64_t offset, void* buffer, int64_t count) = 0;

private:
    char* name;
};

class File {
public:
    File(const char* name, int64_t size, Directory* directory, Filesystem* filesystem);

    void IncreamentRefCount();
    void DecreamentRefCount();

    Filesystem* GetFilesystem();
    int64_t GetSize();
    const char* GetName();

private:
    char* name;

    int64_t size;

    Directory* directory;
    Filesystem* filesystem;

    uint64_t refCount;
};

class Directory {
    friend Filesystem;

public:
    Directory(const char* name, Directory* parent, Filesystem* filesystem);

    void AddSubDirectory(Directory* directory);
    void AddSubFile(File* file);

    Queue<Directory>* GetSubDirectories();
    Queue<File>* GetFiles();
    const char* GetName();
    char* GetFullName();
    Directory* GetParent();
    uint64_t GetNumEntries();

    uint64_t GetEntries(Dirent* entries, uint64_t size);

private:
    char* name;

    Directory* parent;
    Filesystem* filesystem;

    Queue<Directory> subDirectories;
    Queue<File> files;
};

class Filesystem {
public:
    Filesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name, bool readOnly = false);

    int64_t Read(uint64_t address, void* buffer, int64_t count);
    int64_t Write(uint64_t address, void* buffer, int64_t count);

    Device::Device* GetDrive();
    FilesystemDriver* GetDriver();

    Directory* GetRootDirectory();

    void SetRootDirectory(Directory* newRootDir);

    void SetNumber(int number);
    int GetNumber();

private:
    Device::Device* drive;
    FilesystemDriver* driver;

    uint64_t startLBA;
    int64_t length;

    char* volumeName;

    Directory* rootDir;

    bool readOnly;

    int filesystemNumber;
};

struct FileDescriptor {
    FileDescriptor(File* file, int flags);

    File* file;
    int64_t offset;

    int flags;
};

void RegisterDrive(Device::Device* drive, int64_t driveSize);
void RegisterFilesystem(Filesystem* filesystem);
void RegisterFilesystemDriver(FilesystemDriver* driver);