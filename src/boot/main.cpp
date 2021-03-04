#include <console.h>
#include <memory/physical.h>

#include <memory/heap.h>
#include <string.h>

#include <process/control.h>
#include <process/process.h>

extern "C" void kmain() {
    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("Total Memory: %i MB (%i KB)", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetTotalPages() * PAGE_SIZE) / KILOBYTE);
    Console::Println("Free Memory: %i MB (%i KB)", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetFreePages() * PAGE_SIZE) / KILOBYTE);

    uint64_t pid = Fork();

    if (pid == 0) {
        Console::Println("Child starting (%i) . . . ", currentProcess->id);
        for (int i = 0; i < 5; i++)
            Console::Println("Child (%i): %i", currentProcess->id, i);
        Exit(0);
    }

    Console::Println("Current Free Memory: %i KB", (Memory::Physical::GetFreePages() * PAGE_SIZE) / KILOBYTE);

    Wait(pid);

    Console::Println("%i exited, current free memory: %i KB", pid, (Memory::Physical::GetFreePages() * PAGE_SIZE) / KILOBYTE);

    while (1)
        asm volatile("hlt");
}