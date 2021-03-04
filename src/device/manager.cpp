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

    Queue<Device> GetDevices(Device::Type type) {
        Queue<Device> ret;
        if (rootDevices.front() != nullptr)
            for (Queue<Device>::Iterator iter(&rootDevices); iter.value != nullptr; iter.Next())
                iter.value->FindDevices(type, ret);

        return ret;
    }
} // namespace Device