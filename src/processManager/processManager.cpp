#include <proc.h>

#include <elf.h>
#include <fs.h>
#include <logger.h>
#include <stdlib.h>
#include <string.h>

namespace ProcessManager {
    extern "C" void SystemReturn(uint64_t addr);

    void ExecuteNewProcess(const char* filepath) {
        infoLogger.Log("Executing new process: %s", filepath);

        // Verify the file by getting
        VirtualFileSystem::File* file = VirtualFileSystem::GetFile(filepath);
        if (file == nullptr) {
            errorLogger.Log("Failed to find file");
            return;
        }

        // Load the file
        void* filePtr = VirtualFileSystem::ReadFile(file);
        if (filePtr == nullptr) {
            errorLogger.Log("Failed to load file!");
            return;
        }

        // Prepare new process
        Process* newProcess = (Process*)malloc(sizeof(Process));
        newProcess->name = (char*)malloc(strlen(file->name) + 1);
        strcpy((char*)newProcess->name, file->name);

        // Load new process paging structures
        newProcess->cr3 = MemoryManager::Virtual::CreateNewPagingStructure();
        MemoryManager::Virtual::SetPageStructure(newProcess->cr3);

        // Properly load the file into user space
        if (ELF::VerifyElfExecHeader(filePtr)) {
            errorLogger.Log("Invalid ELF header!");
            return;
        }

        debugLogger.Log("Valid ELF Header");

        void* entry = ELF::LoadExecutableIntoUserspace(filePtr);

        debugLogger.Log("Program Entry Address: %#llx", entry);

        // Execute the file
        newProcess->cpuState.cr2 = 0;
        newProcess->cpuState.r15 = 0;
        newProcess->cpuState.r14 = 0;
        newProcess->cpuState.r13 = 0;
        newProcess->cpuState.r12 = 0;
        newProcess->cpuState.r11 = 0;
        newProcess->cpuState.r10 = 0;
        newProcess->cpuState.r9 = 0;
        newProcess->cpuState.r8 = 0;
        newProcess->cpuState.rbp = 0;
        newProcess->cpuState.rsp = 0;
        newProcess->cpuState.rdi = 0;
        newProcess->cpuState.rsi = 0;
        newProcess->cpuState.rdx = 0;
        newProcess->cpuState.rcx = (uint64_t)entry;
        newProcess->cpuState.rbx = 0;
        newProcess->cpuState.rax = 0;

        debugLogger.Log("Returning!");
        SystemReturn((uint64_t)entry);
    }
}; // namespace ProcessManager