#pragma once

#include <device/device.h>

class PS2Keyboard;

class PS2Controller : public Device::Device {
    friend PS2Keyboard;

public:
    PS2Controller();

    uint64_t Write(uint64_t address, uint64_t value) override;

private:
    static void FirstPortIRQ(void* context);
    static void SecondPortIRQ(void* context);

    uint64_t WriteAndWait(uint64_t port, uint8_t data);

    void IdentPort(uint64_t port);

    bool portExists[2];
    bool portIRQ[2];
    uint8_t portData[2];
};

class PS2Keyboard : public Device::Device {
public:
    PS2Keyboard(PS2Controller* controller, uint64_t port);

    int64_t ReadStream(uint64_t address, void* buffer, int64_t count) override;

private:
    PS2Controller* controller;
    uint64_t port;
    Mutex mutex;
};

void InitializePS2Driver();