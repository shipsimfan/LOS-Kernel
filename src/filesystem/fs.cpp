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

File::File(const char* name, int64_t size, Directory* directory, Filesystem* filesystem, uint64_t flags) : size(size), directory(directory), filesystem(filesystem), refCount(0), flags(flags) {
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);
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

void File::SetSize(int64_t newSize) { size = newSize; }

Filesystem* File::GetFilesystem() { return filesystem; }
int64_t File::GetSize() { return size; }
const char* File::GetName() { return name; }
uint64_t File::GetFlags() { return flags; }
Directory* File::GetDirectory() { return directory; }

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

char* Directory::GetFullName() {
    if (parent == this) {
        char* fsName = new char[32];
        fsName[0] = ':';
        int i = 1;
        int fs = filesystem->GetNumber();
        do {
            fsName[i] = (fs % 10) + '0';
            fs /= 10;
            i++;
        } while (fs);

        fsName[i] = '/';
        fsName[i + 1] = 0;
        return fsName;
    } else {
        char* parentName = parent->GetFullName();
        char* fullName = new char[strlen(parentName) + strlen(name) + 2];

        strcpy(fullName, parentName);
        strcat(fullName, name);
        strcat(fullName, "/");

        delete parentName;

        return fullName;
    }
}

Directory* Directory::GetParent() { return parent; }
uint64_t Directory::GetNumEntries() { return subDirectories.count() + files.count() + 2; }

uint64_t Directory::GetEntries(Dirent* entries, uint64_t size) {
    uint64_t numEntries = GetNumEntries();
    if (sizeof(Dirent) * numEntries > size)
        return 0;

    entries[0].type = DIRENT_TYPE_DIRECTORY;
    entries[1].type = DIRENT_TYPE_DIRECTORY;

    strcpy(entries[0].name, ".");
    strcpy(entries[1].name, "..");

    int i = 2;
    if (subDirectories.front() != nullptr) {
        Queue<Directory>::Iterator iter(&subDirectories);
        do {
            entries[i].type = DIRENT_TYPE_DIRECTORY;
            strcpy(entries[i].name, iter.value->GetName());
            i++;
        } while (iter.Next());
    }

    if (files.front() != nullptr) {
        Queue<File>::Iterator iter(&files);
        do {
            entries[i].type = DIRENT_TYPE_FILE;
            entries[i].size = iter.value->GetSize();
            entries[i].flags = iter.value->GetFlags();
            strcpy(entries[i].name, iter.value->GetName());
            i++;
        } while (iter.Next());
    }

    return numEntries;
}

Filesystem::Filesystem(Device::Device* drive, FilesystemDriver* driver, uint64_t startLBA, int64_t length, const char* name, bool readOnly) : drive(drive), driver(driver), startLBA(startLBA), length(length), readOnly(readOnly), filesystemNumber(-1) {
    volumeName = new char[strlen(name) + 1];
    strcpy(volumeName, name);
}

int64_t Filesystem::Read(uint64_t address, void* buffer, int64_t count) { return drive->ReadStream(address, buffer, count); }
int64_t Filesystem::Write(uint64_t address, void* buffer, int64_t count) {
    if (readOnly) {
        errno = ERROR_READ_ONLY;
        return -1;
    }

    return drive->WriteStream(address, buffer, count);
}

Device::Device* Filesystem::GetDrive() { return drive; }
FilesystemDriver* Filesystem::GetDriver() { return driver; }
Directory* Filesystem::GetRootDirectory() { return rootDir; }

void Filesystem::SetRootDirectory(Directory* newRootDir) {
    rootDir = newRootDir;
    rootDir->parent = newRootDir;
}

void Filesystem::SetNumber(int number) { filesystemNumber = number; }
int Filesystem::GetNumber() { return filesystemNumber; }

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
    filesystem->SetNumber(newFilesystemIndex);

    if (filesystems != nullptr)
        delete filesystems;

    filesystems = newArray;

    filesystemsMutex.Unlock();
}

FileDescriptor::FileDescriptor(File* file, int flags) : file(file), offset(0), flags(flags) {}

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

int Open(const char* filepath, int flags) {
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

        if (!filesystems || filesystems[driveNumber] == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        currentDirectory = filesystems[driveNumber]->GetRootDirectory();

        ptr++;
    } else {
        if (currentProcess->currentDirectory == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        currentDirectory = currentProcess->currentDirectory;
    }

    if (currentDirectory == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        filesystemDriversMutex.Unlock();
        return -1;
    }

    const char* start;
    while (1) {
        start = ptr;
        while (*ptr != '/' && *ptr != '\\' && *ptr != 0)
            ptr++;

        if (*ptr == 0)
            break;

        if (start == ptr) {
            errno = ERROR_BAD_PARAMETER;
            filesystemsMutex.Unlock();
            return -1;
        }

        if (ptr - start == 1) {
            if (start[0] == '.') {
                ptr++;
                continue;
            }
        } else if (ptr - start == 2) {
            if (start[0] == '.' && start[1] == '.') {
                currentDirectory = currentDirectory->GetParent();
                ptr++;
                continue;
            }
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

    Queue<File>* files = currentDirectory->GetFiles();
    if (files->front() == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    Queue<File>::Iterator iter(files);
    File* file = nullptr;
    do {
        if (strlen(iter.value->GetName()) != (uint64_t)(ptr - start))
            continue;

        if (memcmp(iter.value->GetName(), start, ptr - start))
            continue;

        file = iter.value;
        break;
    } while (iter.Next());

    filesystemsMutex.Unlock();

    if (file == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    file->IncreamentRefCount();
    return currentProcess->AddFile(file, flags);
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

    if (!(currentProcess->files[fd]->flags & OPEN_READ)) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd]->offset >= currentProcess->files[fd]->file->GetSize()) {
        memset(buffer, 0, count);
        return count;
    }

    if (currentProcess->files[fd]->offset + count >= currentProcess->files[fd]->file->GetSize()) {
        int64_t fileOverrun = (currentProcess->files[fd]->offset + count) - currentProcess->files[fd]->file->GetSize();
        count -= fileOverrun;
        memset((uint8_t*)buffer + count, 0, fileOverrun);
    }

    return currentProcess->files[fd]->file->GetFilesystem()->GetDriver()->Read(currentProcess->files[fd]->file, currentProcess->files[fd]->offset, buffer, count);
}

int64_t Write(int fd, void* buffer, int64_t count) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (!(currentProcess->files[fd]->flags & OPEN_WRITE)) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    return currentProcess->files[fd]->file->GetFilesystem()->GetDriver()->Write(currentProcess->files[fd]->file, currentProcess->files[fd]->offset, buffer, count);
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

int64_t Tell(int fd) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    return currentProcess->files[fd]->offset;
}

int64_t Truncate(int fd, int64_t newSize) {
    if (fd >= (int)currentProcess->filesLength) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (currentProcess->files[fd] == nullptr) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    if (!(currentProcess->files[fd]->flags & OPEN_WRITE)) {
        errno = ERROR_BAD_PARAMETER;
        return -1;
    }

    return currentProcess->files[fd]->file->GetFilesystem()->GetDriver()->Truncate(currentProcess->files[fd]->file, newSize);
}

int GetNumFilesystems() { return filesystemsSize; }

Directory* GetRootDirectory(int filesystem) {
    Directory* ret = nullptr;
    filesystemDriversMutex.Lock();
    if (filesystem < (int)filesystemsSize && filesystems[filesystem] != nullptr)
        ret = filesystems[filesystem]->GetRootDirectory();
    filesystemDriversMutex.Unlock();

    return ret;
}

uint64_t ChangeDirectory(const char* path) {
    // Parse the file path
    const char* ptr = path;
    Directory* currentDirectory;

    filesystemsMutex.Lock();
    if (*ptr == ':') {
        // Absolute filepath
        // Find the drive
        uint64_t driveNumber = 0;
        ptr++;
        while (*ptr != '/' && *ptr != '\\' && *ptr != 0) {
            if (*ptr < '0' || *ptr > '9') {
                filesystemsMutex.Unlock();
                return 1;
            }

            driveNumber *= 10;
            driveNumber += *ptr - '0';
            ptr++;
        }

        if (driveNumber > filesystemsSize) {
            filesystemsMutex.Unlock();
            return 1;
        }

        if (filesystems[driveNumber] == nullptr) {
            filesystemsMutex.Unlock();
            return 1;
        }

        currentDirectory = filesystems[driveNumber]->GetRootDirectory();

        if (*ptr == 0) {
            currentProcess->currentDirectory = currentDirectory;
            filesystemsMutex.Unlock();
            return 0;
        }

        ptr++;
    } else {
        if (currentProcess->currentDirectory == nullptr) {
            filesystemsMutex.Unlock();
            return 1;
        }

        currentDirectory = currentProcess->currentDirectory;
    }

    const char* start;
    while (1) {
        start = ptr;
        while (*ptr != '/' && *ptr != '\\' && *ptr != 0)
            ptr++;

        if (start == ptr) {
            if (*ptr == 0)
                break;
            filesystemsMutex.Unlock();
            return 1;
        }

        if (ptr - start == 1) {
            if (start[0] == '.') {
                if (*ptr == 0)
                    break;
                ptr++;
                continue;
            }
        } else if (ptr - start == 2) {
            if (start[0] == '.' && start[1] == '.') {
                currentDirectory = currentDirectory->GetParent();
                if (*ptr == 0)
                    break;
                ptr++;
                continue;
            }
        }

        Queue<Directory>* directories = currentDirectory->GetSubDirectories();
        if (directories->front() == nullptr) {
            filesystemsMutex.Unlock();
            return 1;
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
            filesystemsMutex.Unlock();
            return 1;
        }

        if (*ptr == 0)
            break;

        ptr++;
    }

    currentProcess->currentDirectory = currentDirectory;

    filesystemsMutex.Unlock();

    return 0;
}