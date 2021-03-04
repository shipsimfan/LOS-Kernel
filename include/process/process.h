#pragma once

#include <memory/defs.h>
#include <mutex.h>
#include <queue.h>
#include <stdint.h>

#define KERNEL_STACK_SIZE 32768
#define PROCESS_HASH_SIZE 1024

extern "C" void SetKernelProcess();

struct Process {
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

    friend void ::SetKernelProcess();
};

extern Process* currentProcess;
extern Process kernelProcess;

extern Mutex processHashMutex;
extern Queue<Process> processHash[PROCESS_HASH_SIZE];