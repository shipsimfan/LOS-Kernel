#include <dev/acpi.h>

#include <logger.h>

namespace DeviceManager { namespace ACPI {
    void Init(void* rdsp) { infoLogger.Log("RDSP Location: %#llx", rdsp); }
}} // namespace DeviceManager::ACPI