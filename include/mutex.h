#pragma once

class Process;

class Mutex {
public:
    Mutex();

    void Lock();
    void Unlock();

private:
    Process* owner;
};