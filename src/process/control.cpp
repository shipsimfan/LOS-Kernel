#include <process/control.h>

#include <memory/virtual.h>
#include <process/process.h>
#include <queue.h>

Process kernelProcess("System");

Process* currentProcess = nullptr;

Queue<Process> processHash[PROCESS_HASH_SIZE];
Mutex processHashMutex;

Queue<Process> runningQueue;

extern "C" uint64_t ProperFork(Process* child);
extern "C" void TaskSwitch(Process* newProcess);
extern "C" void SetStackPointer(uint64_t newStackPointer);
extern "C" void TaskExit();

void Yield() {
    Process* newProcess = runningQueue.front();

    while (newProcess == nullptr)
        newProcess = runningQueue.front();

    runningQueue.pop();

    Memory::Virtual::SetCurrentAddressSpace(newProcess->pagingStructure, &newProcess->pagingStructureMutex);

    TaskSwitch(newProcess);
}

void QueueExecution(Process* process) { runningQueue.push(process); }

uint64_t Fork() {
    // Create the child process
    Process* child = new Process(currentProcess->name);

    // Do proper fork
    if (ProperFork(child)) {
        // Add child to running queue
        runningQueue.push(child);

        // Return the id
        return child->id;
    }

    return 0;
}

uint64_t Wait(uint64_t pid) {
    if (pid == currentProcess->id)
        return 0xFFFFFFFFFFFFFFFF;

    // Locate process
    uint64_t idx = pid % PROCESS_HASH_SIZE;

    bool found = false;

    processHashMutex.Lock();
    for (Queue<Process>::Iterator iter(&processHash[idx]); iter.value != nullptr; iter.Next()) {
        if (iter.value->id == pid) {
            found = true;
            iter.value->exit.push(currentProcess);
            break;
        }
    }
    processHashMutex.Unlock();

    if (!found)
        return 0xFFFFFFFFFFFFFFFF;

    Yield();

    return currentProcess->queueData;
}

void Exit(uint64_t status) {
    // Awaken exit queue
    for (Process* proc = currentProcess->exit.front(); proc != nullptr; proc = currentProcess->exit.front()) {
        proc->queueData = status;
        runningQueue.push(proc);
        currentProcess->exit.pop();
    }

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