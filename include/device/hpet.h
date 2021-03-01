#pragma once

#include <stdint.h>

class HPET {
public:
    HPET();

private:
    uint64_t* address;
};