#include <proc.h>

#include <elf.h>
#include <fs.h>
#include <logger.h>
#include <stdlib.h>
#include <string.h>

namespace ProcessManager {
    extern "C" void TaskEnter(void* entry, Process* newProcess);
    extern "C" void TaskExit();
    extern "C" void TaskSwitch();

    Process* hash[PROCESS_HASH_SIZE];
    uint64_t nextPID;

    Process* currentProcess;
    uint64_t currentProcessStackBase;

    Process* runningQueue;

    void Init() {
        for (int i = 0; i < PROCESS_HASH_SIZE; i++)
            hash[i] = nullptr;

        currentProcess = (Process*)malloc(sizeof(Process));
        currentProcess->name = "System";
        currentProcess->pid = 0;
        currentProcess->cr3 = MemoryManager::Virtual::GetSystemPageStructure();
        debugLogger.Log("CR3: %#llx", currentProcess->cr3);

        // Set RSP
        currentProcess->kernelStackBase = (uint64_t)(&currentProcess->kernelStack);
        currentProcess->kernelStackBase += KERNEL_STACK_SIZE;
        if (currentProcess->kernelStackBase % 16 != 0)
            currentProcess->kernelStackBase -= currentProcess->kernelStackBase % 16;
        currentProcess->rsp = currentProcess->kernelStackBase;
        currentProcessStackBase = currentProcess->kernelStackBase;

        debugLogger.Log("System kernel stack size: %#llx", currentProcess->kernelStackBase);

        currentProcess->queueNext = nullptr;
        currentProcess->hashPrev = nullptr;
        currentProcess->hashNext = nullptr;
        currentProcess->exitQueue = nullptr;

        hash[0] = currentProcess;

        currentProcess->parent = nullptr;

        nextPID = 1;
        runningQueue = nullptr;
    }

    uint64_t Execute(const char* filepath) {
        // Verify the file exists
        VirtualFileSystem::File* file = VirtualFileSystem::GetFile(filepath);
        if (file == nullptr) {
            errorLogger.Log("Unable to locate file %s", filepath);
            return 0;
        }

        // Load the file
        void* filePtr = VirtualFileSystem::ReadFile(file);
        if (filePtr == nullptr) {
            errorLogger.Log("Error while loading file %s", filepath);
            return 0;
        }

        // Verify ELF header
        int status = ELF::VerifyElfExecHeader(filePtr);
        if (status) {
            errorLogger.Log("Invalid ELF header (%i)", status);
            free(filePtr);
            return 0;
        }

        // Create new process
        Process* newProcess = (Process*)malloc(sizeof(Process));
        if (newProcess == nullptr) {
            errorLogger.Log("Unable to allocate new process!");
            free(filePtr);
            return 0;
        }

        // Clear process stack
        memset(&newProcess->kernelStack, 0, KERNEL_STACK_SIZE);

        // Name the process
        newProcess->name = (const char*)malloc(strlen(file->name) + 1);
        if (newProcess->name == nullptr) {
            errorLogger.Log("Unable to allocate name");
            free(filePtr);
            return 0;
        }

        strcpy((char*)newProcess->name, file->name);

        // Get the process address space
        newProcess->cr3 = MemoryManager::Virtual::CreateNewPagingStructure();

        // Set process parent
        newProcess->parent = currentProcess;

        // Set queue next
        newProcess->queueNext = nullptr;
        newProcess->exitQueue = nullptr;

        // Set RSP
        newProcess->kernelStackBase = (uint64_t)(&newProcess->kernelStack);
        newProcess->kernelStackBase += KERNEL_STACK_SIZE;
        if (newProcess->kernelStackBase % 16 != 0)
            newProcess->kernelStackBase -= newProcess->kernelStackBase % 16;
        newProcess->rsp = newProcess->kernelStackBase;

        debugLogger.Log("Kernel Stack base: %#llx", newProcess->kernelStackBase);

        // Change memory space
        MemoryManager::Virtual::SetPageStructure(newProcess->cr3);

        // Load the process into the address space
        void* entry = ELF::LoadExecutableIntoUserspace(filePtr);
        free(filePtr);

        // Get the process PID
        newProcess->pid = nextPID;
        nextPID++;

        // Insert process into hash map
        uint64_t processIndex = newProcess->pid % PROCESS_HASH_SIZE;
        newProcess->hashNext = hash[processIndex];
        newProcess->hashPrev = nullptr;
        if (hash[processIndex] != nullptr)
            hash[processIndex]->hashPrev = newProcess;
        hash[processIndex] = newProcess;

        // Put the current process into the running queue
        if (runningQueue == nullptr)
            runningQueue = currentProcess;
        else {
            Process* p;
            for (p = runningQueue; p->queueNext != nullptr; p = p->queueNext)
                ;

            p->queueNext = currentProcess;
        }

        currentProcess->queueNext = nullptr;

        // Set the TSS
        InterruptHandler::SetTSS(newProcess->kernelStackBase);

        // Set the stack base
        currentProcessStackBase = newProcess->kernelStackBase;

        debugLogger.Log("New Process: %#llx", newProcess);

        // Enter the task
        TaskEnter(entry, newProcess);

        debugLogger.Log("Current Process: %#llx", currentProcess);

        // Return the PID
        return newProcess->pid;
    }

    void Exit(uint64_t status) {
        // Save old process
        Process* oldProcess = currentProcess;

        // Set status codes
        for (Process* p = oldProcess->exitQueue; p != nullptr; p = p->queueNext)
            p->queueData = status;

        // Awaken exit queue
        if (runningQueue == nullptr)
            runningQueue = oldProcess->exitQueue;
        else {
            Process* p;
            for (p = runningQueue; p->queueNext != nullptr; p = p->queueNext)
                ;

            p->queueNext = oldProcess->exitQueue;
        }

        // Switch process
        if (runningQueue == nullptr) {
            errorLogger.Log("Nothing to switch to!");
            while (1)
                ;
        }

        currentProcess = runningQueue;
        runningQueue = currentProcess->queueNext;

        // Switch address space
        MemoryManager::Virtual::SetPageStructure(currentProcess->cr3);

        // Exit children . . .

        // Remove process from hash map
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

        // Switch TSS
        InterruptHandler::SetTSS(currentProcess->kernelStackBase);

        // Set the stack base
        currentProcessStackBase = currentProcess->kernelStackBase;

        // Free old process

        // Switch
        TaskExit();
    }

    extern "C" void Schedule() {
        if (runningQueue == nullptr) {
            errorLogger.Log("Nothing to go to!");
            while (1)
                ;
        }

        currentProcess = runningQueue;
        runningQueue = runningQueue->queueNext;

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
}; // namespace ProcessManager