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

        // Locate the FADT
        FADT* fadt = nullptr;
        for (uint64_t i = 0; i < numDescriptorTables; i++) {
            if (memcmp(systemDescriptorTables[i]->signature, "FACP", 4) == 0) {
                fadt = (FADT*)systemDescriptorTables[i];
                break;
            }
        }

        if (fadt == nullptr) {
            errorLogger.Log("Unable to locate Fixed ACPI Descriptor Table!");
            return false;
        }

        debugLogger.Log("FADT located at %#llx", fadt);

        // Get the DSDT
        DSDT* dsdt = nullptr;
        if (fadt->Xdsdt != 0)
            dsdt = (DSDT*)(fadt->Xdsdt + KERNEL_VMA);
        else if (fadt->dsdt != 0)
            dsdt = (DSDT*)((uint64_t)fadt->dsdt + KERNEL_VMA);

        if (dsdt == nullptr) {
            errorLogger.Log("Inavlid DSDT entries!");
            return false;
        }

        debugLogger.Log("DSDT located at %#llx", dsdt);

        return true;
    }
}} // namespace DeviceManager::ACPI