#pragma once

#include <interrupt.h>
#include <mem/virtual.h>

namespace ProcessManager {
    struct Process {
        const char* name;

        InterruptHandler::CPUState cpuState;
        uint64_t cr3;
    };

    void ExecuteNewProcess(const char* filepath);
} // namespace ProcessManager