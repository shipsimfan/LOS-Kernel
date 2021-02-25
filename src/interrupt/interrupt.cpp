#include <interrupt/exception.h>
#include <interrupt/irq.h>
#include <interrupt/stack.h>

#include <mutex.h>
#include <panic.h>
#include <string.h>

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

    extern "C" void CommonExceptionHandler(Registers reg, ExceptionInfo info) {
        if (exceptionHandlers[info.interrupt] != nullptr)
            exceptionHandlers[info.interrupt](reg, info);
        else
            panic("Exception %#X has occurred (%i)", info.interrupt, info.errorCode);
    }

    extern "C" void CommonIRQHandler() {}

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

    bool InstallIRQHandler(uint8_t irq, IRQHandler handler, void* context = nullptr) { return false; }

    void* RemoveIRQHandler(uint8_t irq) { return nullptr; }

    void SetInterruptStack(uint64_t stackBase) { tss.rsp0 = stackBase; }
} // namespace Interrupt