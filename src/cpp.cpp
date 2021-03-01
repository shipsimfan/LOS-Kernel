#include <errno.h>
#include <panic.h>

uint64_t errno;

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