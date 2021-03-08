#include <console.h>
#include <device/manager.h>
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
        if (arg2 >= KERNEL_VMA)
            break;

        ((uint64_t*)arg2)[0] = Wait(arg1);
        break;

    default:
        Console::Println("Unhandled system call (%#llx)", num);
    }

    return 0;
}