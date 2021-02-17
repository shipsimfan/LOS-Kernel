#pragma once

#include <stdint.h>
#include <mutex.h>

class Process {
public:
    Process(const char* name);

    const char* GetName();
    uint64_t GetPID();

    static Process* CURRENT_PROCESS;
    static Mutex CURRENT_PROCESS_MUTEX;

private:
    char* name;
    uint64_t pid;
};
