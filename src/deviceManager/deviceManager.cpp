#include <dev.h>
#include <dev/acpi.h>
#include <dev/pci.h>
#include <interrupt.h>
#include <logger.h>
#include <stdlib.h>

namespace DeviceManager {
    DeviceDriver* driverRoot = nullptr;

    bool Init(void* rdsp) {
        infoLogger.Log("Initializing device manager . . .");
        if (!ACPI::RegisterACPIDriver(rdsp))
            return false;

        if (!PCI::RegisterPCIDriver())
            return false;

        infoLogger.Log("Device manager initialized!");
        return true;
    }

    bool RegisterDeviceDriver(DeviceDriver* driver) {
        if (driver->signature == DEVICE_DRIVER_SIGNATURE_ACPI) {
            if (driverRoot != nullptr) {
                errorLogger.Log("Attempting to register ACPI driver when one is already installed");
                return false;
            }

            driverRoot = driver;

            return true;
        } else if (driverRoot == nullptr) {
            errorLogger.Log("Attempting to register device driver before root driver has been setup!");
            return false;
        }

        if (driver->parent == nullptr) {
            errorLogger.Log("Attempting to register a driver with no parent");
            return false;
        }

        DeviceDriver* parent = driver->parent;
        if (parent->RegisterChildDriver != nullptr) {
            DeviceDriverNode* newNode = (DeviceDriverNode*)malloc(sizeof(DeviceDriverNode));
            newNode->driver = driver;

            newNode->next = parent->childHead;
            parent->childHead = newNode;

            parent->RegisterChildDriver(driver);
            return true;
        } else {
            errorLogger.Log("Parent device driver does not support having child drivers!");
            return false;
        }
    }

    Device* RegisterDevice(Device* device, DeviceDriver* newDriver) {
        Device* ret = device->next;

        if (newDriver->VerifyDevice(device)) {
            if (device->driver != nullptr)
                if (device->driver->deviceHead == device)
                    device->driver->deviceHead = device->next;

            if (device->prev != nullptr)
                device->prev->next = device->next;
            if (device->next != nullptr)
                device->next->prev = device->prev;

            if (newDriver->deviceHead != nullptr)
                newDriver->deviceHead->prev = device;

            device->next = newDriver->deviceHead;
            newDriver->deviceHead = device;
            device->prev = nullptr;

            device->driver = newDriver;
            newDriver->RegisterDevice(device);
        }

        return ret;
    }

    void outb(uint16_t port, uint8_t val) { asm volatile("outb %0, %1" : : "a"(val), "Nd"(port)); }

    void outw(uint16_t port, uint16_t val) { asm volatile("outw %0, %1" : : "a"(val), "Nd"(port)); }

    void outd(uint16_t port, uint32_t val) { asm volatile("outl %0, %1" : : "a"(val), "Nd"(port)); }

    uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    uint16_t inw(uint16_t port) {
        uint16_t ret;
        asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    uint32_t ind(uint16_t port) {
        uint32_t ret;
        asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }
} // namespace DeviceManager