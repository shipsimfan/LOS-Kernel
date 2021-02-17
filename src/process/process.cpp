#include <process.h>

#include <string.h>

uint64_t nextPID = 0;

Process* Process::CURRENT_PROCESS = nullptr;
Mutex Process::CURRENT_PROCESS_MUTEX;

Process kernelProcess("System");

extern "C" void SetCurrentProcessToKernel() { 
    Process::CURRENT_PROCESS_MUTEX.Lock();
    Process::CURRENT_PROCESS = &kernelProcess;
    Process::CURRENT_PROCESS_MUTEX.Unlock();
}

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