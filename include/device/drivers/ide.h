#pragma once

#include <device/device.h>
#include <device/drivers/pci.h>

class ATAPIDevice;
class ATADevice;

class IDEDevice : public Device::Device {
    friend ATAPIDevice;
    friend ATADevice;

public:
    IDEDevice(PCIDevice* pciDevice);

    uint64_t Read(uint64_t address, uint64_t* value) override;
    int64_t ReadStream(uint64_t address, void* buffer, int64_t count) override;

    uint64_t Write(uint64_t address, uint64_t value) override;

private:
    void WaitIRQ();
    static void IRQHandler(void* context);

    struct {
        uint16_t IO;
        uint16_t control;
        uint16_t busMaster;
        uint8_t nIEN;
        Device* drives[2];
        Mutex mutex;
    } channels[2];

    bool irq;
};

class ATAPIDevice : public Device::Device {
public:
    ATAPIDevice(IDEDevice* ide, uint8_t channel, uint8_t drive);

    uint64_t Read(uint64_t address, uint64_t* value) override;
    int64_t ReadStream(uint64_t address, void* buffer, int64_t count) override;

    uint64_t Write(uint64_t address, uint64_t value) override;

private:
    bool Polling(bool advancedCheck);

    IDEDevice* ide;
    uint8_t channel;
    uint8_t drive;

    uint16_t sign;
    uint16_t capabilities;
    uint32_t commandSets;

    int64_t size;
};

class ATADevice : public Device::Device {
public:
    ATADevice(IDEDevice* ide, uint8_t channel, uint8_t drive);

    uint64_t Read(uint64_t address, uint64_t* value) override;
    int64_t ReadStream(uint64_t address, void* buffer, int64_t count) override;

    uint64_t Write(uint64_t address, uint64_t value) override;

private:
    bool Polling(bool advancedCheck);

    IDEDevice* ide;
    uint8_t channel;
    uint8_t drive;

    uint16_t sign;
    uint16_t capabilities;
    uint32_t commandSets;

    int64_t size;
};

void InitializeIDEDriver();