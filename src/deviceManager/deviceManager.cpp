#include <dev.h>

#include <dev/acpi.h>

namespace DeviceManager {
    bool Init(void* rdsp) { return ACPI::Init((ACPI::RDSP*)rdsp); }
} // namespace DeviceManager