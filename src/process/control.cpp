#include <process/control.h>

#include "elf.h"

#include <console.h>
#include <fs.h>
#include <interrupt/stack.h>
#include <memory/virtual.h>
#include <pair.h>
#include <process/process.h>
#include <queue.h>
#include <string.h>

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
extern "C" void TaskEnter(Process* newProcess, uint64_t entry, char* stackBottom, int argc, const char** argv, const char** envp);

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

uint64_t Execute(const char* filepath, const char** args, const char** env) {
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

    // Count number of arguments
    int argc = 0;
    if (args != nullptr) {
        const char** ptr = args;
        for (; *ptr; argc++)
            ptr++;
    }

    // Prepare start of the stack
    char* stackBottom = (char*)0x7FFFFFFFFFFF;

    // Copy arguments into user space
    const char** argvUser = new const char*[argc];
    for (int i = 0; i < argc; i++) {
        stackBottom -= strlen(args[i]) + 1;
        strcpy(stackBottom, args[i]);
        argvUser[i] = stackBottom;
    }

    // Count number of environment variables
    int envc = 0;
    if (env != nullptr) {
        const char** ptr = env;
        for (; *ptr; envc++)
            ptr++;
    }

    // Copy environment variables into user space
    const char** envpUser = new const char*[envc];
    for (int i = 0; i < envc; i++) {
        stackBottom -= strlen(env[i]) + 1;
        strcpy(stackBottom, env[i]);
        envpUser[i] = stackBottom;
    }

    // Align stack pointer
    stackBottom = (char*)((uint64_t)stackBottom & 0x7FFFFFFFFFF8);

    // Fill argv
    const char** argv = (const char**)stackBottom;
    argv -= argc + 1;
    for (int i = 0; i < argc; i++) {
        *argv = argvUser[i];
        argv++;
    }
    *argv = nullptr;
    argv -= argc;

    delete argvUser;

    // Fill envp
    const char** envp = argv;
    envp -= envc + 1;
    for (int i = 0; i < envc; i++) {
        *envp = envpUser[i];
        envp++;
    }
    *envp = nullptr;
    envp -= envc;

    delete envpUser;

    // Align stack pointer
    stackBottom = (char*)((uint64_t)envp & 0x7FFFFFFFFFF0);

    // Switch process and entry
    QueueExecution(currentProcess);
    asm volatile("cli");
    currentProcess->state = Process::State::NORMAL;
    Interrupt::SetInterruptStack((uint64_t)newProcess->stack);
    TaskEnter(newProcess, entry, stackBottom, argc, argv, envp);
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

int GetCurrentWorkingDirectory(void* ptr, uint64_t size) {
    char* text = (char*)ptr;
    if (currentProcess == nullptr || currentProcess->currentDirectory == nullptr) {
        text[0] = 0;
        return 0;
    }

    char* name = currentProcess->currentDirectory->GetFullName();
    if (size <= strlen(name))
        return -1;

    strcpy(text, name);
    return 0;
}