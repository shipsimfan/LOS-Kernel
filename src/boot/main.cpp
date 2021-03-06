#include <console.h>
#include <memory/physical.h>

#include <memory/heap.h>
#include <string.h>

#include <device/drivers/ide.h>
#include <device/drivers/pci.h>
#include <device/drivers/uefi.h>
#include <device/manager.h>
#include <filesystem/drivers/iso9660.h>
#include <fs.h>
#include <process/control.h>
#include <process/process.h>

extern "C" void kmain() {
    UEFIVideoDevice* videoDevice = new UEFIVideoDevice;
    Device::RegisterDevice(nullptr, videoDevice);
    Console::SetVideoDevice(videoDevice);

    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("Total Memory: %i MB (%i KB)", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetTotalPages() * PAGE_SIZE) / KILOBYTE);
    Console::Println("Free Memory: %i MB (%i KB)", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetFreePages() * PAGE_SIZE) / KILOBYTE);

    InitializeISO9660();
    InitializePCIDriver();
    InitializeIDEDriver();

    int fd = Open(":0/LOS/SHELL.APP");
    if (fd < 0)
        Console::Println("Failed to open file!");
    else {
        Console::Println("File opened!");
        char* buffer = new char[16];
        Read(fd, buffer, 16);
        Console::Println("ELF check: %c%c%c", buffer[1], buffer[2], buffer[3]);
        Close(fd);
    }

    while (1)
        asm volatile("hlt");
}