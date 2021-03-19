#include <console.h>
#include <device/acpi/acpi.h>
#include <device/drivers/ide.h>
#include <device/drivers/pci.h>
#include <device/drivers/ps2.h>
#include <device/drivers/uefi.h>
#include <device/manager.h>
#include <filesystem/drivers/fat.h>
#include <filesystem/drivers/iso9660.h>
#include <memory/physical.h>
#include <process/control.h>

extern "C" void kmain() {
    // Initialize the video driver
    InitializeUEFIVideoDriver();

    Queue<Device::Device> consoles;
    uint64_t count = Device::GetDevices(Device::Device::Type::CONSOLE, consoles);
    if (count > 0) {
        Device::Open(consoles.front());
        Console::SetStdOutput(consoles.front());
    }

    Console::Println("Lance Operating System");
    Console::Println("Written by: Lance Hart\n");

    Console::Println("[ MEM ] Total Memory: %i MB (%i KB)", (Memory::Physical::GetTotalPages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetTotalPages() * PAGE_SIZE) / KILOBYTE);
    Console::Println("[ MEM ] Free Memory: %i MB (%i KB)", (Memory::Physical::GetFreePages() * PAGE_SIZE) / MEGABYTE, (Memory::Physical::GetFreePages() * PAGE_SIZE) / KILOBYTE);

    Console::Println("[ LOS ] Loading built-in drivers . . .");
    InitializeISO9660();
    InitializeFAT();
    InitializePCIDriver();
    InitializeIDEDriver();
    InitializePS2Driver();

    Queue<Device::Device> keyboards;
    count = Device::GetDevices(Device::Device::Type::KEYBOARD, keyboards);
    if (count > 0) {
        Device::Open(keyboards.front());
        Console::SetStdInput(keyboards.front());
    }

    Console::Println("[ LOS ] Executing shell . . .");
    uint64_t pid = Execute(":0/los/shell.app", nullptr, nullptr);

    if (pid == 0) {
        Console::SetForegroundColor(0xFF, 0x00, 0x00);
        Console::Println("[ LOS ] Error while executing!");
    } else {
        uint64_t status = Wait(pid);
        Console::Println("[ LOS ] Shell exited with status %#llX", status);
    }

    Console::Print("Press any key to shutdown . . . ");
    char exitKey;
    Console::Read(&exitKey, 1);

    ACPI::Shutdown();
}