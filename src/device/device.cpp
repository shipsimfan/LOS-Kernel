#include <device/device.h>

#include <errno.h>
#include <process/control.h>

namespace Device {
    Device::Device(const char* name, Type type) : name(name), type(type), parent(nullptr) {}

    Device::~Device() {
        mutex.Lock();
        for (Device* child = children.front(); child != nullptr; children.pop(), child = children.front())
            delete child;
    }

    uint64_t Device::Open() {
        mutex.Lock();
        return OnOpen();
    }

    uint64_t Device::Close() {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

        uint64_t ret = OnClose();
        mutex.Unlock();
        return ret;
    }

    uint64_t Device::Read(uint64_t address, uint64_t* value) {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

        return DoRead(address, value);
    }

    int64_t Device::ReadStream(uint64_t address, void* buffer, int64_t count) {
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

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
        if (mutex.GetOwner() != currentProcess)
            return ERROR_NOT_OWNER;

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

    void Device::FindDevices(Type type, Queue<Device>& queue) {
        mutex.Lock();
        if (this->type == type)
            queue.push(this);

        for (Queue<Device>::Iterator iter(&children); iter.value != nullptr; iter.Next())
            iter.value->FindDevices(type, queue);
        mutex.Unlock();
    }

    const char* Device::GetName() { return name; }
} // namespace Device