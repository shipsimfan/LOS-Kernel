#include <console.h>
#include <process/control.h>

extern "C" uint64_t SystemCall(uint64_t num, uint64_t arg1, uint64_t arg2) {
    switch (num) {
    case 0:
        Exit(arg1);
        while (1)
            asm volatile("hlt");

    case 1:
        if (arg1 >= KERNEL_VMA)
            break;

        return Console::Print("%s", arg1);

    case 2:
        if (arg1 >= KERNEL_VMA)
            break;

        return Console::Read((void*)arg1, arg2);

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