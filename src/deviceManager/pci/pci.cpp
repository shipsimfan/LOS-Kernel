#include <dev/pci.h>

#include <dev.h>
#include <logger.h>

namespace DeviceManager { namespace PCI {
    DeviceDriver PCIDriver;

    // bool verifyDevice(Device* device)
    // Returns true if this driver can initialize the device
    bool PCIVerifyDevice(Device*) { return false; }

    // bool registerDevice(Device* device)
    // Registers a device to this driver
    // Returns false on error
    void PCIRegisterDevice(Device*) {}

    uint32_t PCIToRegister(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
        uint32_t lbus = (uint32_t)bus;
        uint32_t ldevice = (uint32_t)device;
        uint32_t lfunc = (uint32_t)func;

        return (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    }

    void CheckPCIBus(uint8_t bus);

    void CheckPCIFunction(uint8_t bus, uint8_t device, uint8_t function) {
        uint8_t baseClass = ReadConfigB(bus, device, function, PCI_CONFIG_CLASS);
        uint8_t subClass = ReadConfigB(bus, device, function, PCI_CONFIG_SUB_CLASS);

        if (baseClass == 0x06 && subClass == 0x04) {
            uint8_t secondaryBus = ReadConfigB(bus, device, function, PCI_CONFIG_SECONDARY_BUS_NUMBER);
            CheckPCIBus(secondaryBus);
        }

        // Register PCI Device
        debugLogger.Log("New PCI Device %#x-%#x", baseClass, subClass);
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

    void CheckAllPCIBuses() {
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

    bool RegisterPCIDriver() {
        infoLogger.Log("Initializing PCI . . . ");

        // Register PCI Driver with device manager
        PCIDriver.signature = DEVICE_DRIVER_SIGNATURE_PCI;
        PCIDriver.name = "LOS PCI Driver";
        PCIDriver.parent = GetDeviceDriver(DEVICE_DRIVER_SIGNATURE_ACPI);

        PCIDriver.VerifyDevice = PCIVerifyDevice;
        PCIDriver.RegisterDevice = PCIRegisterDevice;
        RegisterDeviceDriver(&PCIDriver);

        // Ennumerate the PCI and register all the devices
        CheckAllPCIBuses();

        return true;
    }

    uint8_t ReadConfigB(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return (uint8_t)((ind(0xCFC) >> ((reg & 3) * 8)) & 0xFF);
    }

    uint16_t ReadConfigW(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return (uint16_t)((ind(0xCFC) >> ((reg & 2) * 8)) & 0xFFFF);
    }

    uint32_t ReadConfigD(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return ind(0xCFC);
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

    void WriteConfigB(uint32_t reg, uint8_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val << ((reg & 3) * 8));
    }

    void WriteConfigW(uint32_t reg, uint16_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val << ((reg & 2) * 8));
    }

    void WriteConfigD(uint32_t reg, uint32_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val);
    }

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

}} // namespace DeviceManager::PCI