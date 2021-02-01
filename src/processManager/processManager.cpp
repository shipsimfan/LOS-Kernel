#include <proc.h>

#include <fs.h>
#include <logger.h>
#include <stdlib.h>
#include <string.h>

namespace ProcessManager {
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
        debugLogger.Log("Parsing ELF file");

        // Execute the file
    }
}; // namespace ProcessManager