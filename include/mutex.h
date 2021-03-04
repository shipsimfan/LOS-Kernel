#pragma once

#include <queue.h>

class Process;

class Mutex {
public:
    Mutex();

    void Lock();
    void Unlock();

    Process* GetOwner();

private:
    Process* owner;

    Queue<Process> waitlist;
};