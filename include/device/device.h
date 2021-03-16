#pragma once

#include <mutex.h>
#include <queue.h>
#include <stdint.h>

namespace Device {
    class Device {
    public:
        enum Type { UNDEFINED, CONSOLE, PCI_DEVICE, CONTROLLER, CD_DRIVE, KEYBOARD, HDD };

        Device(const char* name, Type type);
        virtual ~Device();

        virtual uint64_t Read(uint64_t address, uint64_t* value);
        virtual int64_t ReadStream(uint64_t address, void* buffer, int64_t count);

        virtual uint64_t Write(uint64_t address, uint64_t value);
        virtual int64_t WriteStream(uint64_t address, void* buffer, int64_t count);

        void AddChild(Device* child);
        uint64_t FindDevices(Type type, Queue<Device>& queue);
        void RemoveChild(Device* child);

        const char* GetName();

        Device* GetParent();

        void IncreamentRefCount();
        void DecreamentRefCount();

    protected:
        void SetName(const char* newName);

    private:
        char* name;
        Type type;

        uint64_t refCount;

        Device* parent;
        Queue<Device> children;
        Mutex childrenMutex;
    };
} // namespace Device