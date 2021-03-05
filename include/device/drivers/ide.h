#pragma once

#include <device/device.h>

void InitializeIDEDriver();

class IDEDevice : public Device::Device {
public:
    IDEDevice();

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

private:
    struct {
        uint16_t IO;
        uint16_t control;
        uint16_t busMaster;
        uint8_t nIEN;
    } channels[2];
};

class ATAPIDevice : public Device::Device {
public:
    ATAPIDevice();

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;
};