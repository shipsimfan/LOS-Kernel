#pragma once

#include <device/device.h>

class PS2Keyboard;

class PS2Controller : public Device::Device {
    friend PS2Keyboard;

public:
    PS2Controller();

private:
    static void FirstPortIRQ(void* context);
    static void SecondPortIRQ(void* context);

    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

    uint64_t WriteAndWait(uint64_t port, uint8_t data);

    void IdentPort(uint64_t port);

    bool portExists[2];
    bool portIRQ[2];
    uint8_t portData[2];
};

class PS2Keyboard : public Device::Device {
public:
    PS2Keyboard(PS2Controller* controller, uint64_t port);

private:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

    PS2Controller* controller;
    uint64_t port;
};

void InitializePS2Driver();