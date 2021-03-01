#include <device/acpi/acpi.h>

#include <device/acpi/acpica/acpi.h>
#include <panic.h>

extern "C" void InitACPITables() {
    ACPI_STATUS status = AcpiInitializeSubsystem();
    if (ACPI_FAILURE(status))
        panic("Failed to initialize ACPICA subsystem!");

    status = AcpiInitializeTables(NULL, 16, FALSE);
    if (ACPI_FAILURE(status))
        panic("Failed to initialize ACPICA tables!");

    status = AcpiLoadTables();
    if (ACPI_FAILURE(status))
        panic("Failed to load APCICA tables!");
}

namespace ACPI {
}