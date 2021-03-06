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

    Console::Println("About to execute!");
    uint64_t pid = Execute(":0/LOS/SHELL.APP");

    if (pid == 0)
        Console::Println("Error while executing!");
    else
        Console::Println("New process PID: %i", pid);

    while (1)
        asm volatile("hlt");
}