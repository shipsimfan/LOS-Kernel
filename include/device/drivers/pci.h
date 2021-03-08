#pragma once

#include <device/device.h>

class PCIDevice : public Device::Device {
public:
    PCIDevice(uint8_t bus, uint8_t device, uint8_t function);

    uint64_t Read(uint64_t address, uint64_t* value) override;
    uint64_t Write(uint64_t address, uint64_t value) override;

private:
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

void InitializePCIDriver();