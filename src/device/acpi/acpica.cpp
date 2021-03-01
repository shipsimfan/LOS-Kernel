#include <device/acpi/acpica/acpiosxf.h>

#include <console.h>
#include <device/acpi/acpica/acexcep.h>
#include <memory/heap.h>
#include <process/process.h>

extern "C" uint64_t rsdp;

// State Functions
ACPI_STATUS AcpiOsInitialize() {
    Console::Println("[ACPICA] Initializing ACPICA . . . ");
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate() {
    Console::Println("[ACPICA] Terminating ACPICA . . . ");
    return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() { return rsdp; }

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength) {
    *NewAddress = 0;
    *NewTableLength = 0;
    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable) {
    *NewTable = nullptr;
    return AE_OK;
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* InitVal, ACPI_STRING* NewVal) {
    *NewVal = nullptr;
    return AE_OK;
}

// Synchronization Fucntions
ACPI_THREAD_ID AcpiOsGetThreadId() {
    if (currentProcess == nullptr)
        return 1;

    return currentProcess->id;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    Console::Println("[ACPICA] Execute");
    Function(Context);
    return AE_OK;
}

void AcpiOsWaitEventsComplete() { Console::Println("[ACPICA] Wait Events Complete"); }

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    Console::Println("[ACPICA] Signal");
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle) {
    Console::Println("[ACPICA] Create Semaphore");
    return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    // Console::Println("[ACPICA] Semaphore Signal");
    return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    // Console::Println("[ACPICA] Semaphore Wait");
    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    Console::Println("[ACPICA] Delete Semaphore");
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle) {
    Console::Println("[ACPICA] Create Lock");
    return AE_OK;
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    // Console::Println("[ACPICA] Acquire Lock");
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) { // Console::Println("[ACPICA] Release Lock");
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) { Console::Println("[ACPICA] Delete Lock"); }

ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX* OutHandle) {
    Mutex* mutex = new Mutex;
    *OutHandle = mutex;
    return AE_OK;
}

void AcpiOsDeleteMutex(ACPI_MUTEX Handle) {
    Mutex* mutex = (Mutex*)Handle;
    delete mutex;
}

ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout) {
    Mutex* mutex = (Mutex*)Handle;
    mutex->Lock();
    return AE_OK;
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle) {
    Mutex* mutex = (Mutex*)Handle;
    mutex->Unlock();
}

// Memory Functions
void AcpiOsFree(void* Memory) { Memory::Heap::Free(Memory); }
void* AcpiOsAllocate(ACPI_SIZE Size) { return Memory::Heap::Allocate(Size); }

void AcpiOsUnmapMemory(void* LogicalAddress, ACPI_SIZE Size) {}
void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length) { return (void*)(Where + KERNEL_VMA); }

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width) {
    Console::Println("[ACPICA] Read Memory");
    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    Console::Println("[ACPICA] Write Memory");
    return AE_OK;
}

// Print Functions
void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char* Format, ...) {}
void AcpiOsVprintf(const char* Format, va_list Args) {}

// Hardware Functions
ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width) {
    Console::Println("[ACPICA] Read Port");
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
    Console::Println("[ACPICA] Write Port");
    return AE_OK;
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width) {
    Console::Println("[ACPICA] Read PCI Configuration");
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width) {
    Console::Println("[ACPICA] Write PCI Configuration");

    return AE_OK;
}

// Time Functions
void AcpiOsStall(UINT32 Microseconds) { Console::Println("[ACPICA] Stall"); }

void AcpiOsSleep(UINT64 Milliseconds) { Console::Println("[ACPICA] Sleep"); }

UINT64 AcpiOsGetTimer() {
    Console::Println("[ACPICA] Get Timer");
    return 0;
}

// Interrupt Functions
ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void* Context) {
    Console::Println("[ACPICA] Install Interrupt Handler");
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    Console::Println("[ACPICA] Remove Interrupt Handler");
    return AE_OK;
}