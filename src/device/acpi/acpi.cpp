#include <device/acpi/acpi.h>

#include <device/acpi/acpica/acpi.h>
#include <errno.h>
#include <panic.h>
#include <string.h>

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
    TableHeader* GetTable(const char* tableSignature) {
        if (strlen(tableSignature) != 4) {
            errno = ERROR_BAD_PARAMETER;
            return nullptr;
        }

        TableHeader* table;
        ACPI_STATUS status = AcpiGetTable((ACPI_STRING)tableSignature, 0, (ACPI_TABLE_HEADER**)&table);
        if (ACPI_FAILURE(status)) {
            errno = ERROR_ACPI_ERROR;
            return nullptr;
        }

        return table;
    }

    void Shutdown() {
        AcpiEnterSleepStatePrep(5);
        asm volatile("cli");
        AcpiEnterSleepState(5);

        panic("Should have shutdown!");
    }
} // namespace ACPI