#pragma once

#include <stdint.h>

class Spinlock {
public:
    Spinlock();

    void Acquire();
    void Release();

private:
    uint64_t value;
};