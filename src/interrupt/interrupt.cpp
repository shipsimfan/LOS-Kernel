#include <interrupt/exception.h>
#include <interrupt/irq.h>
#include <interrupt/stack.h>

#include <device/acpi/acpi.h>
#include <device/util.h>
#include <errno.h>
#include <memory/virtual.h>
#include <mutex.h>
#include <panic.h>
#include <string.h>
#include <time.h>

#include <console.h>

#include "interrupt.h"

namespace Interrupt {
    ExceptionHandler exceptionHandlers[NUM_EXCEPTIONS];
    Mutex exceptionHandlersMutex;

    IRQHandler irqHandlers[NUM_IRQ];
    void* irqContexts[NUM_IRQ];
    Mutex irqHandlersMutex;

    IDTDescr idt[256];
    CPUPointer idtr;

    GDTDescr gdt[7];
    CPUPointer gdtr;
    TSS tss;

    uint32_t* localAPIC;

    extern "C" void CommonExceptionHandler(Registers reg, ExceptionInfo info) {
        if (exceptionHandlers[info.interrupt] != nullptr)
            exceptionHandlers[info.interrupt](reg, info);
        else
            panic("Exception %#X has occurred (%i)", info.interrupt, info.errorCode);
    }

    extern "C" void CommonIRQHandler(uint64_t irqNumber) {
        if (irqHandlers[irqNumber] != nullptr)
            irqHandlers[irqNumber](irqContexts[irqNumber]);

        localAPIC[LAPIC_EOI] = 0;
        if (irqNumber >= 8)
            outb(SLAVE_PIC_COMMAND, 0x20);

        outb(MASTER_PIC_COMMAND, 0x20);
    }

    void InstallInterruptHandler(uint8_t interrupt, uint64_t offset) {
        idt[interrupt].offset1 = offset & 0xFFFF;
        idt[interrupt].offset2 = (offset >> 16) & 0xFFFF;
        idt[interrupt].offset3 = (offset >> 32) & 0xFFFFFFFF;

        idt[interrupt].selector = GDT_CODE0;
        idt[interrupt].typeAttr = 0b11101110;

        idt[interrupt].ist = 0;
        idt[interrupt].zero = 0;
    }

    extern "C" void InitExceptions() {
        // Clear the exception handlers
        for (int i = 0; i < NUM_EXCEPTIONS; i++)
            exceptionHandlers[i] = nullptr;

        // Clear the irq handlers
        for (int i = 0; i < NUM_IRQ; i++)
            irqHandlers[i] = nullptr;

        // Clear the gdt, idt, and tss
        memset(idt, 0, sizeof(idt));
        memset(gdt, 0, sizeof(gdt));
        memset(&tss, 0, sizeof(tss));

        // Prepare the GDT

        // CODE 0
        gdt[GDT_CODE0_IDX].limitLow = 0xFFFF;
        gdt[GDT_CODE0_IDX].write = 1;
        gdt[GDT_CODE0_IDX].executable = 1;
        gdt[GDT_CODE0_IDX].code = 1;
        gdt[GDT_CODE0_IDX].present = 1;
        gdt[GDT_CODE0_IDX].limitHigh = 0xF;
        gdt[GDT_CODE0_IDX].code64 = 1;
        gdt[GDT_CODE0_IDX].granularity = 1;

        // DATA 0
        gdt[GDT_DATA0_IDX].limitLow = 0xFFFF;
        gdt[GDT_DATA0_IDX].write = 1;
        gdt[GDT_DATA0_IDX].code = 1;
        gdt[GDT_DATA0_IDX].present = 1;
        gdt[GDT_DATA0_IDX].limitHigh = 0xF;
        gdt[GDT_DATA0_IDX].size = 1;
        gdt[GDT_DATA0_IDX].granularity = 1;

        // CODE 3
        gdt[GDT_CODE3_IDX].limitLow = 0xFFFF;
        gdt[GDT_CODE3_IDX].write = 1;
        gdt[GDT_CODE3_IDX].executable = 1;
        gdt[GDT_CODE3_IDX].code = 1;
        gdt[GDT_CODE3_IDX].dpl = 3;
        gdt[GDT_CODE3_IDX].present = 1;
        gdt[GDT_CODE3_IDX].limitHigh = 0xF;
        gdt[GDT_CODE3_IDX].code64 = 1;
        gdt[GDT_CODE3_IDX].granularity = 1;

        // DATA 3
        gdt[GDT_DATA3_IDX].limitLow = 0xFFFF;
        gdt[GDT_DATA3_IDX].write = 1;
        gdt[GDT_DATA3_IDX].code = 1;
        gdt[GDT_DATA3_IDX].dpl = 3;
        gdt[GDT_DATA3_IDX].present = 1;
        gdt[GDT_DATA3_IDX].limitHigh = 0xF;
        gdt[GDT_DATA3_IDX].size = 1;
        gdt[GDT_DATA3_IDX].granularity = 1;

        // TSS
        uint64_t tssBase = (uint64_t)(&tss);
        gdt[GDT_TSS_IDX].limitLow = sizeof(tss);
        gdt[GDT_TSS_IDX].baseLow = tssBase & 0xFFFF;
        gdt[GDT_TSS_IDX].baseMid = (tssBase >> 16) & 0xFF;
        gdt[GDT_TSS_IDX].access = 1;
        gdt[GDT_TSS_IDX].executable = 1;
        gdt[GDT_TSS_IDX].dpl = 3;
        gdt[GDT_TSS_IDX].present = 1;
        gdt[GDT_TSS_IDX].granularity = 1;
        gdt[GDT_TSS_IDX].baseHigh = (tssBase >> 24) & 0xFF;
        gdt[GDT_TSS_IDX + 1].limitLow = (tssBase >> 32) & 0xFFFF;
        gdt[GDT_TSS_IDX + 1].baseLow = (tssBase >> 48) & 0xFFFF;

        // Prepare the GDTR
        gdtr.limit = sizeof(gdt) - 1;
        gdtr.pointer = gdt;

        // Prepare the IDT
        InstallInterruptHandler(0, (uint64_t)ExceptionHandler0);
        InstallInterruptHandler(1, (uint64_t)ExceptionHandler1);
        InstallInterruptHandler(2, (uint64_t)ExceptionHandler2);
        InstallInterruptHandler(3, (uint64_t)ExceptionHandler3);
        InstallInterruptHandler(4, (uint64_t)ExceptionHandler4);
        InstallInterruptHandler(5, (uint64_t)ExceptionHandler5);
        InstallInterruptHandler(6, (uint64_t)ExceptionHandler6);
        InstallInterruptHandler(7, (uint64_t)ExceptionHandler7);
        InstallInterruptHandler(8, (uint64_t)ExceptionHandler8);
        InstallInterruptHandler(9, (uint64_t)ExceptionHandler9);
        InstallInterruptHandler(10, (uint64_t)ExceptionHandler10);
        InstallInterruptHandler(11, (uint64_t)ExceptionHandler11);
        InstallInterruptHandler(12, (uint64_t)ExceptionHandler12);
        InstallInterruptHandler(13, (uint64_t)ExceptionHandler13);
        InstallInterruptHandler(14, (uint64_t)ExceptionHandler14);
        InstallInterruptHandler(15, (uint64_t)ExceptionHandler15);
        InstallInterruptHandler(16, (uint64_t)ExceptionHandler16);
        InstallInterruptHandler(17, (uint64_t)ExceptionHandler17);
        InstallInterruptHandler(18, (uint64_t)ExceptionHandler18);
        InstallInterruptHandler(19, (uint64_t)ExceptionHandler19);
        InstallInterruptHandler(20, (uint64_t)ExceptionHandler20);
        InstallInterruptHandler(21, (uint64_t)ExceptionHandler21);
        InstallInterruptHandler(22, (uint64_t)ExceptionHandler22);
        InstallInterruptHandler(23, (uint64_t)ExceptionHandler23);
        InstallInterruptHandler(24, (uint64_t)ExceptionHandler24);
        InstallInterruptHandler(25, (uint64_t)ExceptionHandler25);
        InstallInterruptHandler(26, (uint64_t)ExceptionHandler26);
        InstallInterruptHandler(27, (uint64_t)ExceptionHandler27);
        InstallInterruptHandler(28, (uint64_t)ExceptionHandler28);
        InstallInterruptHandler(29, (uint64_t)ExceptionHandler29);
        InstallInterruptHandler(30, (uint64_t)ExceptionHandler30);
        InstallInterruptHandler(31, (uint64_t)ExceptionHandler31);

        // Prepare the IDTR
        idtr.limit = sizeof(idt) - 1;
        idtr.pointer = idt;

        // Install GDT and IDT
        InstallGDT(&gdtr, GDT_DATA0, GDT_TSS);
        InstallIDT(&idtr);

        // Enable interrupts
        asm volatile("sti");
    }

    extern "C" void InitIRQ() {
        // Get MADT
        ACPI::MADT* madt = (ACPI::MADT*)ACPI::GetTable(MADT_SIGNATURE);
        if (madt == nullptr)
            panic("Failed to get MADT! (%#llX)", errno);

        // Verify 8259 PICs
        if ((madt->flags & 1) == 0)
            panic("No 8259 PICs installed! I/O APIC not currently supported!");

        // Save the local APIC address
        localAPIC = (uint32_t*)((uint64_t)madt->localAPICAddress + KERNEL_VMA);

        // Find and mask all IO APICs
        // Also look for a APIC address override
        ACPI::MADT::EntryHeader* entry = madt->entries;
        for (uint64_t i = (uint64_t)entry; i < (uint64_t)madt + madt->length; i += entry->length, entry = (ACPI::MADT::EntryHeader*)i) {
            switch (entry->type) {
            case ACPI::MADT::EntryType::IO_APIC: {
                ACPI::MADT::IOAPICEntry* ioapic = (ACPI::MADT::IOAPICEntry*)entry;
                uint32_t* selectReg = (uint32_t*)((uint64_t)ioapic->address + KERNEL_VMA);
                uint32_t* dataReg = (uint32_t*)((uint64_t)selectReg + 0x10);

                *selectReg = 1; // Select IOAPICVER
                uint8_t numIRQ = ((*dataReg) >> 16) + 1;
                for (int i = 0; i < numIRQ; i++) {
                    *selectReg = 0x10 + 2 * i;
                    *dataReg = LAPIC_DISABLE;
                }

                break;
            }

            case ACPI::MADT::EntryType::LOCAL_APIC_ADDRESS_OVERRIDE:
                localAPIC = (uint32_t*)(((ACPI::MADT::LocalAPICAddressOverrideEntry*)entry)->address + KERNEL_VMA);
                break;

            default:
                break;
            }
        }

        // Allocate the local APIC address
        // Done after madt loop incase of an override address
        Memory::Virtual::Allocate(localAPIC, (uint64_t)localAPIC - KERNEL_VMA);

        // Initialize the local APIC
        InstallInterruptHandler(SPURIOUS_INTERRUPT_VECTOR, (uint64_t)SpuriousIRQHandler);
        localAPIC[LAPIC_SPURIOUS_INTERRUPT_VECTOR] = SPURIOUS_INTERRUPT_VECTOR | 0x100;
        localAPIC[LAPIC_TASK_PRIORITY] = 0;

        // Install IRQ handlers
        InstallInterruptHandler(NUM_EXCEPTIONS + 0, (uint64_t)IRQHandler0);
        InstallInterruptHandler(NUM_EXCEPTIONS + 1, (uint64_t)IRQHandler1);
        InstallInterruptHandler(NUM_EXCEPTIONS + 2, (uint64_t)IRQHandler2);
        InstallInterruptHandler(NUM_EXCEPTIONS + 3, (uint64_t)IRQHandler3);
        InstallInterruptHandler(NUM_EXCEPTIONS + 4, (uint64_t)IRQHandler4);
        InstallInterruptHandler(NUM_EXCEPTIONS + 5, (uint64_t)IRQHandler5);
        InstallInterruptHandler(NUM_EXCEPTIONS + 6, (uint64_t)IRQHandler6);
        InstallInterruptHandler(NUM_EXCEPTIONS + 7, (uint64_t)IRQHandler7);
        InstallInterruptHandler(NUM_EXCEPTIONS + 8, (uint64_t)IRQHandler8);
        InstallInterruptHandler(NUM_EXCEPTIONS + 9, (uint64_t)IRQHandler9);
        InstallInterruptHandler(NUM_EXCEPTIONS + 10, (uint64_t)IRQHandler10);
        InstallInterruptHandler(NUM_EXCEPTIONS + 11, (uint64_t)IRQHandler11);
        InstallInterruptHandler(NUM_EXCEPTIONS + 12, (uint64_t)IRQHandler12);
        InstallInterruptHandler(NUM_EXCEPTIONS + 13, (uint64_t)IRQHandler13);
        InstallInterruptHandler(NUM_EXCEPTIONS + 14, (uint64_t)IRQHandler14);
        InstallInterruptHandler(NUM_EXCEPTIONS + 15, (uint64_t)IRQHandler15);

        // Initialize the 8259 PICs
        outb(MASTER_PIC_COMMAND, 0x11);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(SLAVE_PIC_COMMAND, 0x11);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(MASTER_PIC_DATA, NUM_EXCEPTIONS);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(SLAVE_PIC_DATA, NUM_EXCEPTIONS + 8);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(MASTER_PIC_DATA, 4);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(SLAVE_PIC_DATA, 2);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(MASTER_PIC_DATA, 0x01);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(SLAVE_PIC_DATA, 0x01);
        outb(0x80, 0);
        outb(0x80, 0);
        outb(MASTER_PIC_DATA, 0);
        outb(SLAVE_PIC_DATA, 0);
    }

    extern "C" void InitLocalAPICTimer() {
        // Set the interrupt
        localAPIC[LAPIC_TIMER_LVT] = LAPIC_DISABLE;
        InstallInterruptHandler(PREEMPT_INTERRUPT_VECTOR, (uint64_t)PreemptHandler);

        // Set the divider
        localAPIC[LAPIC_TIMER_DIV] = 0x3;

        // Wait till the start of the millisecond
        uint64_t start = GetCurrentTime() + 1;
        while (GetCurrentTime() < start)
            ;

        localAPIC[LAPIC_TIMER_LVT] = PREEMPT_INTERRUPT_VECTOR;
        localAPIC[LAPIC_TIMER_INITCNT] = 0xFFFFFFFF;

        while (GetCurrentTime() < start + 10)
            ;

        localAPIC[LAPIC_TIMER_LVT] = LAPIC_DISABLE;

        uint32_t ticksIn10ms = 0xFFFFFFFF - localAPIC[LAPIC_TIMER_CURCNT];

        localAPIC[LAPIC_TIMER_LVT] = PREEMPT_INTERRUPT_VECTOR | 0x20000;
        localAPIC[LAPIC_TIMER_INITCNT] = ticksIn10ms;
    }

    bool InstallExceptionHandler(ExceptionType exception, ExceptionHandler handler) {
        exceptionHandlersMutex.Lock();
        if (exceptionHandlers[exception] != nullptr) {
            exceptionHandlersMutex.Unlock();
            return false;
        }

        exceptionHandlers[exception] = handler;
        exceptionHandlersMutex.Unlock();
        return true;
    }

    void RemoveExceptionHandler(ExceptionType exception) {
        exceptionHandlersMutex.Lock();
        exceptionHandlers[exception] = nullptr;
        exceptionHandlersMutex.Unlock();
    }

    bool InstallIRQHandler(uint8_t irq, IRQHandler handler, void* context = nullptr) {
        if (irq > NUM_IRQ)
            return false;

        if (irqHandlers[irq] != nullptr)
            return false;

        irqContexts[irq] = context;
        irqHandlers[irq] = handler;

        return true;
    }

    void* RemoveIRQHandler(uint8_t irq) {
        if (irq > NUM_IRQ)
            return nullptr;

        void* context = irqContexts[irq];
        irqHandlers[irq] = nullptr;
        irqContexts[irq] = nullptr;
        return context;
    }

    void SetInterruptStack(uint64_t stackBase) { tss.rsp0 = stackBase; }
} // namespace Interrupt