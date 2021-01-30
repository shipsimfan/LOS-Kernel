#include <dev.h>
#include <dev/acpi/acpi.h>
#include <dev/pci.h>
#include <interrupt.h>
#include <kernel/time.h>
#include <logger.h>
#include <mem/heap.h>
#include <mem/virtual.h>

// Environmental and ACPI Tables

extern "C" extern "C" ACPI_STATUS AcpiOsInitialize() { return AE_OK; }

extern "C" ACPI_STATUS AcpiOsTerminate() {
    errorLogger.Log("ACPICA Terminating\n");
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* predefinedObject, ACPI_STRING* newValue) {
    *newValue = nullptr;
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* existingTable, ACPI_TABLE_HEADER** newTable) {
    *newTable = nullptr;
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* existingTable, ACPI_PHYSICAL_ADDRESS* newAddress, UINT32* newTableLength) {
    *newAddress = 0;
    *newTableLength = 0;
    return AE_OK;
}

// Memory Management
extern "C" void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS physicalAddress, ACPI_SIZE length) {
    UINT64 virtualAddress = physicalAddress + KERNEL_VMA;
    UINT64 originalVirtAddress = virtualAddress;
    UINT64 numPages = (length + PAGE_SIZE - 1) / PAGE_SIZE;

    for (UINT64 i = 0; i < numPages; i++, virtualAddress += PAGE_SIZE, physicalAddress += PAGE_SIZE)
        MemoryManager::Virtual::AllocatePage((virtAddr_t)virtualAddress, physicalAddress, true);

    return (void*)originalVirtAddress;
}

extern "C" void AcpiOsUnmapMemory(void* where, ACPI_SIZE length) {}

extern "C" ACPI_STATUS AcpiOsGetPhysicalAddress(void* logicalAddress, ACPI_PHYSICAL_ADDRESS* physicalAddress) {
    warningLogger.Log("ACPICA Get physical address\n");
    return 0;
}

extern "C" void* AcpiOsAllocate(ACPI_SIZE size) { return MemoryManager::Heap::malloc(size); }

extern "C" void AcpiOsFree(void* memory) { MemoryManager::Heap::free(memory); }

extern "C" BOOLEAN AcpiOsReadable(void* memory, ACPI_SIZE length) {
    warningLogger.Log("ACPICA Readable\n");
    return TRUE;
}

extern "C" BOOLEAN AcpiOsWritable(void* memory, ACPI_SIZE length) {
    warningLogger.Log("ACPICA Writable\n");
    return TRUE;
}

extern "C" ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS address, UINT64 value, UINT32 width) {
    warningLogger.Log("ACPICA Write memory\n");
    return 0;
}

extern "C" ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS address, UINT64* value, UINT32 width) {
    warningLogger.Log("ACPICA Read memory\n");
    return 0;
}

extern "C" ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* pciId, UINT32 reg, UINT64* value, UINT32 width) {
    switch (width) {
    case 8:
        *value = DeviceManager::PCI::ReadConfigB(reg);
        break;

    case 16:
        *value = DeviceManager::PCI::ReadConfigW(reg);
        break;

    case 32:
        *value = DeviceManager::PCI::ReadConfigD(reg);
        break;

    default:
        errorLogger.Log("Inavlid ACPICA PCI read width (%i)", width);
        return AE_ERROR;
    }

    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* pciId, UINT32 reg, UINT64 value, UINT32 width) {
    switch (width) {
    case 8:
        DeviceManager::PCI::WriteConfigB(reg, value);
        break;

    case 16:
        DeviceManager::PCI::WriteConfigW(reg, value);
        break;

    case 32:
        DeviceManager::PCI::WriteConfigD(reg, value);
        break;

    default:
        errorLogger.Log("Inavlid ACPICA PCI read width (%i)", width);
        return AE_ERROR;
    }

    return AE_OK;
}

// Multithreading and scheduling services
extern "C" ACPI_THREAD_ID AcpiOsGetThreadId() {
    return 1; // No multitasking yet
}

extern "C" ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE type, ACPI_OSD_EXEC_CALLBACK function, void* context) {
    warningLogger.Log("ACPICA Execute");
    function(context);
    return 0;
}

extern "C" void AcpiOsSleep(UINT64 milliseconds) { sleep(milliseconds); }

extern "C" ACPI_STATUS AcpiOsEnterSleep(UINT8 sleepState, UINT32 regaValue, UINT32 regbValue) {
    warningLogger.Log("ACPICA Enter sleep\n");
    return 0;
}

extern "C" void AcpiOsStall(UINT32 microseconds) { sleep((microseconds + 999) / 1000); }

extern "C" ACPI_STATUS AcpiOsSignal(UINT32 function, void* info) {
    warningLogger.Log("ACPICA Signal\n");
    return 0;
}

extern "C" UINT64 AcpiOsGetTimer() {
    // printf("ACPICA Get timer\n");
    return 0;
}

extern "C" void AcpiOsWaitEventsComplete() { warningLogger.Log("ACPICA Wait events complete\n"); }

// Mutual Exclusion and Synchronization
extern "C" ACPI_STATUS AcpiOsCreateSemaphore(UINT32 maxUnits, UINT32 initialUnits, ACPI_SEMAPHORE* outHandle) {
    return AE_OK; // No multitasking yet
}

extern "C" ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE handle) {
    return AE_OK; // No multitasking yet
}

extern "C" ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE handle, UINT32 units, UINT16 timeout) {
    return AE_OK; // No multitasking yet
}

extern "C" ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE handle, UINT32 units) {
    return AE_OK; // No multitasking yet
}

extern "C" ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* outHandle) {
    return AE_OK; // No multitasking yet
}

extern "C" void AcpiOsDeleteLock(ACPI_HANDLE handle) {}

extern "C" ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK handle) {
    return AE_OK; // No multitasking yet
}

extern "C" void AcpiOsReleaseLock(ACPI_SPINLOCK handle, ACPI_CPU_FLAGS flags) {} // No multitasking yet

// Interrupt Handling
extern "C" ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 interruptNumber, ACPI_OSD_HANDLER handler, void* context) {
    warningLogger.Log("ACPICA Install interrupt handler(%i)", interruptNumber);
    return 0;
}

extern "C" ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 interruptNumber, ACPI_OSD_HANDLER handler) {
    InterruptHandler::ClearInterruptHandler(interruptNumber);
    return AE_OK;
}

// HardwareACPI_STATUS
extern "C" ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS address, UINT32 value, UINT32 width) {
    switch (width) {
    case 8:
        DeviceManager::outb(address, value);
        break;

    case 16:
        DeviceManager::outw(address, value);
        break;

    case 32:
        DeviceManager::outd(address, value);
        break;

    default:
        errorLogger.Log("Invalid ACPICA write port width (%i)", width);
        return AE_ERROR;
    }

    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS address, UINT32* value, UINT32 width) {
    switch (width) {
    case 8:
        *value = DeviceManager::inb(address);
        break;

    case 16:
        *value = DeviceManager::inw(address);
        break;

    case 32:
        *value = DeviceManager::ind(address);
        break;

    default:
        errorLogger.Log("Invalid ACPICA read port width (%i)", width);
        return AE_ERROR;
    }

    return AE_OK;
}

// Printf
extern "C" void AcpiOsPrintf(const char* format, ...) {}
extern "C" void AcpiOsVprintf(const char* format, va_list args) {}