#include <device/device.h>

#include <errno.h>
#include <process/control.h>
#include <string.h>

namespace Device {
    Device::Device(const char* name, Type type) : type(type), parent(nullptr), refCount(0) {
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
    }

    Device::~Device() {
        childrenMutex.Lock();

        while (refCount != 0)
            asm volatile("pause");

        for (Device* child = children.front(); child != nullptr; children.pop(), child = children.front())
            delete child;
    }

    uint64_t Device::Read(uint64_t address, uint64_t* value) { return ERROR_NOT_IMPLEMENTED; }

    int64_t Device::ReadStream(uint64_t address, void* buffer, int64_t count) {
        errno = ERROR_NOT_IMPLEMENTED;
        return -1;
    }

    uint64_t Device::Write(uint64_t address, uint64_t value) { return ERROR_NOT_IMPLEMENTED; }

    int64_t Device::WriteStream(uint64_t address, void* buffer, int64_t count) {
        errno = ERROR_NOT_IMPLEMENTED;
        return -1;
    }

    void Device::AddChild(Device* child) {
        childrenMutex.Lock();
        children.push(child);
        child->parent = this;
        childrenMutex.Unlock();
    }

    uint64_t Device::FindDevices(Type type, Queue<Device>& queue) {
        uint64_t count = 0;
        childrenMutex.Lock();
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

        childrenMutex.Unlock();

        return count;
    }

    const char* Device::GetName() { return name; }

    void Device::SetName(const char* newName) {
        delete name;
        name = new char[strlen(newName) + 1];
        strcpy(name, newName);
    }

    void Device::IncreamentRefCount() {
        if (refCount == ~0)
            return;

        refCount++;
    }

    void Device::DecreamentRefCount() {
        if (refCount == 0)
            return;

        refCount--;
    }
} // namespace Device