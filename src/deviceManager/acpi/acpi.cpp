#include <dev/acpi.h>

#include <dev/acpi/acpi.h>
#include <interrupt.h>
#include <logger.h>

#define ACPI_NAME_BUFFER_LENGTH 256

namespace DeviceManager { namespace ACPI {
    ACPI_PHYSICAL_ADDRESS rdspAddr;

    DeviceManager::DeviceDriver ACPIDriver;

    extern "C" ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() { return rdspAddr; }

    bool InitACPICA() {
        ACPI_STATUS status = AcpiInitializeSubsystem();
        if (ACPI_FAILURE(status)) {
            errorLogger.Log("An error ocurred while initializing subsystem(%i)", status);
            return false;
        }

        status = AcpiInitializeTables(nullptr, 16, FALSE);
        if (ACPI_FAILURE(status)) {
            errorLogger.Log("An error occured while initializing tables (%i)", status);
            return false;
        }

        status = AcpiLoadTables();
        if (ACPI_FAILURE(status)) {
            errorLogger.Log("An error occured while loading tables (%i)", status);
            return false;
        }

        return true;
    }

    void RegisterChildDriver(DeviceDriver* driver) {}

    bool VerifyDevice(Device* device) { return false; }

    void ACPIRegisterDevice(Device* device) {}

    bool RegisterACPIDriver(void* rdsp) {
        infoLogger.Log("Initializing ACPI . . . ");

        rdspAddr = (ACPI_PHYSICAL_ADDRESS)rdsp;

        ACPIDriver.signature = DEVICE_DRIVER_SIGNATURE_ACPI;
        ACPIDriver.name = "LOS Integrated Intel ACPICA";
        ACPIDriver.parent = nullptr;
        ACPIDriver.RegisterChildDriver = RegisterChildDriver;
        ACPIDriver.VerifyDevice = VerifyDevice;
        ACPIDriver.RegisterDevice = ACPIRegisterDevice;

        DeviceManager::RegisterDeviceDriver(&ACPIDriver);

        // Initialize ACPICA
        if (!InitACPICA())
            return false;

        // Initialize APIC
        ACPI_TABLE_HEADER* madt;
        ACPI_STATUS status = AcpiGetTable((char*)"APIC", 1, &madt);
        if (ACPI_FAILURE(status)) {
            errorLogger.Log("Error while getting MADT! (%i)", status);
            return false;
        }

        if (!InterruptHandler::InitAPIC(madt))
            return false;

        infoLogger.Log("ACPI initialized!");

        return true;
    }

    void* GetTable(const char* signature) {
        ACPI_TABLE_HEADER* ret;
        ACPI_STATUS status = AcpiGetTable((char*)signature, 1, &ret);
        if (ACPI_FAILURE(status)) {
            errorLogger.Log("Error while getting ACPI table with signature %s (%i)", signature, status);
            return nullptr;
        }

        return ret;
    }

    extern "C" void Shutdown() {
        AcpiEnterSleepStatePrep(5);
        InterruptHandler::DisableInterrupts();
        AcpiEnterSleepState(5);

        errorLogger.Log("Shouldn't be here!");
        while (1)
            ;
    }
}} // namespace DeviceManager::ACPI