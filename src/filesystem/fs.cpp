#include <filesystem/driver.h>

#include <console.h>
#include <errno.h>
#include <string.h>

Mutex filesystemsMutex;
Filesystem** filesystems = nullptr;
uint64_t filesystemsSize = 0;

Queue<FilesystemDriver> filesystemDrivers;
Mutex filesystemDriversMutex;

File::File(const char* name, const char* extension, int64_t size, Directory* directory, Filesystem* filesystem) : size(size), directory(directory), filesystem(filesystem), lock(0) {
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    this->extension = new char[strlen(extension) + 1];
    strcpy(this->extension, extension);

    Console::Println("New file: %s.%s", this->name, this->extension);
}

Directory::Directory(const char* name, Directory* parent, Filesystem* filesystem) : filesystem(filesystem) {
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    if (parent == nullptr)
        this->parent = this;
    else
        this->parent = parent;

    Console::Println("New Directory: %s", this->name);
}

void Directory::AddSubDirectory(Directory* directory) { subDirectories.push(directory); }
void Directory::AddSubFile(File* file) { files.push(file); }

Filesystem::Filesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name) : drive(drive), driver(driver), startLBA(startLBA), length(length) {
    volumeName = new char[strlen(name) + 1];
    strcpy(volumeName, name);
}

int64_t Filesystem::Read(uint64_t address, void* buffer, int64_t count) { return drive->ReadStream(address, buffer, count); }
int64_t Filesystem::Write(uint64_t address, void* buffer, int64_t count) { return drive->WriteStream(address, buffer, count); }

void Filesystem::SetRootDirectory(Directory* newRootDir) { rootDir = newRootDir; }

void RegisterFilesystem(Filesystem* filesystem) {
    filesystemsMutex.Lock();
    for (uint64_t i = 0; i < filesystemsSize; i++) {
        if (filesystems[i] == nullptr) {
            filesystems[i] = filesystem;
            filesystemsMutex.Unlock();
            return;
        }
    }

    // No filesystems available
    // Resize the filesystem array
    uint64_t newFilesystemIndex = filesystemsSize;
    filesystemsSize = filesystemsSize << 1;

    if (filesystemsSize == 0)
        filesystemsSize = 8;

    Filesystem** newArray = new Filesystem*[filesystemsSize];
    for (uint64_t i = 0; i < newFilesystemIndex; i++)
        newArray[i] = filesystems[i];

    for (uint64_t i = newFilesystemIndex; i < filesystemsSize; i++)
        newArray[i] = nullptr;

    newArray[newFilesystemIndex] = filesystem;

    if (filesystems != nullptr)
        delete filesystems;

    filesystems = newArray;

    filesystemsMutex.Unlock();
}

void RegisterDrive(Device::Device* drive, int64_t driveSize) {
    if (driveSize < 0)
        return;

    // TODO: Check for GPT

    // If no GPT, assume the volume spans the whole disk
    filesystemDriversMutex.Lock();
    if (filesystemDrivers.front() != nullptr) {
        Queue<FilesystemDriver>::Iterator iter(&filesystemDrivers);
        do {
            int64_t status = iter.value->DetectFilesystem(drive, 0, driveSize);
            if (status == 0)
                break;
            else if (status < 0) {
                Console::Println("[ Filesystem ] Error while detecting filesystem (%#X)", errno);
                break;
            }
        } while (iter.Next());
    }
    filesystemDriversMutex.Unlock();
}

void RegisterFilesystemDriver(FilesystemDriver* driver) {
    filesystemDriversMutex.Lock();
    filesystemDrivers.push(driver);
    filesystemDriversMutex.Unlock();
}