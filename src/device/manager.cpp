#include <device/manager.h>

#include <errno.h>

namespace Device {
    Queue<Device> rootDevices;

    uint64_t RegisterDevice(Device* parent, Device* newDevice) {
        if (newDevice == nullptr)
            return ERROR_BAD_PARAMETER;

        if (parent == nullptr) {
            rootDevices.push(newDevice);
            return SUCCESS;
        }

        parent->AddChild(newDevice);

        return SUCCESS;
    }

    uint64_t GetDevices(Device::Type type, Queue<Device>& queue) {
        uint64_t count = 0;
        if (rootDevices.front() != nullptr) {
            Queue<Device>::Iterator iter(&rootDevices);
            do
                count += iter.value->FindDevices(type, queue);
            while (iter.Next());
        }

        return count;
    }
} // namespace Device