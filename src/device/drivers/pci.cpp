#include <device/drivers/pci.h>

#include "pci.h"

#include <console.h>
#include <device/manager.h>
#include <device/util.h>
#include <errno.h>

void WriteConfigB(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    outd(0xCFC, val << ((offset & 3) * 8));
}

void WriteConfigW(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    outd(0xCFC, val << ((offset & 2) * 8));
}

void WriteConfigD(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    outd(0xCFC, val);
}

uint8_t ReadConfigB(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    uint32_t tmp = ind(0xCFC);
    return (uint8_t)((tmp >> ((offset & 3) * 8)) & 0xff);
}

uint16_t ReadConfigW(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    return (uint16_t)((ind(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint32_t ReadConfigD(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000);

    outd(0xCF8, address);
    return ind(0xCFC);
}

void CheckPCIBus(uint8_t bus);

void CheckPCIFunction(uint8_t bus, uint8_t device, uint8_t function) {
    // Register device
    PCIDevice* newDevice = new PCIDevice(bus, device, function);
    Device::RegisterDevice(nullptr, newDevice);

    uint64_t devClass, subClass;
    newDevice->Open();
    newDevice->Read(PCI_CONFIG_CLASS, &devClass);
    newDevice->Read(PCI_CONFIG_SUB_CLASS, &subClass);
    newDevice->Close();

    Console::Println("Registering new PCI Device: %#X - %#X", (uint8_t)devClass, (uint8_t)subClass);

    if (devClass == 0x06 && subClass == 0x04) {
        uint8_t secondaryBus = ReadConfigB(bus, device, function, PCI_CONFIG_SECONDARY_BUS_NUMBER);
        CheckPCIBus(secondaryBus);
    }
}

void CheckPCIDevice(uint8_t bus, uint8_t device) {
    uint8_t function = 0;

    uint16_t vendorID = ReadConfigW(bus, device, function, PCI_CONFIG_VENDOR_ID);
    if (vendorID == 0xFFFF)
        return;

    CheckPCIFunction(bus, device, function);
    uint8_t headerType = ReadConfigB(bus, device, function, PCI_CONFIG_HEADER_TYPE);
    if ((headerType & 0x80) != 0) {
        for (function = 1; function < 8; function++)
            if (ReadConfigW(bus, device, function, PCI_CONFIG_VENDOR_ID) != 0xFFFF)
                CheckPCIFunction(bus, device, function);
    }
}

void CheckPCIBus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++)
        CheckPCIDevice(bus, device);
}

void InitializePCIDriver() {
    uint8_t headerType = ReadConfigB(0, 0, 0, PCI_CONFIG_HEADER_TYPE);
    if ((headerType & 0x80) == 0)
        CheckPCIBus(0);
    else {
        for (uint8_t function = 0; function < 8; function++) {
            if (ReadConfigW(0, 0, function, PCI_CONFIG_VENDOR_ID) != 0xFFFF)
                break;

            CheckPCIBus(function);
        }
    }
}

PCIDevice::PCIDevice(uint8_t bus, uint8_t device, uint8_t function) : Device("PCI Device", Type::PCI_DEVICE), bus(bus), device(device), function(function) {}

uint64_t PCIDevice::OnOpen() { return SUCCESS; }
uint64_t PCIDevice::OnClose() { return SUCCESS; }

uint64_t PCIDevice::DoRead(uint64_t address, uint64_t* value) {
    switch (address) {
    case PCI_CONFIG_CLASS:
    case PCI_CONFIG_SUB_CLASS:
    case PCI_CONFIG_PROG_IF:
    case PCI_CONFIG_REVISION:
    case PCI_CONFIG_BIST:
    case PCI_CONFIG_HEADER_TYPE:
    case PCI_CONFIG_LATENCY_TIMER:
    case PCI_CONFIG_CACHE_LINE_SIZE:
    case PCI_CONFIG_INT_LINE:
    case PCI_CONFIG_INT_PIN:
        *value = ReadConfigB(bus, device, function, address);
        return SUCCESS;

    case PCI_CONFIG_DEVICE_ID:
    case PCI_CONFIG_VENDOR_ID:
        *value = ReadConfigW(bus, device, function, address);
        return SUCCESS;

    case PCI_CONFIG_BAR_0:
    case PCI_CONFIG_BAR_1:
    case PCI_CONFIG_BAR_2:
    case PCI_CONFIG_BAR_3:
    case PCI_CONFIG_BAR_4:
    case PCI_CONFIG_BAR_5:
        *value = ReadConfigD(bus, device, function, address);
        return SUCCESS;

    default:
        return ERROR_BAD_PARAMETER;
    }
}

uint64_t PCIDevice::DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) { return ERROR_NOT_IMPLEMENTED; }

uint64_t PCIDevice::DoWrite(uint64_t address, uint64_t value) {
    switch (address) {
    case PCI_CONFIG_CLASS:
    case PCI_CONFIG_SUB_CLASS:
    case PCI_CONFIG_PROG_IF:
    case PCI_CONFIG_REVISION:
    case PCI_CONFIG_BIST:
    case PCI_CONFIG_HEADER_TYPE:
    case PCI_CONFIG_LATENCY_TIMER:
    case PCI_CONFIG_CACHE_LINE_SIZE:
    case PCI_CONFIG_INT_LINE:
    case PCI_CONFIG_INT_PIN:
        WriteConfigB(bus, device, function, address, value);
        return SUCCESS;

    case PCI_CONFIG_DEVICE_ID:
    case PCI_CONFIG_VENDOR_ID:
        WriteConfigW(bus, device, function, address, value);
        return SUCCESS;

    case PCI_CONFIG_BAR_0:
    case PCI_CONFIG_BAR_1:
    case PCI_CONFIG_BAR_2:
    case PCI_CONFIG_BAR_3:
    case PCI_CONFIG_BAR_4:
    case PCI_CONFIG_BAR_5:
        WriteConfigD(bus, device, function, address, value);
        return SUCCESS;

    default:
        return ERROR_BAD_PARAMETER;
    }
}
uint64_t PCIDevice::DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) { return ERROR_NOT_IMPLEMENTED; }