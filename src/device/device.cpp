#include <device/device.h>

#include <errno.h>
#include <process/control.h>
#include <string.h>

namespace Device {
    Device::Device(const char* name, Type type) : type(type), parent(nullptr) {
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
    }

    Device::~Device() {
        mutex.Lock();
        for (Device* child = children.front(); child != nullptr; children.pop(), child = children.front())
            delete child;
    }

    int64_t Device::Open() {
        mutex.Lock();
        uint64_t descriptor = currentProcess->AddDevice(this);
        uint64_t status = OnOpen();
        if (status != SUCCESS) {
            errno = status;
            return -1;
        }

        return descriptor;
    }

    uint64_t Device::Close() {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

        uint64_t ret = OnClose();
        currentProcess->RemoveDevice(this);
        mutex.Unlock();
        return ret;
    }

    uint64_t Device::Read(uint64_t address, uint64_t* value) {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

        return DoRead(address, value);
    }

    int64_t Device::ReadStream(uint64_t address, void* buffer, int64_t count) {
        if (mutex.GetOwner() != currentProcess) {
            errno = ERROR_NOT_OWNER;
            return -1;
        }

        int64_t countRead;
        errno = DoReadStream(address, buffer, count, countRead);
        if (errno != SUCCESS)
            return -1;

        return countRead;
    }

    uint64_t Device::Write(uint64_t address, uint64_t value) {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

        return DoWrite(address, value);
    }

    int64_t Device::WriteStream(uint64_t address, void* buffer, int64_t count) {
        if (mutex.GetOwner() != currentProcess) {
            errno = ERROR_NOT_OWNER;
            return -1;
        }

        int64_t countWritten;
        errno = DoWriteStream(address, buffer, count, countWritten);
        if (errno != SUCCESS)
            return -1;

        return countWritten;
    }

    void Device::AddChild(Device* child) {
        mutex.Lock();
        children.push(child);
        child->parent = this;
        mutex.Unlock();
    }

    uint64_t Device::FindDevices(Type type, Queue<Device>& queue) {
        uint64_t count = 0;
        mutex.Lock();
        if (this->type == type) {
            queue.push(this);
            count++;
        }

        if (children.front() != nullptr) {
            Queue<Device>::Iterator iter(&children);
            do
                count += iter.value->FindDevices(type, queue);
            while (iter.Next());
        }

        mutex.Unlock();

        return count;
    }

    const char* Device::GetName() { return name; }

    void Device::SetName(const char* newName) {
        delete name;
        name = new char[strlen(newName) + 1];
        strcpy(name, newName);
    }
} // namespace Device