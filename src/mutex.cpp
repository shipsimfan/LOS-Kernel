#include <mutex.h>

#include <panic.h>

Mutex::Mutex() { val = true; }

void Mutex::Lock() {
    if (!val)
        panic("Locking already locked mutex!");

    val = false;
}

void Mutex::Unlock() { val = true; }