#include <console.h>
#include <memory/physical.h>
#include <panic.h>
#include <process/control.h>
#include <process/process.h>

extern "C" void kmain() {
    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("Total Memory: %i MB", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE);
    Console::Println("Free Memory: %i MB", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE);

    while (1)
        asm volatile("hlt");
}

extern "C" void __cxa_pure_virtual() { panic("Attempting to invocate a pure virtual function!"); }

struct atexitFuncEntry {
    void (*destructorFunc)(void*);
    void* objPtr;
    void* dsoHandle;
};

atexitFuncEntry __atexitFuncs[128];
uint64_t __atexitFuncCount = 0;

void* __dso_handle;

extern "C" int __cxa_atexit(void (*f)(void*), void* objPtr, void* dso) {
    if (__atexitFuncCount >= 128)
        return -1;

    __atexitFuncs[__atexitFuncCount].destructorFunc = f;
    __atexitFuncs[__atexitFuncCount].objPtr = objPtr;
    __atexitFuncs[__atexitFuncCount].dsoHandle = dso;

    return 0;
}

extern "C" void __cxa_finalize(void* f) {
    uint64_t i = __atexitFuncCount;
    if (!f) {
        while (i--) {
            if (__atexitFuncs[i].destructorFunc)
                __atexitFuncs[i].destructorFunc(__atexitFuncs[i].objPtr);
        }

        return;
    }

    while (i--) {
        if (__atexitFuncs[i].destructorFunc == f) {
            __atexitFuncs[i].destructorFunc(__atexitFuncs[i].objPtr);
            __atexitFuncs[i].destructorFunc = 0;
        }
    }
}