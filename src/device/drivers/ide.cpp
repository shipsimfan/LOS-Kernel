#include <device/drivers/ide.h>

#include "ide.h"
#include "pci.h"

#include <console.h>
#include <device/drivers/pci.h>
#include <device/manager.h>
#include <device/util.h>
#include <errno.h>

void InitializeIDEDriver() {
    Queue<Device::Device> queue;
    Device::GetDevices(Device::Device::Type::PCI_DEVICE, queue);

    if (queue.front() != nullptr) {
        Queue<Device::Device>::Iterator iter(&queue);
        do {
            uint64_t devClass, subClass;
            iter.value->Open();
            if (iter.value->Read(PCI_CONFIG_CLASS, &devClass) != SUCCESS) {
                iter.value->Close();
                Console::SetForegroundColor(0xFF, 0x00, 0x00);
                Console::Println("Failed to read PCI device class!");
                Console::SetForegroundColor(0xFF, 0xFF, 0xFF);
                continue;
            }

            if (iter.value->Read(PCI_CONFIG_SUB_CLASS, &subClass) != SUCCESS) {
                iter.value->Close();
                Console::SetForegroundColor(0xFF, 0x00, 0x00);
                Console::Println("Failed to read PCI device sub class!");
                Console::SetForegroundColor(0xFF, 0xFF, 0xFF);
                continue;
            }

            iter.value->Close();

            if (devClass == 1 && subClass == 1) {
                IDEDevice* newDevice = new IDEDevice();
                Device::RegisterDevice(nullptr, newDevice);
                PCIDevice* oldDevice = (PCIDevice*)iter.value;
                iter.Remove();
                delete oldDevice;
                return;
            }
        } while (iter.Next());
    }
}

IDEDevice::IDEDevice() : Device("IDE Controller", Type::IDE_CONTROLLER) { Console::Println("Initializing IDE Controller . . ."); }

uint64_t IDEDevice::OnOpen() { return SUCCESS; }
uint64_t IDEDevice::OnClose() { return SUCCESS; }

uint64_t IDEDevice::DoRead(uint64_t address, uint64_t* value) {
    uint8_t reg = address & 0xFF;
    uint8_t channel = (address >> 8) & 1;

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    if (reg < 0x08)
        *value = inb(channels[channel].IO + reg);
    else if (reg < 0x0C)
        *value = inb(channels[channel].IO + reg - 0x06);
    else if (reg < 0x0E)
        *value = inb(channels[channel].control + reg - 0x0A);
    else if (reg < 0x16)
        *value = inb(channels[channel].busMaster + reg - 0x0E);

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    return SUCCESS;
}

uint64_t IDEDevice::DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) { return ERROR_NOT_IMPLEMENTED; }

uint64_t IDEDevice::DoWrite(uint64_t address, uint64_t value) {
    uint8_t reg = address & 0xFF;
    uint8_t channel = (address >> 8) & 1;

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    if (reg < 0x08)
        outb(channels[channel].IO + reg, value);
    else if (reg < 0x0C)
        outb(channels[channel].IO + reg - 0x06, value);
    else if (reg < 0x0E)
        outb(channels[channel].control + reg - 0x0A, value);
    else if (reg < 0x16)
        outb(channels[channel].busMaster + reg - 0x0E, value);

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    return SUCCESS;
}

uint64_t IDEDevice::DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) { return ERROR_NOT_IMPLEMENTED; }