#pragma once

#include <stdint.h>

class Semaphore {
public:
    Semaphore(uint64_t initialValue = 0, uint64_t maxValue = 0);

    void Signal();
    void Wait();

private:
    uint64_t value;
    uint64_t maxValue;
};