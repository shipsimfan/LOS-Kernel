#pragma once

#include <stdint.h>

namespace DeviceManager { namespace IDE {
    struct IDEDriveInfo;

    struct IDEDeviceInfo {
        struct {
            uint16_t IO;
            uint16_t control;
            uint16_t busMaster;
            uint8_t nIEN;
        } channels[2];

        uint8_t irq1;
        uint8_t irq2;

        IDEDriveInfo* drives[4];
    };

    struct IDEDriveInfo {
        IDEDeviceInfo* device;

        uint8_t channel;
        uint8_t drive;
        uint16_t type;
        uint16_t sign;
        uint16_t capabilities;
        uint32_t commandsets;
        uint32_t size;
        uint8_t model[41];
    };

    void RegisterIDEDriver();
}} // namespace DeviceManager::IDE