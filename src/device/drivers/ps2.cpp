#include <device/drivers/ps2.h>

#include <console.h>
#include <device/acpi/acpi.h>
#include <device/manager.h>
#include <device/util.h>
#include <errno.h>
#include <interrupt/irq.h>
#include <time.h>

#include "ps2.h"

PS2Controller::PS2Controller() : Device("PS/2 Controller", Type::CONTROLLER), portExists{false, false}, portIRQ{true, true} {
    Interrupt::InstallIRQHandler(1, FirstPortIRQ, this);
    Interrupt::InstallIRQHandler(12, SecondPortIRQ, this);

    // Disable devices for initialization
    WriteCommand(PS2_CMD_DISABLE_FIRST_PORT);
    WriteCommand(PS2_CMD_DISABLE_SECOND_PORT);

    // Flush data buffer
    inb(PS2_REG_DATA);

    // Set the configuration byte
    WriteCommand(PS2_CMD_READ_CONFIG_BYTE);
    ReadData(uint8_t config);

    config &= 0b10111100;
    bool hasTwoPorts = config & 5;

    WriteCommand(PS2_CMD_WRITE_CONFIG_BYTE);
    WriteData(config);

    // Perform self test
    WriteCommand(PS2_CMD_TEST);
    ReadData(uint8_t response);
    if (response != 0x55) {
        Console::Println("[ PS2 ] Error during self test!");
        return;
    }

    // Rewrite configuration byte
    WriteCommand(PS2_CMD_WRITE_CONFIG_BYTE);
    WriteData(config);

    // Test for two channels
    if (hasTwoPorts) {
        WriteCommand(PS2_CMD_ENABLE_SECOND_PORT);

        WriteCommand(PS2_CMD_READ_CONFIG_BYTE);
        ReadData(config);

        if (config & 5)
            hasTwoPorts = false;
        else {
            WriteCommand(PS2_CMD_DISABLE_SECOND_PORT);
        }
    }

    // Test interfaces
    WriteCommand(PS2_CMD_TEST_FIRST_PORT);
    ReadData(uint8_t status);
    portExists[0] = !status;

    if (hasTwoPorts) {
        WriteCommand(PS2_CMD_TEST_SECOND_PORT);
        ReadData(status);
        portExists[1] = !status;
    }

    // Enable devices
    uint8_t intPins = 0;
    if (portExists[0]) {
        WriteCommand(PS2_CMD_ENABLE_FIRST_PORT);
        intPins |= 1;
    }

    if (portExists[1]) {
        WriteCommand(PS2_CMD_ENABLE_SECOND_PORT);
        intPins |= 2;
    }

    WriteCommand(PS2_CMD_READ_CONFIG_BYTE);
    ReadData(config);
    config |= intPins;
    WriteCommand(PS2_CMD_WRITE_CONFIG_BYTE);
    WriteData(config);

    if (portExists[0])
        IdentPort(0);

    if (portExists[1])
        IdentPort(1);
}

void PS2Controller::FirstPortIRQ(void* context) {
    PS2Controller* controller = (PS2Controller*)context;

    if (!controller->portIRQ[0])
        controller->portData[0] = inb(PS2_REG_DATA);
    else
        inb(PS2_REG_DATA);

    controller->portIRQ[0] = true;
}

void PS2Controller::SecondPortIRQ(void* context) {
    PS2Controller* controller = (PS2Controller*)context;

    if (!controller->portIRQ[1])
        controller->portData[1] = inb(PS2_REG_DATA);
    else
        inb(PS2_REG_DATA);

    controller->portIRQ[1] = true;
}

uint64_t PS2Controller::Write(uint64_t address, uint64_t value) {
    if (address > 1)
        return ERROR_BAD_PARAMETER;

    if (!portExists[address])
        return ERROR_OUT_OF_RANGE;

    if (address == 1) {
        WriteCommand(PS2_CMD_SELECT_SECOND_INPUT);
    }

    uint64_t start = GetCurrentTime();
    while (inb(PS2_REG_STATUS) & 2) {
        if (GetCurrentTime() > start + PS2_TIMEOUT)
            return ERROR_TIMEOUT;
    }

    outb(PS2_REG_DATA, value);

    return SUCCESS;
}

void PS2Controller::IdentPort(uint64_t port) {
    // Reset device
    if (WriteAndWait(port, PS2_DEV_CMD_RESET) != SUCCESS) {
        portExists[port] = false;
        return;
    }

    if (portData[port] != PS2_DEV_ACK) {
        portExists[port] = false;
        return;
    }

    // Identify device
    if (WriteAndWait(port, PS2_DEV_CMD_DISABLE_SCAN) != SUCCESS) {
        portExists[port] = false;
        return;
    }

    if (portData[port] != PS2_DEV_ACK) {
        portExists[port] = false;
        return;
    }

    uint8_t ident[2];
    int len = -1;
    uint64_t start;
    if (WriteAndWait(port, PS2_DEV_CMD_IDENTIFY) != SUCCESS) {
        portExists[port] = false;
        return;
    }

    if (portData[port] != PS2_DEV_ACK) {
        portExists[port] = false;
        return;
    }

    portIRQ[port] = false;
    start = GetCurrentTime();
    while (!portIRQ[port]) {
        if (GetCurrentTime() > start + PS2_TIMEOUT) {
            len = 0;
            break;
        }
    }

    if (len == -1) {
        ident[0] = portData[port];
        portIRQ[port] = false;
        start = GetCurrentTime();

        while (!portIRQ[port]) {
            if (GetCurrentTime() > start + PS2_TIMEOUT) {
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
        // Create PS/2 keyboard device
        PS2Keyboard* newKeyboard = new PS2Keyboard(this, port);
        RegisterDevice(this, newKeyboard);
    } else {
        Console::Print("[ PS2 ] Unknown device on port %i (%#X", port, ident[0]);
        if (len == 2)
            Console::Print(",%#X", ident[1]);
        Console::Println(")");
        portExists[port] = false;
    }
}

uint64_t PS2Controller::WriteAndWait(uint64_t port, uint8_t data) {
    if (port > 1)
        return ERROR_BAD_PARAMETER;

    portIRQ[port] = false;
    uint64_t status = Write(port, data);
    if (status != SUCCESS)
        return status;

    uint64_t start = GetCurrentTime();
    while (!portIRQ[port])
        if (GetCurrentTime() > start + PS2_TIMEOUT)
            return ERROR_TIMEOUT;

    return SUCCESS;
}

PS2Keyboard::PS2Keyboard(PS2Controller* controller, uint64_t port) : Device("PS/2 Keyboard", Type::KEYBOARD), controller(controller), port(port) {
    // Set scancode set to 2
    if (controller->WriteAndWait(port, 0xF0) != SUCCESS)
        return;

    outb(PS2_REG_DATA, 2);

    // Enable scanning
    if (controller->WriteAndWait(port, PS2_DEV_CMD_ENABLE_SCAN) != SUCCESS)
        return;
}

int64_t PS2Keyboard::ReadStream(uint64_t address, void* buffer, int64_t count) {
    mutex.Lock();
    uint8_t* buf = (uint8_t*)buffer;
    int64_t countRead;
    for (countRead = 0; countRead < count; countRead++) {
        controller->portIRQ[port] = false;
        while (!controller->portIRQ[port])
            ;

        // Enter key
        if (controller->portData[port] == 0x5A) {
            buf[countRead] = 0;
            mutex.Unlock();
            return countRead;
        }

        if (controller->portData[port] > 0x55) {
            countRead--;
            continue;
        }

        if (scancodeToChar[controller->portData[port]] == 0) {
            countRead--;
            continue;
        }

        Console::Print("%c", scancodeToChar[controller->portData[port]]);
        buf[countRead] = scancodeToChar[controller->portData[port]];
    }

    buf[count - 1] = 0;
    mutex.Unlock();

    return countRead;
}

void InitializePS2Driver() {
    // Determine if PS/2 controller exists
    ACPI::FADT* fadt = (ACPI::FADT*)ACPI::GetTable(FADT_SIGNATURE);
    if (fadt == nullptr) {
        Console::Println("[ PS2 ] Failed to get FADT!");
        return;
    }

    if (!(fadt->bootArchitectureFlags & 2)) {
        Console::Println("[ PS2 ] No PS/2 Controller installed!");
        return;
    }

    PS2Controller* controller = new PS2Controller;
    Device::RegisterDevice(nullptr, controller);
}