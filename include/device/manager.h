#pragma once

#include <device/device.h>

namespace Device {
    uint64_t RegisterDevice(Device* parent, Device* newDevice);

    uint64_t GetDevices(Device::Type type, Queue<Device>& queue);
}