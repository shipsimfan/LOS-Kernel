#include <console.h>
#include <interrupt.h>
#include <logger.h>
#include <mem.h>
#include <mem/defs.h>
#include <stdlib.h>

#define MAJOR_VERSION 0
#define MINOR_VERSION 4

Logger infoLogger;
Logger debugLogger;
Logger warningLogger;
Logger errorLogger;

extern "C" void kmain(MemoryMap* mmap, Console::GraphicsInfo* gmode) {
    Console::Init(gmode);

    infoLogger.Set("Kernel", Logger::TYPE::INFORMATION, 0xFFFFFFFF);
    debugLogger.Set("Kernel", Logger::TYPE::DEBUGGER, 0xFFFFFFFF);
    warningLogger.Set("Kernel", Logger::TYPE::WARNING, 0xFFFFFF00);
    errorLogger.Set("Kernel", Logger::TYPE::ERROR, 0xFFFF0000);

    infoLogger.Log("Unamed Operating System");
    infoLogger.Log("Written By: Lance Hart");
    infoLogger.Log("Version %i.%i\n", MAJOR_VERSION, MINOR_VERSION);

    InterruptHandler::Init();

    if (!MemoryManager::Init(mmap)) {
        errorLogger.Log("Unable to continue boot! Fatal eror while starting the memory manager!");
        while (1)
            ;
    }

    volatile uint32_t* ptr = (uint32_t*)(KERNEL_VMA + 0x100000000);
    ptr[0] = 0xDEADBEEF;
    infoLogger.Log("Ptr[0] = %#x", ptr[0]);

    while (1)
        ;
}