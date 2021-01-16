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

extern "C" void kmain(multiboot2BootInformation* multibootInfo) {
    multibootInfo = (multiboot2BootInformation*)((uint64_t)multibootInfo + 0xFFFF800000000000);

    Console::SetBackgroundColor(0);
    Console::ClearScreen();

    infoLogger.Set("Kernel", Logger::TYPE::INFORMATION, Logger::COLOR::WHITE);
    debugLogger.Set("Kernel", Logger::TYPE::DEBUGGER, Logger::COLOR::WHITE);
    warningLogger.Set("Kernel", Logger::TYPE::WARNING, Logger::COLOR::YELLOW);
    errorLogger.Set("Kernel", Logger::TYPE::ERROR, Logger::COLOR::RED);

    infoLogger.Log("Unamed Operating System");
    infoLogger.Log("Written By: Lance Hart");
    infoLogger.Log("Version %i.%i\n", MAJOR_VERSION, MINOR_VERSION);

    InterruptHandler::Init();

    if (!MemoryManager::Init(multibootInfo)) {
        errorLogger.Log("Unable to continue boot! Fatal eror while starting the memory manager!");
        while (1)
            ;
    }

    while (1)
        ;
}