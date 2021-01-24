#pragma once

#include <types.h>

#define PCI_CONFIG_DEVICE_ID 0x00
#define PCI_CONFIG_VENDOR_ID 0x02
#define PCI_CONFIG_STATUS 0x04
#define PCI_CONFIG_COMMAND 0x06
#define PCI_CONFIG_CLASS 0x08
#define PCI_CONFIG_SUB_CLASS 0x09
#define PCI_CONFIG_PROG_IF 0x0A
#define PCI_CONFIG_REVISION 0x0B
#define PCI_CONFIG_BIST 0x0C
#define PCI_CONFIG_HEADER_TYPE 0x0D
#define PCI_CONFIG_LATENCY_TIMER 0x0E
#define PCI_CONFIG_CACHE_LINE_SIZE 0x0F

#define PCI_CONFIG_SECONDARY_BUS_NUMBER 0x1A

namespace DeviceManager { namespace PCI {
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