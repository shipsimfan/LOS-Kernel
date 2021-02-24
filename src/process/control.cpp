#include <process/control.h>

#include <memory/virtual.h>
#include <process/process.h>
#include <queue.h>

Process kernelProcess("System");

Process* currentProcess = nullptr;

Process* processHash[PROCESS_HASH_SIZE];
Mutex processHashMutex;

Queue<Process> runningQueue;

extern "C" uint64_t ProperFork(Process* child);
extern "C" void TaskSwitch(Process* newProcess);
extern "C" void SetStackPointer(uint64_t newStackPointer);
extern "C" void TaskExit();

void Schedule() {
    Process* newProcess = runningQueue.front();

    while (newProcess == nullptr)
        newProcess = runningQueue.front();

    runningQueue.pop();

    Memory::Virtual::SetCurrentAddressSpace(newProcess->pagingStructure, &newProcess->pagingStructureMutex);

    TaskSwitch(newProcess);
}

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
    for (Process* proc = processHash[idx]; proc != nullptr && proc->id <= pid; proc = proc->hashNext) {
        if (proc->id == pid) {
            found = true;
            proc->exit.push(currentProcess);
            break;
        }
    }

    if (!found)
        return 0xFFFFFFFFFFFFFFFF;

    Schedule();

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