#include <process/process.h>

#include <memory/heap.h>
#include <memory/virtual.h>
#include <string.h>

uint64_t nextID = 1;

extern "C" uint64_t stackTop;

Process::Process(const char* name) {
    // Select next ID
    id = nextID;
    nextID++;

    // Save name
    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    if (currentProcess != nullptr) {
        // Copy current address paging structure
        currentProcess->pagingStructureMutex.Lock();
        pagingStructure = Memory::Virtual::CopyAddressSpace(currentProcess->pagingStructure);
        currentProcess->pagingStructureMutex.Unlock();

        // Initialize a new stack
        stack = (uint8_t*)((uint64_t)Memory::Heap::AllocateAligned(KERNEL_STACK_SIZE, 16) + KERNEL_STACK_SIZE);
        memset((void*)((uint64_t)stack - KERNEL_STACK_SIZE), 0, KERNEL_STACK_SIZE);

        // Insert into hashmap
        int hashIdx = id % PROCESS_HASH_SIZE;

        processHashMutex.Lock();
        processHash[hashIdx].push(this);
        processHashMutex.Unlock();
    } else {
        // Clear the hashmap
        processHashMutex.Lock();
        processHash[id % PROCESS_HASH_SIZE].push(this);
        processHashMutex.Unlock();

        // Set the current process to this
        currentProcess = this;

        // Set the paging structure
        pagingStructure = Memory::Virtual::GetKernelPagingStructure();
        Memory::Virtual::SetCurrentAddressSpace(pagingStructure, &pagingStructureMutex);

        // Set the stack
        stack = (uint8_t*)&stackTop;
    }
}

Process::~Process() {
    Memory::Heap::Free((void*)((uint64_t)stack - KERNEL_STACK_SIZE));
    Memory::Virtual::DeletePagingStructure(pagingStructure);
    delete name;
}