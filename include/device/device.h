#pragma once

#include <mutex.h>
#include <queue.h>
#include <stdint.h>

namespace Device {
    class Device {
    public:
        enum Type { UNDEFINED, CONSOLE };

        Device(const char* name, Type type);
        virtual ~Device();

        uint64_t Open();
        uint64_t Close();

        uint64_t Read(uint64_t address, uint64_t* value);
        int64_t ReadStream(uint64_t address, void* buffer, int64_t count);

        uint64_t Write(uint64_t address, uint64_t value);
        int64_t WriteStream(uint64_t address, void* buffer, int64_t count);

        void AddChild(Device* child);
        void FindDevices(Type type, Queue<Device>& queue);

        const char* GetName();

    protected:
        virtual uint64_t OnOpen() = 0;
        virtual uint64_t OnClose() = 0;

        virtual uint64_t DoRead(uint64_t address, uint64_t* value) = 0;
        virtual uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) = 0;

        virtual uint64_t DoWrite(uint64_t address, uint64_t value) = 0;
        virtual uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) = 0;

    private:
        const char* name;
        Type type;

        Device* parent;
        Queue<Device> children;

        Mutex mutex;
    };
} // namespace Device