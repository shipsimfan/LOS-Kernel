#pragma once

#include <stdint.h>

#define PCI_CONFIG_VENDOR_ID 0x00
#define PCI_CONFIG_DEVICE_ID 0x02
#define PCI_CONFIG_COMMAND 0x04
#define PCI_CONFIG_STATUS 0x06
#define PCI_CONFIG_REVISION 0x08
#define PCI_CONFIG_PROG_IF 0x09
#define PCI_CONFIG_SUB_CLASS 0x0A
#define PCI_CONFIG_CLASS 0x0B
#define PCI_CONFIG_CACHE_LINE_SIZE 0x0C
#define PCI_CONFIG_LATENCY_TIMER 0x0D
#define PCI_CONFIG_HEADER_TYPE 0x0E
#define PCI_CONFIG_BIST 0x0F

#define PCI_CONFIG_BAR_0 0x10
#define PCI_CONFIG_BAR_1 0x14
#define PCI_CONFIG_BAR_2 0x18
#define PCI_CONFIG_BAR_3 0x1C
#define PCI_CONFIG_BAR_4 0x20
#define PCI_CONFIG_BAR_5 0x24
#define PCI_CONFIG_INT_LINE 0x3C
#define PCI_CONFIG_INT_PIN 0x3D

#define PCI_CONFIG_SECONDARY_BUS_NUMBER 0x1A

namespace DeviceManager { namespace PCI {
    struct PCIDeviceInfo {
        uint8_t bus;
        uint8_t device;
        uint8_t function;
        uint16_t vendorID;
        uint16_t deviceID;
        uint8_t classCode;
        uint8_t subClass;
        uint8_t progIF;
        uint8_t headerType;
    };

    struct StandardPCIDeviceInfo : public PCIDeviceInfo {
        uint32_t baseAddr0;
        uint32_t baseAddr1;
        uint32_t baseAddr2;
        uint32_t baseAddr3;
        uint32_t baseAddr4;
        uint32_t baseAddr5;
        uint8_t interruptLine;
    };

    bool RegisterPCIDriver();

    uint8_t ReadConfigB(uint32_t reg);
    uint16_t ReadConfigW(uint32_t reg);
    uint32_t ReadConfigD(uint32_t reg);

    uint8_t ReadConfigB(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
    uint16_t ReadConfigW(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
    uint32_t ReadConfigD(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

    void WriteConfigB(uint32_t reg, uint8_t val);
    void WriteConfigW(uint32_t reg, uint16_t val);
    void WriteConfigD(uint32_t reg, uint32_t val);

    void WriteConfigB(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint8_t val);
    void WriteConfigW(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint16_t val);
    void WriteConfigD(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t val);
}} // namespace DeviceManager::PCI