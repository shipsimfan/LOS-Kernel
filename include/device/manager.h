#pragma once

#include <device/device.h>

namespace Device {
    uint64_t RegisterDevice(Device* parent, Device* newDevice);

    Queue<Device> GetDevices(Device::Type type);
}