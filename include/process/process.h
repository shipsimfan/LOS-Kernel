#pragma once

#include <device/device.h>
#include <filesystem/driver.h>
#include <memory/defs.h>
#include <mutex.h>
#include <queue.h>
#include <stdint.h>

#define KERNEL_STACK_SIZE 32768
#define PROCESS_HASH_SIZE 1024

extern "C" void SetKernelProcess();

struct Process {
    enum State { NORMAL, UNINTERRUPTABLE };

    Process(const char* name);
    ~Process();

    uint64_t kernelStackPointer;
    uint8_t* stack;

    char* name;
    uint64_t id;

    PhysicalAddress pagingStructure;
    Mutex pagingStructureMutex;

    Queue<Process> exit;
    uint64_t queueData;

    uint64_t errno;

    Device::Device** devices;
    uint64_t devicesLength;

    FileDescriptor** files;
    uint64_t filesLength;

    State state;

    friend void ::SetKernelProcess();

    uint64_t AddDevice(Device::Device* device);
    void RemoveDevice(Device::Device* device);
    void RemoveDevice(uint64_t deviceDescriptor);

    uint64_t AddFile(File* file);
    void RemoveFile(File* file);
    void RemoveFile(uint64_t fileDescriptor);
};

extern Process* currentProcess;
extern Process kernelProcess;

extern Mutex processHashMutex;
extern Queue<Process> processHash[PROCESS_HASH_SIZE];