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

    void RegisterChildDriver(DeviceDriver* driver) {
        Device* device = ACPIDriver.deviceHead;
        while (device != nullptr)
            device = RegisterDevice(device, driver);
    }

    bool VerifyDevice(Device* device) { return true; }

    void ACPIRegisterDevice(Device* device) {
        // Initialize the ACPI device name
        ACPI_DEVICE_INFO* devInfo;
        AcpiGetObjectInfo((ACPI_HANDLE)device->driverInfo, &devInfo);

        device->name = (char*)malloc(11);
        device->name[0] = 'A';
        device->name[1] = 'C';
        device->name[2] = 'P';
        device->name[3] = 'I';
        device->name[4] = ':';
        device->name[5] = ' ';
        device->name[6] = devInfo->Name & 0xFF;
        device->name[7] = (devInfo->Name >> 8) & 0xFF;
        device->name[8] = (devInfo->Name >> 16) & 0xFF;
        device->name[9] = (devInfo->Name >> 24) & 0xFF;
        device->name[10] = 0;

        device->driverInfo = nullptr;
    }

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

        InterruptHandler::InitAPIC(madt);

        infoLogger.Log("ACPI initialized!");

        return true;
    }
}} // namespace DeviceManager::ACPI