#pragma once

#include <queue.h>

class Process;

class Mutex {
public:
    Mutex();

    void Lock();
    void Unlock();

private:
    Process* owner;

    Queue<Process> waitlist;
};