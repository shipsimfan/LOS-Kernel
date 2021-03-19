#include <console.h>
#include <device/manager.h>
#include <fs.h>
#include <process/control.h>
#include <string.h>
#include <time.h>

extern "C" uint64_t SystemCall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    switch (num) {
    case 0:
        Exit(arg1);
        while (1)
            asm volatile("hlt");

    case 1:
        if (arg3 >= KERNEL_VMA)
            break;

        return Device::WriteStream(arg1, arg2, (void*)arg3, arg4);

    case 2:
        if (arg3 >= KERNEL_VMA)
            break;

        return Device::ReadStream(arg1, arg2, (void*)arg3, arg4);

    case 3: {
        if (arg1 >= KERNEL_VMA || arg2 >= KERNEL_VMA || arg3 >= KERNEL_VMA)
            return 0;

        // Copy args into kernel space
        int argc = 0;
        const char** argvUser = (const char**)arg2;
        if (argvUser != nullptr) {
            for (; (uint64_t)argvUser[argc] < KERNEL_VMA && argvUser[argc]; argc++)
                ;

            if ((uint64_t)argvUser[argc] >= KERNEL_VMA)
                return 0;
        }

        char** argvKernel = new char*[argc + 1];
        int i;
        for (i = 0; i < argc; i++) {
            char* arg = new char[strlen(argvUser[i]) + 1];
            strcpy(arg, argvUser[i]);
            argvKernel[i] = arg;
        }
        argvKernel[i] = nullptr;

        // Copy environment variables into kernel space
        int envc = 0;
        const char** envpUser = (const char**)arg3;
        if (envpUser != nullptr) {
            for (; (uint64_t)envpUser[envc] < KERNEL_VMA && envpUser[envc]; envc++)
                ;

            if ((uint64_t)envpUser[envc] >= KERNEL_VMA)
                return 0;
        }

        char** envpKernel = new char*[envc + 1];
        for (i = 0; i < envc; i++) {
            char* envVar = new char[strlen(envpUser[i]) + 1];
            strcpy(envVar, envpUser[i]);
            envpKernel[i] = envVar;
        }
        envpKernel[i] = nullptr;

        uint64_t ret = Execute((const char*)arg1, (const char**)argvKernel, (const char**)envpKernel);
        for (i = 0; i < argc; i++)
            delete argvKernel[i];
        for (i = 0; i < envc; i++)
            delete envpKernel[i];
        delete argvKernel;
        delete envpKernel;
        return ret;
    }

    case 4:
        return Wait(arg1);

    case 5:
        if (arg1 >= KERNEL_VMA)
            break;

        return Open((const char*)arg1, arg2);

    case 6:
        Close(arg1);
        return 0;

    case 7:
        if (arg2 >= KERNEL_VMA)
            break;

        return Read(arg1, (void*)arg2, arg3);

    case 8:
        return Seek(arg1, arg2, arg3);

    case 9:
        return Tell(arg1);

    case 10:
        if (arg2 >= KERNEL_VMA)
            break;

        return Write(arg1, (void*)arg2, arg3);

    case 12:
        if (arg1 >= KERNEL_VMA)
            break;

        return GetCurrentWorkingDirectory((void*)arg1, arg2);

    case 13:
        if (arg1 >= KERNEL_VMA)
            return 1;

        return ChangeDirectory((const char*)arg1);

    case 14:
        return currentProcess->currentDirectory->GetNumEntries();

    case 15:
        if (arg1 >= KERNEL_VMA)
            return 0;

        return currentProcess->currentDirectory->GetEntries((Dirent*)arg1, arg2);

    case 16:
        return GetCurrentTime();

    case 17:
        return Truncate(arg1, arg2);

    default:
        Console::Println("Unhandled system call (%#llx)", num);
    }

    return ~0;
}