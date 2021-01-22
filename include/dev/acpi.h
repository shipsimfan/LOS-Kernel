#pragma once

#include <dev.h>

namespace DeviceManager { namespace ACPI {
    struct DeviceInfo {
        uint32_t name;
    };

    bool RegisterACPIDriver(void *rdsp);
}}