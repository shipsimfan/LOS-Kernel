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

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint64_t size;
    uint64_t key;
    uint64_t descSize;
    uint32_t descVersion;
    uint32_t reserved;
    uint64_t mapAddr;
} MemoryMap_t;

typedef struct {
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t pixelFormat;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t reserved;
    uint32_t pixelsPerScanline;
    uint64_t frameBufferBase;
    uint64_t frameBufferSize;
} GraphicsMode_t;

#pragma pack(pop)

extern "C" void kmain(void* mmap, GraphicsMode_t* gmode) {
    volatile uint32_t* framebuffer = (volatile uint32_t*)gmode->frameBufferBase;

    for (uint64_t i = 0; i < gmode->frameBufferSize / 4; i++)
        framebuffer[i] = 0xFFFF00FF;

    /*multibootInfo = (multiboot2BootInformation*)((uint64_t)multibootInfo + 0xFFFF800000000000);

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
    }*/

    while (1)
        ;
}