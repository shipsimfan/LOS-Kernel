#pragma once

#include <proc.h>
#include <types.h>

namespace DeviceManager {
#define DEVICE_DRIVER_SIGNATURE_UNKNOWN 0
#define DEVICE_DRIVER_SIGNATURE_ACPI 1
#define DEVICE_DRIVER_SIGNATURE_PCI 2
#define DEVICE_DRIVER_SIGNATURE_IDE 3
#define DEVICE_DRIVER_SIGNATURE_ATA 4
#define DEVICE_DRIVER_SIGNATURE_KEYBOARD 5

    struct DeviceDriver;

    struct Device {
        char* name;
        DeviceDriver* driver;
        uint64_t driverDeviceType; // Optional, only used by drivers
        void* driverInfo;
        Device* next;
        Device* prev;

        Mutex deviceMutex;
    };

    struct DeviceDriverNode {
        DeviceDriver* driver;
        DeviceDriverNode* next;
        DeviceDriverNode* prev;
    };

    struct DeviceDriver {
        uint64_t signature;
        const char* name;
        DeviceDriver* parent;
        DeviceDriverNode* childHead = nullptr;
        Device* deviceHead = nullptr;

        void (*RegisterChildDriver)(DeviceDriver*) = nullptr;

        // bool verifyDevice(Device* device)
        // Returns true if this driver can initialize the device
        bool (*VerifyDevice)(Device*);

        // bool registerDevice(Device* device)
        // Registers a device to this driver
        // Returns false on error
        void (*RegisterDevice)(Device*);

        // uint64_t Read(Device* device, uint64_t address)
        // Returns the number read
        uint64_t (*Read)(Device*, uint64_t) = nullptr;

        // uint64_t ReadStream(Device* device, uint64_t address, void* buffer, size_t bufferSize);
        // Returns number of bytes read into buffer
        uint64_t (*ReadStream)(Device*, uint64_t, void*, size_t) = nullptr;

        // uint64_t Write(Device* device, uint64_t address, uint64_t value);
        // Returns true if the write was successful
        bool (*Write)(Device*, uint64_t, uint64_t) = nullptr;

        // uint64_t WriteStream(Device* device, uint64_t address, void* buffer, size_t bufferSize);
        // Returns number of bytes written
        uint64_t (*WriteStream)(Device*, uint64_t, void*, size_t) = nullptr;
    };

    bool Init(void* rdsp);

    bool RegisterDeviceDriver(DeviceDriver* driver);

    Device* RegisterDevice(Device* device, DeviceDriver* newDriver);

    DeviceDriver* GetDeviceDriver(uint64_t signature);
    Device** GetDevices(uint64_t signature);

    void outb(uint16_t port, uint8_t val);
    void outw(uint16_t port, uint16_t val);
    void outd(uint16_t port, uint32_t val);

    uint8_t inb(uint16_t port);
    uint16_t inw(uint16_t port);
    uint32_t ind(uint16_t port);

    void IOWait();

} // namespace DeviceManager

extern "C" void Shutdown();