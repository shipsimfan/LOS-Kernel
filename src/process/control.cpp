#include <process/control.h>

#include "elf.h"

#include <console.h>
#include <fs.h>
#include <interrupt/stack.h>
#include <memory/virtual.h>
#include <pair.h>
#include <process/process.h>
#include <queue.h>

Process kernelProcess("System");

Process* currentProcess = nullptr;

Queue<Process> processHash[PROCESS_HASH_SIZE];
Mutex processHashMutex;

Queue<Process> runningQueue;

Queue<Pair<uint64_t, uint64_t>> zombie;
Mutex zombieMutex;

extern "C" uint64_t ProperFork(Process* child);
extern "C" void TaskSwitch(Process* newProcess);
extern "C" void SetStackPointer(uint64_t newStackPointer);
extern "C" void TaskExit();
extern "C" void TaskEnter(Process* newProcess, uint64_t entry);

uint64_t test = 0;

void Preempt() {
    if (currentProcess == nullptr)
        return;

    if (currentProcess->state == Process::State::UNINTERRUPTABLE)
        return;

    if (runningQueue.front() == nullptr)
        return;

    QueueExecution(currentProcess);

    asm volatile("sti");

    Yield();
}

void Yield() {
    Process* newProcess = runningQueue.front();

    while (newProcess == nullptr)
        newProcess = runningQueue.front();

    runningQueue.pop();

    Memory::Virtual::SetCurrentAddressSpace(newProcess->pagingStructure, &newProcess->pagingStructureMutex);
    Interrupt::SetInterruptStack((uint64_t)newProcess->stack);

    TaskSwitch(newProcess);
}

void QueueExecution(Process* process) { runningQueue.push(process); }

uint64_t Fork() {
    // Create the child process
    Process* child = new Process(currentProcess->name);

    // Copy stack pointer
    child->userStackPointer = currentProcess->userStackPointer;

    // Copy address space
    Memory::Virtual::CopyCurrentAddressSpace(child->pagingStructure);

    // Do proper fork
    if (ProperFork(child)) {
        // Add child to running queue
        runningQueue.push(child);

        // Return the id
        return child->id;
    }

    return 0;
}

uint64_t Execute(const char* filepath) {
    // Open the file
    int fd = Open(filepath);
    if (fd < 0)
        return 0;

    // Verify the elf header
    if (!VerifyELFExecutable(fd))
        return 0;

    // Get the file name
    const char* lastSlash = filepath;
    for (int i = 0; filepath[i]; i++)
        if (filepath[i] == '/' || filepath[i] == '\\')
            lastSlash = filepath + i;

    if (lastSlash != filepath)
        lastSlash++;

    // Create a new process
    Process* newProcess = new Process(lastSlash);

    // Switch to the new process address space
    currentProcess->state = Process::State::UNINTERRUPTABLE;
    Memory::Virtual::SetCurrentAddressSpace(newProcess->pagingStructure, &newProcess->pagingStructureMutex);

    // Load the file into the new address space
    uint64_t entry = LoadELFExecutable(fd);

    Close(fd);

    if (entry >= KERNEL_VMA) {
        currentProcess->state = Process::State::NORMAL;
        return 0;
    }

    // Switch process and entry
    QueueExecution(currentProcess);
    asm volatile("cli");
    currentProcess->state = Process::State::NORMAL;
    Interrupt::SetInterruptStack((uint64_t)newProcess->stack);
    TaskEnter(newProcess, entry);
    return newProcess->id;
}

uint64_t Wait(uint64_t pid) {
    if (pid == currentProcess->id)
        return 0xFFFFFFFFFFFFFFFF;

    // Locate process
    uint64_t idx = pid % PROCESS_HASH_SIZE;

    bool found = false;

    processHashMutex.Lock();
    if (processHash[idx].front() == nullptr) {
        processHashMutex.Unlock();
        zombieMutex.Lock();
        if (zombie.front() == nullptr) {
            zombieMutex.Unlock();
            return 0xFFFFFFFFFFFFFFFF;
        }

        Queue<Pair<uint64_t, uint64_t>>::Iterator iter(&zombie);
        do {
            if (iter.value->key == pid) {
                zombieMutex.Unlock();
                uint64_t ret = iter.value->value;
                iter.Remove();
                return ret;
            }
        } while (iter.Next());

        zombieMutex.Unlock();
        return 0xFFFFFFFFFFFFFFFF;
    }

    Queue<Process>::Iterator iter(&processHash[idx]);
    do {
        if (iter.value->id == pid) {
            found = true;
            iter.value->exit.push(currentProcess);
            break;
        }
    } while (iter.Next());

    processHashMutex.Unlock();

    if (!found)
        return 0xFFFFFFFFFFFFFFFF;

    Yield();

    return currentProcess->queueData;
}

void Exit(uint64_t status) {
    // Awaken exit queue
    if (currentProcess->exit.front() != nullptr) {
        for (Process* proc = currentProcess->exit.front(); proc != nullptr; proc = currentProcess->exit.front()) {
            proc->queueData = status & 0xFF;
            QueueExecution(proc);
            currentProcess->exit.pop();
        }
    } else
        zombie.push(new Pair<uint64_t, uint64_t>(currentProcess->id, status & 0xFF));

    // Exit
    register Process* oldProcess = currentProcess;

    currentProcess = runningQueue.front();
    while (currentProcess == nullptr)
        runningQueue.front();

    runningQueue.pop();

    Memory::Virtual::SetCurrentAddressSpace(currentProcess->pagingStructure, &currentProcess->pagingStructureMutex);

    SetStackPointer(currentProcess->kernelStackPointer);

    delete oldProcess;

    TaskExit();
}