#pragma once

#include <stdint.h>

class Process {
public:
    Process(const char* name);

    const char* GetName();
    uint64_t GetPID();

private:
    char* name;
    uint64_t pid;
};

extern "C" Process* currentProcess;