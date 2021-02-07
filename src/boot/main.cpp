#include <console.h>
#include <dev.h>
#include <interrupt.h>
#include <kernel/time.h>
#include <logger.h>
#include <mem.h>
#include <mem/defs.h>
#include <proc.h>
#include <stdlib.h>

#define MAJOR_VERSION 0
#define MINOR_VERSION 4

Logger infoLogger;
Logger debugLogger;
Logger warningLogger;
Logger errorLogger;

extern "C" void kmain(MemoryMap* mmap, Console::GraphicsInfo* gmode, void* rdsp) {
    Console::Init(gmode);

    infoLogger.Set("Kernel", Logger::TYPE::INFORMATION, 0xFFFFFFFF);
    debugLogger.Set("Kernel", Logger::TYPE::DEBUGGER, 0xFFFFFFFF);
    warningLogger.Set("Kernel", Logger::TYPE::WARNING, 0xFFFFFF00);
    errorLogger.Set("Kernel", Logger::TYPE::ERROR, 0xFFFF0000);

    infoLogger.Log("Lance Operating System");
    infoLogger.Log("Written By: Lance Hart");
    infoLogger.Log("Version %i.%i\n", MAJOR_VERSION, MINOR_VERSION);

    InterruptHandler::Init();

    if (!MemoryManager::Init(mmap)) {
        errorLogger.Log("Unable to continue boot! Fatal eror while starting the memory manager!");
        while (1)
            ;
    }

    Console::InitDoubleBuffering();

    ProcessManager::Init();

    if (!DeviceManager::Init(rdsp)) {
        errorLogger.Log("Unable to continue boot! Fatal error while starting the device manager!");
        while (1)
            ;
    }

    uint64_t pid = ProcessManager::Execute(":0/LOS/SHELL.APP");

    uint64_t shellStatus;
    ProcessManager::WaitPID(pid, &shellStatus);

    Shutdown();

    while (1)
        ;
}