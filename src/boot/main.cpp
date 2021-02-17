#include <console.h>
#include <memory/physical.h>
#include <panic.h>

extern "C" void kmain() {
    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("Total Memory: %i MB", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE);
    Console::Println("Free Memory: %i MB", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE);

    while (1)
        asm volatile("hlt");
}

extern "C" void __cxa_pure_virtual() { panic("Attempting to invocate a pure virtual function!"); }