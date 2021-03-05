#pragma once

#include <device/device.h>

void InitializePCIDriver();

class PCIDevice : public Device::Device {
public:
    PCIDevice(uint8_t bus, uint8_t device, uint8_t function);

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

private:
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};