#include <dev/acpi.h>

#include <logger.h>
#include <mem/defs.h>
#include <stdlib.h>
#include <string.h>

namespace DeviceManager { namespace ACPI {
    SDTHeader** systemDescriptorTables;
    uint32_t numDescriptorTables;

    SDTHeader* RSDT::GetEntry(int i) {
        if (i >= GetNumEntries())
            return nullptr;

        return (SDTHeader*)((uint64_t)(&entries)[i]);
    }

    int RSDT::GetNumEntries() { return (length - sizeof(SDTHeader)) / 4; }

    SDTHeader* XSDT::GetEntry(int i) {
        if (i >= GetNumEntries())
            return nullptr;

        return (SDTHeader*)(&entries)[i];
    }

    int XSDT::GetNumEntries() { return (length - sizeof(SDTHeader)) / 8; }

    bool Init(RDSP* rdsp) {
        infoLogger.Log("Loading ACPI Tables . . .");

        rdsp = (RDSP*)((uint64_t)rdsp + KERNEL_VMA);

        if (rdsp->revision != 2) {
            errorLogger.Log("Inavlid ACPI revision!");
            return false;
        }

        if (rdsp->xsdtAddr == 0) {
            infoLogger.Log("Using RSDT");
            RSDT* rsdt = (RSDT*)(rdsp->rsdtAddr + KERNEL_VMA);
            numDescriptorTables = rsdt->GetNumEntries();
            systemDescriptorTables = (SDTHeader**)malloc(numDescriptorTables * sizeof(SDTHeader*));

            for (uint64_t i = 0; i < numDescriptorTables; i++)
                systemDescriptorTables[i] = (SDTHeader*)((uint64_t)rsdt->GetEntry(i) + KERNEL_VMA);
        } else {
            infoLogger.Log("Using XSDT");
            XSDT* xsdt = (XSDT*)(rdsp->xsdtAddr + KERNEL_VMA);
            numDescriptorTables = xsdt->GetNumEntries();
            systemDescriptorTables = (SDTHeader**)malloc(numDescriptorTables * sizeof(SDTHeader*));

            for (uint64_t i = 0; i < numDescriptorTables; i++)
                systemDescriptorTables[i] = (SDTHeader*)((uint64_t)xsdt->GetEntry(i) + KERNEL_VMA);
        }

        for (uint64_t i = 0; i < numDescriptorTables; i++)
            infoLogger.Log("ACPI Table Signature: %c%c%c%c", systemDescriptorTables[i]->signature[0], systemDescriptorTables[i]->signature[1], systemDescriptorTables[i]->signature[2], systemDescriptorTables[i]->signature[3]);

        return true;
    }
}} // namespace DeviceManager::ACPI