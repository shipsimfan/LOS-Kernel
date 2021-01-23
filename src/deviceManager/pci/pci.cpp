#include <dev/pci.h>

#include <dev.h>
#include <logger.h>

namespace DeviceManager { namespace PCI {
    bool RegisterPCIDriver() {
        infoLogger.Log("Initializing PCI . . . ");
        return true;
    }

    uint8_t ReadConfigB(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return (uint8_t)((ind(0xCFC) >> ((reg & 2) * 8)) & 0xFF);
    }

    uint16_t ReadConfigW(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return (uint16_t)((ind(0xCFC) >> ((reg & 2) * 8)) & 0xFFFF);
    }

    uint32_t ReadConfigD(uint32_t reg) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        return ind(0xCFC);
    }

    void WriteConfigB(uint32_t reg, uint8_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val << ((reg & 2) * 8));
    }

    void WriteConfigW(uint32_t reg, uint16_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val << ((reg & 2) * 8));
    }

    void WriteConfigD(uint32_t reg, uint32_t val) {
        outd(0xCF8, (reg & 0xFFFFFFFC) | 0x80000000);
        outd(0xCFC, val);
    }
}} // namespace DeviceManager::PCI