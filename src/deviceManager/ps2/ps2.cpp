#include <dev/ps2.h>

#include <console.h>
#include <dev.h>
#include <dev/scancode.h>
#include <interrupt.h>
#include <kernel/time.h>
#include <logger.h>

#define PS2_REG_DATA 0x60
#define PS2_REG_COMMAND 0x64
#define PS2_REG_STATUS 0x64

#define PS2_CMD_READ_CONFIG_BYTE 0x20
#define PS2_CMD_WRITE_CONFIG_BYTE 0x60
#define PS2_CMD_DISABLE_SECOND_PORT 0xA7
#define PS2_CMD_ENABLE_SECOND_PORT 0xA8
#define PS2_CMD_TEST_SECOND_PORT 0xA9
#define PS2_CMD_TEST 0xAA
#define PS2_CMD_TEST_FIRST_PORT 0xAB
#define PS2_CMD_DISABLE_FIRST_PORT 0xAD
#define PS2_CMD_ENABLE_FIRST_PORT 0xAE
#define PS2_CMD_SELECT_SECOND_INPUT 0xD4

#define PS2_DEV_CMD_IDENTIFY 0xF2
#define PS2_DEV_CMD_ENABLE_SCAN 0xF4
#define PS2_DEV_CMD_DISABLE_SCAN 0xF5
#define PS2_DEV_CMD_RESET 0xFF

#define PS2_DEV_ACK 0xFA

#define PS2_TIMEOUT 10

namespace DeviceManager { namespace PS2 {
    DeviceDriver ps2Driver;

    bool hasTwoPorts;

    Device ports[2];
    bool portExists[2];
    bool portIRQ[2];
    uint8_t portData[2];

    uint8_t keyboardPort = 0xFF;

    void FirstPortIRQHandler() {
        if (!portIRQ[0])
            portData[0] = inb(PS2_REG_DATA);
        else
            inb(PS2_REG_DATA);

        portIRQ[0] = true;
    }

    void SecondPortIRQHandler() {
        if (!portIRQ[1])
            portData[1] = inb(PS2_REG_DATA);
        else
            inb(PS2_REG_DATA);

        portIRQ[1] = true;
    }

    bool WriteFirstPort(uint8_t data) {
        if (!portExists[0])
            return false;

        uint64_t start = getCurrentTimeMillis();
        while (inb(PS2_REG_STATUS) & 1) {
            if (getCurrentTimeMillis() > start + PS2_TIMEOUT) {
                errorLogger.Log("Timed out on PS/2 write");
                return false;
            }
        };

        outb(PS2_REG_DATA, data);

        return true;
    }

    bool WriteSecondPort(uint8_t data) {
        if (!portExists[1])
            return false;

        outb(PS2_REG_COMMAND, PS2_CMD_SELECT_SECOND_INPUT);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        uint64_t start = getCurrentTimeMillis();
        while (inb(PS2_REG_STATUS) & 1) {
            if (getCurrentTimeMillis() > start + PS2_TIMEOUT) {
                errorLogger.Log("Timed out on PS/2 write");
                return false;
            }
        };

        outb(PS2_REG_DATA, data);

        return true;
    }

    bool WritePort(uint8_t port, uint8_t data) {
        if (port > 1)
            return false;

        if (port == 0)
            return WriteFirstPort(data);
        else
            return WriteSecondPort(data);
    }

    bool WriteAndWait(uint8_t port, uint8_t data) {
        if (port > 1)
            return false;

        portIRQ[port] = false;
        WritePort(port, data);
        uint64_t start = getCurrentTimeMillis();
        while (!portIRQ[port])
            if (getCurrentTimeMillis() > start + PS2_TIMEOUT)
                return false;

        return true;
    }

    extern "C" size_t ReadKey(char* buffer, size_t size) {
        if (keyboardPort == 0xFF)
            return 0;

        // Enable scanning
        WriteAndWait(keyboardPort, PS2_DEV_CMD_ENABLE_SCAN);

        // Read from keyboard
        for (size_t i = 0; i < size; i++) {
            portIRQ[keyboardPort] = false;
            while (!portIRQ[keyboardPort])
                ;

            if (portData[keyboardPort] == 0x5A) {
                buffer[i] = 0;
                return i;
            }

            if (portData[keyboardPort] > 0x55) {
                i--;
                continue;
            }

            if (scancodeToChar[portData[keyboardPort]] == 0) {
                i--;
                continue;
            }

            Console::DisplayCharacter(scancodeToChar[portData[keyboardPort]]);
            buffer[i] = scancodeToChar[portData[keyboardPort]];
        }

        // Disable scanning
        WriteAndWait(keyboardPort, PS2_DEV_CMD_DISABLE_SCAN);

        return size;
    }

    bool VerifyPS2Device(Device* device) {
        uint8_t port = ((uint64_t)device->driverInfo) & 1;
        return portExists[port];
    }

    void RegisterPS2Device(Device* device) {
        uint8_t port = ((uint64_t)device->driverInfo) & 1;

        // Reset device
        if (!WriteAndWait(port, PS2_DEV_CMD_RESET)) {
            errorLogger.Log("Reset timeout (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        if (portData[port] != PS2_DEV_ACK) {
            errorLogger.Log("PS/2 port reset failure (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        // Identify device
        if (!WriteAndWait(port, PS2_DEV_CMD_DISABLE_SCAN)) {
            errorLogger.Log("Disable scan timeout (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        if (portData[port] != PS2_DEV_ACK) {
            errorLogger.Log("Unable to disable scanning (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        uint8_t ident[2];
        int len = -1;
        uint64_t start;
        if (!WriteAndWait(port, PS2_DEV_CMD_IDENTIFY)) {
            errorLogger.Log("Identify timeout (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        if (portData[port] != PS2_DEV_ACK) {
            errorLogger.Log("Unable to identify device (%#x)", portData[port]);
            portExists[port] = false;
            return;
        }

        portIRQ[port] = false;
        start = getCurrentTimeMillis();
        while (!portIRQ[port]) {
            if (getCurrentTimeMillis() > start + PS2_TIMEOUT) {
                len = 0;
                break;
            }
        }

        if (len == -1) {
            ident[0] = portData[port];
            portIRQ[port] = false;
            start = getCurrentTimeMillis();
            while (!portIRQ[port]) {
                if (getCurrentTimeMillis() > start + PS2_TIMEOUT) {
                    len = 1;
                    break;
                }
            }

            if (len == -1) {
                ident[1] = portData[port];
                len = 2;
            }
        }

        if (len == 0) {
            ports[port].driverDeviceType = 0;
            keyboardPort = port;

            WriteAndWait(port, 0xF0);
            outb(PS2_REG_DATA, 2);
        } else {
            errorLogger.Log("Unknown PS/2 device type (%i)(%#x %#x)", len, ident[0], ident[1]);
            portExists[port] = false;
        }
    }

    uint64_t ReadStream(Device* device, uint64_t address, void* buffer, size_t bufferSize) { return 0; }

    void RegisterPS2Driver() {
        ps2Driver.signature = 0xFFFFFFFFFFFFFFFF;
        ps2Driver.name = "LOS PS/2 Driver";
        ps2Driver.parent = GetDeviceDriver(DEVICE_DRIVER_SIGNATURE_ACPI);
        ps2Driver.RegisterDevice = RegisterPS2Device;
        ps2Driver.ReadStream = ReadStream;
        ps2Driver.VerifyDevice = VerifyPS2Device;
        RegisterDeviceDriver(&ps2Driver);

        InterruptHandler::SetIRQ(1, FirstPortIRQHandler);
        InterruptHandler::SetIRQ(12, SecondPortIRQHandler);

        // Initialize the PS/2 Controller
        portExists[0] = false;
        portExists[1] = false;

        // Disable devices
        outb(PS2_REG_COMMAND, PS2_CMD_DISABLE_FIRST_PORT);
        while (inb(PS2_REG_STATUS) & 1)
            ;
        outb(PS2_REG_COMMAND, PS2_CMD_DISABLE_SECOND_PORT);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        // Flush output buffer
        inb(PS2_REG_DATA);

        // Set the configuration byte
        outb(PS2_REG_COMMAND, PS2_CMD_READ_CONFIG_BYTE);
        while (inb(PS2_REG_STATUS) & 1)
            ;
        uint8_t config = inb(PS2_REG_DATA);

        config &= 0b10111100;

        if (config & 5)
            hasTwoPorts = true;
        else
            hasTwoPorts = false;

        outb(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_BYTE);
        while (inb(PS2_REG_STATUS) & 1)
            ;
        outb(PS2_REG_DATA, config);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        // Perform self test
        outb(PS2_REG_COMMAND, PS2_CMD_TEST);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        if (inb(PS2_REG_DATA) != 0x55) {
            errorLogger.Log("Error while testing PS/2 device!");
            return;
        }

        // Rewrite the configuration byte
        outb(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_BYTE);
        while (inb(PS2_REG_STATUS) & 1)
            ;
        outb(PS2_REG_DATA, config);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        // Determine two channel
        if (hasTwoPorts) {
            outb(PS2_REG_COMMAND, PS2_CMD_ENABLE_SECOND_PORT);
            while (inb(PS2_REG_STATUS) & 1)
                ;

            outb(PS2_REG_COMMAND, PS2_CMD_READ_CONFIG_BYTE);
            while (inb(PS2_REG_STATUS) & 1)
                ;
            config = inb(PS2_REG_DATA);

            if (config & 5)
                hasTwoPorts = false;
            else {
                outb(PS2_REG_COMMAND, PS2_CMD_DISABLE_SECOND_PORT);
                while (inb(PS2_REG_STATUS) & 1)
                    ;
            }
        }

        // Test interfaces
        outb(PS2_REG_COMMAND, PS2_CMD_TEST_FIRST_PORT);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        uint8_t status = inb(PS2_REG_DATA);
        if (status)
            errorLogger.Log("First port returned test error: %#x", status);
        else
            portExists[0] = true;

        if (hasTwoPorts) {
            outb(PS2_REG_COMMAND, PS2_CMD_TEST_SECOND_PORT);
            while (inb(PS2_REG_STATUS) & 1)
                ;

            status = inb(PS2_REG_DATA);
            if (status)
                errorLogger.Log("Second port returned test error: %#x", status);
            else
                portExists[1] = true;
        }

        // Enable devices
        uint8_t intPins = 0;
        if (portExists[0]) {
            outb(PS2_REG_COMMAND, PS2_CMD_ENABLE_FIRST_PORT);
            while (inb(PS2_REG_STATUS) & 1)
                ;
            intPins |= 1;
        }

        if (portExists[1]) {
            outb(PS2_REG_COMMAND, PS2_CMD_ENABLE_SECOND_PORT);
            while (inb(PS2_REG_STATUS) & 1)
                ;
            intPins |= 2;
        }

        outb(PS2_REG_COMMAND, PS2_CMD_READ_CONFIG_BYTE);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        config = inb(PS2_REG_DATA);
        config |= intPins;

        outb(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_BYTE);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        outb(PS2_REG_DATA, config);
        while (inb(PS2_REG_STATUS) & 1)
            ;

        // Register devices
        ports[0].driverInfo = (void*)0;
        ports[1].driverInfo = (void*)1;

        if (portExists[0])
            RegisterDevice(&ports[0], &ps2Driver);

        if (portExists[1])
            RegisterDevice(&ports[1], &ps2Driver);
    }
}} // namespace DeviceManager::PS2