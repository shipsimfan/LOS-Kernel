#pragma once

#include <interrupt.h>
#include <mem/virtual.h>

#define KERNEL_STACK_SIZE 8192
#define PROCESS_HASH_SIZE 1024

namespace ProcessManager {
#pragma pack(push)
#pragma pack(1)

    struct Process {
        uint64_t rsp;
        uint64_t kernelRSP;
        uint64_t kernelStackBase;

        uint64_t cr3;

        const char* name;
        uint64_t pid;

        Process* parent;
        Process* child;

        Process* nextChild;

        Process* hashNext;
        Process* hashPrev;

        Process* queueNext;
        uint64_t queueData;

        Process* exitQueue;

        void** fd;
        int fdSize;

        uint8_t kernelStack[KERNEL_STACK_SIZE];
    };

#pragma pack(pop)

    extern "C" Process* currentProcess;
    extern "C" uint64_t currentProcessStackBase;

    void Init();

    uint64_t Execute(const char* filepath);
    void Exit(uint64_t status);

    void WaitPID(uint64_t pid, uint64_t* status);

    Process* GetCurrentProcess();
} // namespace ProcessManager

struct Mutex {
    bool value;
    ProcessManager::Process* queue;

    void Lock();
    void Unlock();
};