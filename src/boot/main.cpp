#include <console.h>
#include <memory/physical.h>

#include <process/control.h>
#include <process/process.h>

extern "C" void kmain() {
    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("Total Memory: %i MB", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE);
    Console::Println("Free Memory: %i MB", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE);

    uint64_t pid = Fork();

    if (pid == 0) {
        Console::Println("Child (%i) starting . . . ", currentProcess->id);
        for (int i = 0; i < 32; i++)
            Console::Println("Child (%i): %i", currentProcess->id, i);
        Exit(0);
    } else {
        Console::Println("Parent (%i) starting . . . ", currentProcess->id);
        for (int i = 0; i < 32; i++)
            Console::Println("Parent (%i): %i", currentProcess->id, i);
        Wait(pid);
    }

    Console::Println("Going to sleep");

    while (1)
        asm volatile("hlt");
}