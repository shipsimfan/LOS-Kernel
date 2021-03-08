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

    void UnregisterDevice(Device* device) {
        if (device->GetParent() == nullptr) {
            if (rootDevices.front() == nullptr)
                return;

            Queue<Device>::Iterator iter(&rootDevices);
            do {
                if (iter.value == device) {
                    iter.Remove();
                    break;
                }
            } while (iter.Next());
        } else
            device->GetParent()->RemoveChild(device);
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

    int Open(Device* device) {
        currentProcess->AddDevice(device);
        device->IncreamentRefCount();
    }

    void Close(int dd) {
        Device* device = currentProcess->GetDevice(dd);
        if (device == nullptr)
            return;

        device->DecreamentRefCount();
        currentProcess->RemoveDevice(dd);
    }

    uint64_t Read(int dd, uint64_t address, uint64_t* value) {
        Device* device = currentProcess->GetDevice(dd);
        if (device == nullptr)
            return ERROR_BAD_PARAMETER;

        return device->Read(address, value);
    }

    uint64_t Write(int dd, uint64_t address, uint64_t value) {
        Device* device = currentProcess->GetDevice(dd);
        if (device == nullptr)
            return ERROR_BAD_PARAMETER;

        return device->Write(address, value);
    }

    int64_t ReadStream(int dd, uint64_t address, void* buffer, int64_t count) {
        Device* device = currentProcess->GetDevice(dd);
        if (device == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            return -1;
        }

        return device->ReadStream(address, buffer, count);
    }

    int64_t WriteStream(int dd, uint64_t address, void* buffer, int64_t count) {
        Device* device = currentProcess->GetDevice(dd);
        if (device == nullptr) {
            errno = ERROR_BAD_PARAMETER;
            return -1;
        }

        return device->WriteStream(address, buffer, count);
    }
} // namespace Device