#pragma once

#include <dev.h>

namespace DeviceManager { namespace ACPI {
    bool RegisterACPIDriver(void* rdsp);

    void* GetTable(const char* signature);
}} // namespace DeviceManager::ACPI