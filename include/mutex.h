#pragma once

class Mutex {
public:
    Mutex();

    void Lock();
    void Unlock();

private:
    bool val;
};