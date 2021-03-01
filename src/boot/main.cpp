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