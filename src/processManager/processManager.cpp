#include <proc.h>

#include <elf.h>
#include <fs.h>
#include <logger.h>
#include <stdlib.h>
#include <string.h>

namespace ProcessManager {
    extern "C" void PreemptHandler();

    extern "C" void TaskEnter(void* entry, Process* newProcess);
    extern "C" void TaskExit(Process* oldProcess);
    extern "C" void TaskSwitch();

    Process* hash[PROCESS_HASH_SIZE];
    uint64_t nextPID;

    Process* currentProcess;
    uint64_t currentProcessStackBase;

    Process* runningHead;
    Process* runningTail;

    void Init() {
        for (int i = 0; i < PROCESS_HASH_SIZE; i++)
            hash[i] = nullptr;

        currentProcess = (Process*)malloc(sizeof(Process));
        currentProcess->name = "System";
        currentProcess->pid = 0;
        currentProcess->cr3 = MemoryManager::Virtual::GetSystemPageStructure();

        // Set kernel stack base
        currentProcess->kernelStackBase = (uint64_t)(&currentProcess->kernelStack);
        currentProcess->kernelStackBase += KERNEL_STACK_SIZE;
        if (currentProcess->kernelStackBase % 16 != 0)
            currentProcess->kernelStackBase -= currentProcess->kernelStackBase % 16;
        currentProcessStackBase = currentProcess->kernelStackBase;

        currentProcess->queueNext = nullptr;
        currentProcess->hashPrev = nullptr;
        currentProcess->hashNext = nullptr;
        currentProcess->exitQueue = nullptr;
        currentProcess->fd = nullptr;
        currentProcess->fdSize = 0;

        hash[0] = currentProcess;

        nextPID = 1;
        runningHead = nullptr;
        runningTail = nullptr;
    }

    uint64_t t;
    void InitPreempt() {
        t = 0;
        InterruptHandler::InitPreemptTimer(PreemptHandler);
    }

    extern "C" void Preempt() {
        if (runningHead == nullptr)
            return;

        runningTail->queueNext = currentProcess;

        TaskSwitch();
    }

    uint64_t Execute(const char* filepath) {
        // Verify the file exists
        int file = VirtualFileSystem::Open(filepath);
        if (file == -1) {
            errorLogger.Log("Unable to locate file %s", filepath);
            return 0;
        }

        // Verify ELF header
        int status = ELF::VerifyElfExecHeader(file);
        if (status) {
            errorLogger.Log("Invalid ELF header (%i)", status);
            return 0;
        }

        // Create new process
        Process* newProcess = (Process*)malloc(sizeof(Process));
        if (newProcess == nullptr) {
            errorLogger.Log("Unable to allocate new process!");
            return 0;
        }

        // Clear process stack
        memset(&newProcess->kernelStack, 0, KERNEL_STACK_SIZE);

        // Name the process
        uint64_t filepathLen = strlen(filepath);
        size_t i;
        for (i = filepathLen; filepath[i] != '/'; i--)
            ;

        i++;

        newProcess->name = (const char*)malloc(filepathLen - i + 1);
        if (newProcess->name == nullptr) {
            errorLogger.Log("Unable to allocate name");
            return 0;
        }

        strcpy((char*)newProcess->name, filepath + i);

        // Get the process address space
        newProcess->cr3 = MemoryManager::Virtual::CreateNewPagingStructure();

        // Set null pointers
        newProcess->queueNext = nullptr;
        newProcess->exitQueue = nullptr;
        newProcess->fd = nullptr;
        newProcess->fdSize = 0;

        // Set kernel stack base
        newProcess->kernelStackBase = (uint64_t)(&newProcess->kernelStack);
        newProcess->kernelStackBase += KERNEL_STACK_SIZE;
        if (newProcess->kernelStackBase % 16 != 0)
            newProcess->kernelStackBase -= newProcess->kernelStackBase % 16;

        // Change memory space
        MemoryManager::Virtual::SetPageStructure(newProcess->cr3);

        // Load the process into the address space
        void* entry = ELF::LoadExecutableIntoUserspace(file);
        VirtualFileSystem::Close(file);

        // Get the process PID
        newProcess->pid = nextPID;
        uint64_t pid = nextPID;
        nextPID++;

        // Insert process into hash map
        uint64_t processIndex = newProcess->pid % PROCESS_HASH_SIZE;
        newProcess->hashNext = hash[processIndex];
        newProcess->hashPrev = nullptr;
        if (hash[processIndex] != nullptr)
            hash[processIndex]->hashPrev = newProcess;
        hash[processIndex] = newProcess;

        // Put the current process into the running queue
        if (runningHead == nullptr) {
            runningHead = currentProcess;
            runningTail = currentProcess;
        } else {
            runningTail->queueNext = currentProcess;
            runningTail = currentProcess;
        }
        currentProcess->queueNext = nullptr;

        // Set the TSS
        InterruptHandler::SetTSS(newProcess->kernelStackBase);

        // Set the stack base
        currentProcessStackBase = newProcess->kernelStackBase;

        // Enter the task
        TaskEnter(entry, newProcess);

        // Return the PID
        return pid;
    }

    void Exit(uint64_t status) {
        // Close all file descriptors
        for (int i = 0; i < currentProcess->fdSize; i++)
            VirtualFileSystem::Close(i);

        // Save old process
        Process* oldProcess = currentProcess;

        // Set status codes
        for (Process* p = oldProcess->exitQueue; p != nullptr; p = p->queueNext)
            p->queueData = status;

        // Awaken exit queue
        if (oldProcess->exitQueue != nullptr) {
            if (runningHead == nullptr) {
                runningHead = oldProcess->exitQueue;
                runningTail = runningHead;
            } else {
                runningTail->queueNext = oldProcess->exitQueue;
                Process* p = oldProcess->exitQueue;
                while (p->queueNext != nullptr)
                    p = p->queueNext;

                runningTail = p;
            }
        }

        // Wait for process to awaken
        while (runningHead == nullptr)
            ;

        currentProcess = runningHead;
        if (runningTail == currentProcess)
            runningTail = nullptr;

        runningHead = currentProcess->queueNext;

        // Switch address space
        MemoryManager::Virtual::SetPageStructure(currentProcess->cr3);

        // Switch TSS
        InterruptHandler::SetTSS(currentProcess->kernelStackBase);

        // Set the stack base
        currentProcessStackBase = currentProcess->kernelStackBase;

        // Remove oldProcess from hash map
        if (oldProcess->hashPrev != nullptr)
            oldProcess->hashPrev->hashNext = oldProcess->hashNext;
        if (oldProcess->hashNext != nullptr)
            oldProcess->hashNext->hashPrev = oldProcess->hashPrev;

        uint64_t hashIndex = oldProcess->pid % PROCESS_HASH_SIZE;
        if (hash[hashIndex] == oldProcess)
            hash[hashIndex] = oldProcess->hashNext;

        // Clear address space
        MemoryManager::Virtual::ClearPageStructure(oldProcess->cr3);

        // Free name
        free((void*)(oldProcess->name));

        // Exit
        TaskExit(oldProcess);
    }

    extern "C" void Schedule() {
        // Wait for a process
        while (runningHead == nullptr)
            ;

        currentProcess = runningHead;
        runningHead = runningHead->queueNext;

        currentProcess->queueNext = nullptr;

        MemoryManager::Virtual::SetPageStructure(currentProcess->cr3);
        InterruptHandler::SetTSS(currentProcess->kernelStackBase);
        currentProcessStackBase = currentProcess->kernelStackBase;
    }

    void WaitPID(uint64_t pid, uint64_t* status) {
        // Verify pid
        uint64_t hashIndex = pid % PROCESS_HASH_SIZE;
        Process* p;

        for (p = hash[hashIndex]; p != nullptr; p = p->hashNext) {
            if (p->pid == pid)
                break;
            else if (p->pid < pid) {
                *status = 0xFFFFFFFFFFFFFFFF;
                return;
            }
        }

        if (p == nullptr) {
            *status = 0xFFFFFFFFFFFFFFFF;
            return;
        }

        // Place current process in wait queue
        currentProcess->queueNext = p->exitQueue;
        p->exitQueue = currentProcess;

        // Suspend this process
        TaskSwitch();

        *status = currentProcess->queueData;
    }

    Process* GetCurrentProcess() { return currentProcess; }
}; // namespace ProcessManager

void Mutex::Lock() {
    if (value)
        value = false;
    else {
        if (queue == nullptr)
            queue = ProcessManager::currentProcess;
        else {
            ProcessManager::Process* p;
            for (p = queue; p->queueNext != nullptr; p = p->queueNext)
                ;

            p->queueNext = ProcessManager::currentProcess;
        }

        ProcessManager::currentProcess->queueNext = nullptr;

        ProcessManager::TaskSwitch();
    }
}

void Mutex::Unlock() {
    if (queue == nullptr)
        value = true;
    else {
        if (ProcessManager::runningHead == nullptr) {
            ProcessManager::runningHead = queue;
            ProcessManager::runningTail = queue;
        } else {
            ProcessManager::runningTail->queueNext = queue;
            ProcessManager::runningTail = queue;
        }

        queue = queue->queueNext;
        queue->queueNext = nullptr;
    }
}