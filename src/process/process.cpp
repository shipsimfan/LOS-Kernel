#include <process/process.h>

#include <device/manager.h>
#include <fs.h>
#include <memory/heap.h>
#include <memory/virtual.h>
#include <process/control.h>
#include <string.h>

uint64_t nextID = 1;

extern "C" uint64_t stackTop;

Process::Process(const char* name) {
    // Set state
    state = State::NORMAL;

    // Select next ID
    id = nextID;
    nextID++;

    // Save name
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    // Setup descriptors
    devices = nullptr;
    devicesLength = 0;

    files = nullptr;
    filesLength = 0;

    if (currentProcess != nullptr) {
        // Create new paging structures
        currentProcess->pagingStructureMutex.Lock();
        pagingStructure = Memory::Virtual::CreateAddressSpace();
        currentProcess->pagingStructureMutex.Unlock();

        // Initialize a new stack
        stack = (uint8_t*)((uint64_t)Memory::Heap::AllocateAligned(KERNEL_STACK_SIZE, 16) + KERNEL_STACK_SIZE);
        memset((void*)((uint64_t)stack - KERNEL_STACK_SIZE), 0, KERNEL_STACK_SIZE);

        // Insert into hashmap
        int hashIdx = id % PROCESS_HASH_SIZE;

        processHashMutex.Lock();
        processHash[hashIdx].push(this);
        processHashMutex.Unlock();

        // Clone descriptors
        devicesLength = currentProcess->devicesLength;
        if (devicesLength > 0) {
            devices = new Device::Device*[devicesLength];
            for (uint64_t i = 0; i < devicesLength; i++) {
                devices[i] = currentProcess->devices[i];
                if (devices[i] != nullptr)
                    devices[i]->IncreamentRefCount();
            }
        }

        filesLength = currentProcess->filesLength;
        if (filesLength > 0) {
            files = new FileDescriptor*[filesLength];
            for (uint64_t i = 0; i < filesLength; i++) {
                files[i] = currentProcess->files[i];
                if (files[i] != nullptr)
                    files[i]->file->IncreamentRefCount();
            }
        }

        // Set current working directory
        if (currentProcess->currentDirectory != nullptr)
            currentDirectory = currentProcess->currentDirectory;
        else
            currentDirectory = GetRootDirectory(0);
    } else {
        // Clear the hashmap
        processHashMutex.Lock();
        processHash[id % PROCESS_HASH_SIZE].push(this);
        processHashMutex.Unlock();

        // Set the current process to this
        currentProcess = this;

        // Set the paging structure
        pagingStructure = Memory::Virtual::GetKernelPagingStructure();
        Memory::Virtual::SetCurrentAddressSpace(pagingStructure, &pagingStructureMutex);

        // Set the stack
        stack = (uint8_t*)&stackTop;

        // Set current directory to nullptr
        currentDirectory = nullptr;
    }
}

Process::~Process() {
    // Free the stack
    Memory::Heap::Free((void*)((uint64_t)stack - KERNEL_STACK_SIZE));

    // Free the memory
    Memory::Virtual::DeletePagingStructure(pagingStructure);

    // Remove from hashmap
    uint64_t idx = id % PROCESS_HASH_SIZE;
    processHashMutex.Lock();
    Queue<Process>::Iterator iter(&processHash[idx]);
    do {
        if (iter.value->id == id) {
            iter.Remove();
            break;
        }
    } while (iter.Next());

    // Close all descriptors
    for (uint64_t i = 0; i < devicesLength; i++)
        if (devices[i] != nullptr)
            devices[i]->DecreamentRefCount();

    for (uint64_t i = 0; i < filesLength; i++) {
        if (files[i] != nullptr) {
            files[i]->file->DecreamentRefCount();
            delete files[i];
        }
    }

    processHashMutex.Unlock();

    delete devices;
    delete files;
    delete name;
}

uint64_t Process::AddDevice(Device::Device* device) {
    for (uint64_t i = 0; i < devicesLength; i++) {
        if (devices[i] == nullptr) {
            devices[i] = device;
            return i;
        }
    }

    uint64_t newDeviceDescriptor = devicesLength;
    devicesLength = devicesLength << 1;

    if (devicesLength == 0)
        devicesLength = 8;

    Device::Device** newArray = new Device::Device*[devicesLength];

    for (uint64_t i = 0; i < newDeviceDescriptor; i++)
        newArray[i] = devices[i];

    for (uint64_t i = newDeviceDescriptor; i < devicesLength; i++)
        newArray[i] = nullptr;

    newArray[newDeviceDescriptor] = device;

    delete devices;
    devices = newArray;

    return newDeviceDescriptor;
}

void Process::RemoveDevice(Device::Device* device) {
    for (uint64_t i = 0; i < devicesLength; i++) {
        if (devices[i] == device) {
            devices[i] = nullptr;
            return;
        }
    }
}

void Process::RemoveDevice(uint64_t deviceDescriptor) {
    if (deviceDescriptor < devicesLength)
        devices[deviceDescriptor] = nullptr;
}

Device::Device* Process::GetDevice(int deviceDescriptor) {
    if ((uint64_t)deviceDescriptor >= devicesLength)
        return nullptr;

    return devices[deviceDescriptor];
}

uint64_t Process::AddFile(File* file) {
    for (uint64_t i = 0; i < filesLength; i++) {
        if (files[i] == nullptr) {
            files[i] = new FileDescriptor(file);
            return i;
        }
    }

    uint64_t newFileDescriptor = filesLength;
    filesLength = filesLength << 1;

    if (filesLength == 0)
        filesLength = 8;

    FileDescriptor** newArray = new FileDescriptor*[filesLength];

    for (uint64_t i = 0; i < newFileDescriptor; i++)
        newArray[i] = files[i];

    for (uint64_t i = newFileDescriptor; i < filesLength; i++)
        newArray[i] = nullptr;

    newArray[newFileDescriptor] = new FileDescriptor(file);

    delete files;
    files = newArray;

    return newFileDescriptor;
}

void Process::RemoveFile(File* file) {
    for (uint64_t i = 0; i < filesLength; i++) {
        if (files[i]->file == file) {
            delete files[i];
            files[i] = nullptr;
            return;
        }
    }
}

void Process::RemoveFile(uint64_t fileDescriptor) {
    if (fileDescriptor < filesLength) {
        if (files[fileDescriptor] != nullptr)
            delete files[fileDescriptor];
        files[fileDescriptor] = nullptr;
    }
}