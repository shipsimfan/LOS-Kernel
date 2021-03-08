#pragma once

#include <device/device.h>

namespace Device {
    uint64_t RegisterDevice(Device* parent, Device* newDevice);
    void UnregisterDevice(Device* device);

    uint64_t GetDevices(Device::Type type, Queue<Device>& queue);

    int Open(Device* device);
    void Close(int dd);

    uint64_t Read(int dd, uint64_t address, uint64_t* value);
    uint64_t Write(int dd, uint64_t address, uint64_t value);

    int64_t ReadStream(int dd, uint64_t address, void* buffer, int64_t count);
    int64_t WriteStream(int dd, uint64_t address, void* buffer, int64_t count);
}