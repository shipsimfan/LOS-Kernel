#include <mutex.h>

#include <asm.h>
#include <panic.h>
#include <process.h>

Mutex::Mutex() { owner = nullptr; }

void Mutex::Lock() {
    if (Process::CURRENT_PROCESS == nullptr)
        return;

    if (CompareExchange(&owner, 0, (uint64_t)Process::CURRENT_PROCESS | 1))
        return;

    // Spinlock

    // Sleep
    panic("Mutex sleep not implmented yet");
}

void Mutex::Unlock() {
    if (Process::CURRENT_PROCESS == nullptr || owner == nullptr)
        return;

    if ((uint64_t)owner == ((uint64_t)Process::CURRENT_PROCESS | 1))
        owner = nullptr;
    else
        panic("Attempting to unlock mutex owned by another process!");
}