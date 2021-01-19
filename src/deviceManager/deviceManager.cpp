#include <dev.h>

#include <dev/acpi.h>

namespace DeviceManager {
    void Init(void* rdsp) { ACPI::Init(rdsp); }
} // namespace DeviceManager