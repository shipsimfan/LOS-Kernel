#include <console.h>
#include <device/manager.h>
#include <fs.h>
#include <process/control.h>

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

    case 3:
        if (arg1 >= KERNEL_VMA)
            break;

        return Execute((const char*)arg1);

    case 4:
        return Wait(arg1);

    case 5:
        if (arg1 >= KERNEL_VMA)
            break;

        return Open((const char*)arg1);

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

    default:
        Console::Println("Unhandled system call (%#llx)", num);
    }

    return ~0;
}