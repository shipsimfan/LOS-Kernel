#pragma once

#include <device/device.h>
#include <device/drivers/pci.h>

void InitializeIDEDriver();

class ATAPIDevice;

class IDEDevice : public Device::Device {
    friend ATAPIDevice;

public:
    IDEDevice(PCIDevice* pciDevice);

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

private:
    void WaitIRQ();
    static void IRQHandler(void* context);

    struct {
        uint16_t IO;
        uint16_t control;
        uint16_t busMaster;
        uint8_t nIEN;
        Device* drives[2];
    } channels[2];

    bool irq;
};

class ATAPIDevice : public Device::Device {
public:
    ATAPIDevice(IDEDevice* ide, uint8_t channel, uint8_t drive);

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

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
