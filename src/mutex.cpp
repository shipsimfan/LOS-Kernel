#include <mutex.h>

#include <asm.h>
#include <panic.h>
#include <process/control.h>
#include <process/process.h>

Mutex::Mutex() { owner = nullptr; }

void Mutex::Lock() {
    if (currentProcess == nullptr)
        return;

    if (CompareExchange(&owner, 0, (uint64_t)currentProcess))
        return;

    // Sleep
    waitlist.push(currentProcess);
    Yield();
}

void Mutex::Unlock() {
    if (currentProcess == nullptr || owner == nullptr)
        return;

    if (owner == currentProcess) {
        if (waitlist.front() == nullptr)
            owner = nullptr;
        else {
            Process* nextOwner = waitlist.front();
            waitlist.pop();

            QueueExecution(nextOwner);
            owner = nextOwner;
        }
    } else
        panic("Attempting to unlock mutex owned by another process!");
}