#include <console.h>
#include <panic.h>

extern "C" void kmain() {
    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart");

    while (1)
        asm volatile("hlt");
}