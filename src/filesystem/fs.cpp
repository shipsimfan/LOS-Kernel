#include <filesystem/driver.h>
#include <fs.h>

#include <console.h>
#include <errno.h>
#include <string.h>

Mutex filesystemsMutex;
Filesystem** filesystems = nullptr;
uint64_t filesystemsSize = 0;

Queue<FilesystemDriver> filesystemDrivers;
Mutex filesystemDriversMutex;

File::File(const char* name, const char* extension, int64_t size, Directory* directory, Filesystem* filesystem) : size(size), directory(directory), filesystem(filesystem), refCount(0) {
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    this->extension = new char[strlen(extension) + 1];
    strcpy(this->extension, extension);
}

void File::IncreamentRefCount() {
    if (refCount == (uint64_t)~0)
        return;

    refCount++;
}

void File::DecreamentRefCount() {
    if (refCount == 0)
        return;

    refCount--;
}

Filesystem* File::GetFilesystem() { return filesystem; }
int64_t File::GetSize() { return size; }
const char* File::GetName() { return name; }
const char* File::GetExtension() { return extension; }

Directory::Directory(const char* name, Directory* parent, Filesystem* filesystem) : filesystem(filesystem) {
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    if (parent == nullptr)
        this->parent = this;
    else
        this->parent = parent;
}

void Directory::AddSubDirectory(Directory* directory) { subDirectories.push(directory); }
void Directory::AddSubFile(File* file) { files.push(file); }

const char* Directory::GetName() { return name; }
Queue<Directory>* Directory::GetSubDirectories() { return &subDirectories; }
Queue<File>* Directory::GetFiles() { return &files; }

Filesystem::Filesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name) : drive(drive), driver(driver), startLBA(startLBA), length(length) {
    volumeName = new char[strlen(name) + 1];
    strcpy(volumeName, name);
}

int64_t Filesystem::Read(uint64_t address, void* buffer, int64_t count) { return drive->ReadStream(address, buffer, count); }
int64_t Filesystem::Write(uint64_t address, void* buffer, int64_t count) { return drive->WriteStream(address, buffer, count); }

Device::Device* Filesystem::GetDrive() { return drive; }
FilesystemDriver* Filesystem::GetDriver() { return driver; }
Directory* Filesystem::GetRootDirectory() { return rootDir; }

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

FileDescriptor::FileDescriptor(File* file) : file(file), offset(0) {}

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

int Open(const char* filepath) {
    // Parse the file path
    const char* ptr = filepath;
    Directory* currentDirectory;

    filesystemsMutex.Lock();
    if (*ptr == ':') {
        // Absolute filepath
        // Find the drive
        uint64_t driveNumber = 0;
        ptr++;
        while (*ptr != '/' && *ptr != '\\') {
            if (*ptr < '0' || *ptr > '9') {
                errno = ERROR_BAD_PARAMETER;
                filesystemsMutex.Unlock();
                return -1;
            }

            driveNumber *= 10;
            driveNumber += *ptr - '0';
            ptr++;
        }

        if (driveNumber > filesystemsSize) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        if (filesystems[driveNumber] == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        currentDirectory = filesystems[driveNumber]->GetRootDirectory();

        ptr++;
    } else {
        // Relative filepath

        // TODO: setu directory to present working directory
        errno = ERROR_BAD_PARAMETER;
        filesystemsMutex.Unlock();
        return -1;
    }

    const char* start;
    const char* lastDot = nullptr;
    while (1) {
        start = ptr;
        while (*ptr != '/' && *ptr != '\\' && *ptr != 0) {
            if (*ptr == '.')
                lastDot = ptr;
            ptr++;
        }

        if (*ptr == 0)
            break;

        if (start == ptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        Queue<Directory>* directories = currentDirectory->GetSubDirectories();
        if (directories->front() == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        Queue<Directory>::Iterator iter(directories);
        bool found = false;
        do {
            if (strlen(iter.value->GetName()) != (uint64_t)(ptr - start))
                continue;

            if (memcmp(iter.value->GetName(), start, ptr - start))
                continue;

            found = true;
            currentDirectory = iter.value;
        } while (iter.Next());

        if (!found) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        ptr++;
    }

    if (start == ptr) {
        errno = ERROR_BAD_PARAMETER;
        filesystemsMutex.Unlock();
        return -1;
    }

    if (lastDot <= start)
        lastDot = ptr;

    Queue<File>* files = currentDirectory->GetFiles();
    if (files->front() == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    Queue<File>::Iterator iter(files);
    File* file = nullptr;
    do {
        if (strlen(iter.value->GetName()) != (uint64_t)(lastDot - start))
            continue;

        if (memcmp(iter.value->GetName(), start, lastDot - start))
            continue;

        if (strlen(iter.value->GetExtension()) != (uint64_t)(ptr - lastDot - 1))
            continue;

        if (memcmp(iter.value->GetExtension(), lastDot + 1, ptr - lastDot - 1))
            ;

        file = iter.value;
    } while (iter.Next());

    filesystemsMutex.Unlock();

    if (file == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    file->IncreamentRefCount();
    return currentProcess->AddFile(file);
}

void Close(int fd) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return;
    }

    currentProcess->files[fd]->file->DecreamentRefCount();
    delete currentProcess->files[fd];
    currentProcess->files[fd] = nullptr;
}

int64_t Read(int fd, void* buffer, int64_t count) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd]->offset >= currentProcess->files[fd]->file->GetSize()) {
        memset(buffer, 0, count);
        return count;
    }

    return currentProcess->files[fd]->file->GetFilesystem()->GetDriver()->Read(currentProcess->files[fd]->file, currentProcess->files[fd]->offset, buffer, count);
}

int64_t Seek(int fd, int64_t offset, int whence) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (whence == SEEK_SET)
        currentProcess->files[fd]->offset = offset;
    else if (whence == SEEK_CUR)
        currentProcess->files[fd]->offset += offset;
    else if (whence == SEEK_END)
        currentProcess->files[fd]->offset = currentProcess->files[fd]->file->GetSize() + offset;

    return currentProcess->files[fd]->offset;
}