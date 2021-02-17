#include <process.h>

#include <string.h>

uint64_t nextPID = 0;

extern "C" Process* currentProcess = nullptr;

Process kernelProcess("System");

extern "C" void SetCurrentProcessToKernel() { currentProcess = &kernelProcess; }

Process::Process(const char* name) {
    // Select next PID
    pid = nextPID;
    nextPID++;

    // Save name
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);
}

const char* Process::GetName() { return name; }

uint64_t Process::GetPID() { return pid; }