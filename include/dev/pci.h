#pragma once

#include <types.h>

namespace DeviceManager { namespace PCI {
    bool RegisterPCIDriver();

    uint8_t ReadConfigB(uint32_t reg);
    uint16_t ReadConfigW(uint32_t reg);
    uint32_t ReadConfigD(uint32_t reg);

    void WriteConfigB(uint32_t red, uint8_t val);
    void WriteConfigW(uint32_t reg, uint16_t val);
    void WriteConfigD(uint32_t red, uint32_t val);
}} // namespace DeviceManager::PCI