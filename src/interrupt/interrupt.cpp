#include <interrupt.h>

#include <dev.h>
#include <logger.h>
#include <mem/defs.h>
#include <mem/virtual.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAPIC_ID 0x20 / 4
#define LAPIC_VERSION 0x30 / 4

#define LAPIC_TASK_PRIORITY 0x80 / 4
#define LAPIC_ARBITRATION_PRIORITY 0x90 / 4
#define LAPIC_PROCESSOR_PRIORITY 0xA0 / 4
#define LAPIC_EOI 0xB0 / 4
#define LAPIC_REMOTE_READ 0xC0 / 4
#define LAPIC_LOGICAL_DESTINATION 0xD0 / 4
#define LAPIC_DESTINATION_FORMAT 0xE0 / 4
#define LAPIC_SPURIOUS_INTERRUPT_VECTOR 0xF0 / 4
#define LAPIC_IN_SERVICE_START 0x100 / 4

#define LAPIC_TRIGGER_MODE_START 0x180 / 4

#define LAPIC_INTERRUPT_REQUEST_START 0x200 / 4

#define LAPIC_ERROR_STATUS 0x280 / 4

#define LAPIC_CMCI_LVT 0x2F0 / 4
#define LAPIC_INTERRUPT_CONTROL 0x300 / 4
#define LAPIC_TIMER_LVT 0x320 / 4
#define LAPIC_THERMAL_SENSOR_LVT 0x330 / 4
#define LAPIC_PERFORMANCE_MONITOR_COUNTER_LVT 0x340 / 4
#define LAPIC_LINT0_LVT 0x350 / 4
#define LAPIC_LINT1_LVT 0x360 / 4
#define LAPIC_ERROR_LVT 0x370 / 4
#define LAPIC_TIMER_INITCNT 0x380 / 4
#define LAPIC_TIMER_CURCNT 0x390 / 4

#define LAPIC_TIMER_DIV 0x3E0 / 4

#define APIC_DISABLE 0x10000
#define APIC_NMI (4 << 8)
#define APIC_SW_ENABLE 0x100

extern "C" void InterruptHandler0();
extern "C" void InterruptHandler1();
extern "C" void InterruptHandler2();
extern "C" void InterruptHandler3();
extern "C" void InterruptHandler4();
extern "C" void InterruptHandler5();
extern "C" void InterruptHandler6();
extern "C" void InterruptHandler7();
extern "C" void InterruptHandler8();
extern "C" void InterruptHandler9();
extern "C" void InterruptHandler10();
extern "C" void InterruptHandler11();
extern "C" void InterruptHandler12();
extern "C" void InterruptHandler13();
extern "C" void InterruptHandler14();
extern "C" void InterruptHandler15();
extern "C" void InterruptHandler16();
extern "C" void InterruptHandler17();
extern "C" void InterruptHandler18();
extern "C" void InterruptHandler19();
extern "C" void InterruptHandler20();
extern "C" void InterruptHandler21();
extern "C" void InterruptHandler22();
extern "C" void InterruptHandler23();
extern "C" void InterruptHandler24();
extern "C" void InterruptHandler25();
extern "C" void InterruptHandler26();
extern "C" void InterruptHandler27();
extern "C" void InterruptHandler28();
extern "C" void InterruptHandler29();
extern "C" void InterruptHandler30();
extern "C" void InterruptHandler31();

extern "C" void InstallIDT();

extern "C" void DisableInterrupts();
extern "C" void EnableInterrupts();

extern "C" void SpuriousISRHandler();
extern "C" void TimerISRHandler();

extern "C" void EnableAPIC();

namespace InterruptHandler {
#pragma pack(push)
#pragma pack(1)
    struct MADTEntry {
        uint8_t type;
        uint8_t length;
    };

    struct LAPICEntry : public MADTEntry {
        uint8_t processorID;
        uint8_t apicID;
        uint32_t flags;
    };

    struct IOAPICEntry : public MADTEntry {
        uint8_t ioAPICID;
        uint8_t reserved;
        uint32_t ioAPICAddress;
        uint32_t gloablSystemInterruptBase;
    };

    struct MADT {
        uint8_t signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        uint8_t OEMID[6];
        uint8_t OEMTableID[8];
        uint32_t OEMRevision;
        uint32_t creatorID;
        uint32_t creatorRevision;
        uint32_t localAPICAddress;
        uint32_t flags;
        MADTEntry firstEntry;
    };
#pragma pack(pop)

    struct IOAPIC {
        IOAPIC* next;
        uint8_t id;
        uint32_t* selectRegister;
        uint32_t* dataRegister;
        uint32_t irqBase;
        uint8_t numIRQ;
    };

    const char* exceptions[] = {"Divide by zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available", "Double fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment not present", "Stack-segmentation", "General protection", "Page", "", "x87 Floating-Point", "Alignment Check", "Machine Check", "SIMD Floating-Point", "Virtualization"};

    IDTDescr idt[256];

    bool (*exceptionHandlers[32])(CPUState, StackState);
    void (*interruptHandlers[256 - 32])(void*);

    uint32_t* localAPIC;

    IOAPIC* ioapics;

    uint8_t irqMapping[256 - 32];
    uint8_t numIRQ;

    void InfoDump(CPUState cpu, StackState stack) {
        printf("rax: 0x%llx    rbx: 0x%llx    rcx: 0x%llx\n", cpu.rax, cpu.rbx, cpu.rcx);
        printf("rdx: 0x%llx    rsi: 0x%llx    rdi: 0x%llx\n", cpu.rdx, cpu.rsi, cpu.rdi);
        printf("               rbp: 0x%llx    r8:  0x%llx\n", cpu.rbp, cpu.r8);
        printf("r9:  0x%llx    r10: 0x%llx    r11: 0x%llx\n", cpu.r9, cpu.r10, cpu.r11);
        printf("r12: 0x%llx    r13: 0x%llx    r14: 0x%llx\n", cpu.r12, cpu.r13, cpu.r14);
        printf("r15: 0x%llx    rip: 0x%llx    cs:  0x%llx\n", cpu.r15, stack.rip, stack.cs);
        printf("rflags: 0x%llx rsp: 0x%llx    ss:  0x%llx\n", stack.rflags, stack.rsp, stack.ss);
    }

    extern "C" void ExceptionHandler(InterruptHandler::CPUState cpu, InterruptHandler::StackState stack) {
        if (exceptionHandlers[stack.interrupt] != nullptr) {
            if (exceptionHandlers[stack.interrupt](cpu, stack))
                return;
        } else {
            errorLogger.Log("%s exception (%i) has occurred!", exceptions[stack.interrupt], stack.interrupt);
            errorLogger.Log("Error code: %#x", stack.errorCode);

            errorLogger.Log("Core dump:");
            InfoDump(cpu, stack);
        }

        while (1)
            ;
    }

    void InstallInterruptHandler(int interrupt, uint64_t offset) {
        idt[interrupt].offset1 = offset & 0xFFFF;
        idt[interrupt].offset2 = (offset >> 16) & 0xFFFF;
        idt[interrupt].offset3 = (offset >> 32) & 0xFFFFFFFF;

        idt[interrupt].selector = 0x8;
        idt[interrupt].typeAttr = 0b10001110;

        idt[interrupt].ist = 0;
        idt[interrupt].zero = 0;
    }

    void WriteIOAPIC(IOAPIC* ioapic, uint8_t reg, uint32_t val) {
        *ioapic->selectRegister = reg;
        *ioapic->dataRegister = val;
    }

    uint32_t ReadIOAPIC(IOAPIC* ioapic, uint8_t reg) {
        *ioapic->selectRegister = reg;
        return *ioapic->dataRegister;
    }

    void Init() {
        infoLogger.Log("Initializing interrupt handler . . .");

        for (int i = 0; i < 32; i++)
            exceptionHandlers[i] = nullptr;

        memset(idt, 0, sizeof(idt));

        InstallInterruptHandler(0, (uint64_t)InterruptHandler0);
        InstallInterruptHandler(1, (uint64_t)InterruptHandler1);
        InstallInterruptHandler(2, (uint64_t)InterruptHandler2);
        InstallInterruptHandler(3, (uint64_t)InterruptHandler3);
        InstallInterruptHandler(4, (uint64_t)InterruptHandler4);
        InstallInterruptHandler(5, (uint64_t)InterruptHandler5);
        InstallInterruptHandler(6, (uint64_t)InterruptHandler6);
        InstallInterruptHandler(7, (uint64_t)InterruptHandler7);
        InstallInterruptHandler(8, (uint64_t)InterruptHandler8);
        InstallInterruptHandler(9, (uint64_t)InterruptHandler9);
        InstallInterruptHandler(10, (uint64_t)InterruptHandler10);
        InstallInterruptHandler(11, (uint64_t)InterruptHandler11);
        InstallInterruptHandler(12, (uint64_t)InterruptHandler12);
        InstallInterruptHandler(13, (uint64_t)InterruptHandler13);
        InstallInterruptHandler(14, (uint64_t)InterruptHandler14);
        InstallInterruptHandler(15, (uint64_t)InterruptHandler15);
        InstallInterruptHandler(16, (uint64_t)InterruptHandler16);
        InstallInterruptHandler(17, (uint64_t)InterruptHandler17);
        InstallInterruptHandler(18, (uint64_t)InterruptHandler18);
        InstallInterruptHandler(19, (uint64_t)InterruptHandler19);
        InstallInterruptHandler(20, (uint64_t)InterruptHandler20);
        InstallInterruptHandler(21, (uint64_t)InterruptHandler21);
        InstallInterruptHandler(22, (uint64_t)InterruptHandler22);
        InstallInterruptHandler(23, (uint64_t)InterruptHandler23);
        InstallInterruptHandler(24, (uint64_t)InterruptHandler24);
        InstallInterruptHandler(25, (uint64_t)InterruptHandler25);
        InstallInterruptHandler(26, (uint64_t)InterruptHandler26);
        InstallInterruptHandler(27, (uint64_t)InterruptHandler27);
        InstallInterruptHandler(28, (uint64_t)InterruptHandler28);
        InstallInterruptHandler(29, (uint64_t)InterruptHandler29);
        InstallInterruptHandler(30, (uint64_t)InterruptHandler30);
        InstallInterruptHandler(31, (uint64_t)InterruptHandler31);

        InstallIDT();

        EnableInterrupts();

        infoLogger.Log("Interrupt handler initialized!");
    }

    void InitAPIC(void* madt) {
        MADT* madtP = (MADT*)madt;

        // Save and allocate LAPIC address
        localAPIC = (uint32_t*)((uint64_t)madtP->localAPICAddress + KERNEL_VMA);
        MemoryManager::Virtual::AllocatePage(localAPIC, madtP->localAPICAddress, true);

        // Find all IOAPICs
        uint64_t entry = (uint64_t)&madtP->firstEntry;
        MADTEntry* entryPtr = (MADTEntry*)entry;
        numIRQ = 0;
        while (entry < (uint64_t)madtP + (madtP->length)) {
            if (entryPtr->type == 1) {
                IOAPICEntry* ioapic = (IOAPICEntry*)entryPtr;
                IOAPIC* newIOAPIC = (IOAPIC*)malloc(sizeof(IOAPIC));
                newIOAPIC->id = ioapic->ioAPICID;
                newIOAPIC->selectRegister = (uint32_t*)((uint64_t)ioapic->ioAPICAddress + KERNEL_VMA);
                newIOAPIC->dataRegister = (uint32_t*)((uint64_t)newIOAPIC->selectRegister + 0x10);

                // Allocate the register pages
                MemoryManager::Virtual::AllocatePage(newIOAPIC->selectRegister, ioapic->ioAPICAddress, true);

                newIOAPIC->irqBase = ioapic->gloablSystemInterruptBase;
                newIOAPIC->numIRQ = (ReadIOAPIC(newIOAPIC, 1) >> 16) & 0xF;

                newIOAPIC->next = ioapics;
                ioapics = newIOAPIC;

                if (newIOAPIC->irqBase >= numIRQ)
                    numIRQ = newIOAPIC->irqBase + newIOAPIC->numIRQ;

                // Map the IRQs
                for (int i = 0; i < newIOAPIC->numIRQ; i++) {
                    uint32_t IRQLowInfo = ReadIOAPIC(newIOAPIC, 0x10 + 2 * i);
                    uint32_t IRQHighInfo = ReadIOAPIC(newIOAPIC, 0x11 + 2 * i);
                    IRQLowInfo &= 0xFFFFF800;
                    IRQLowInfo |= (newIOAPIC->irqBase + i + 32) & 0xFF;
                    IRQHighInfo &= 0xF0FFFFFF;
                    IRQHighInfo |= (localAPIC[LAPIC_ID] & 0xF) << 24;
                    WriteIOAPIC(newIOAPIC, 0x10 + 2 * i, IRQLowInfo);
                    WriteIOAPIC(newIOAPIC, 0x11 + 2 * i, IRQHighInfo);

                    irqMapping[i + newIOAPIC->irqBase] = IRQLowInfo & 0xFF;
                }
            }

            entry += entryPtr->length;
            entryPtr = (MADTEntry*)entry;
        }

        for (int i = 0; i < numIRQ; i++)
            interruptHandlers[i] = nullptr;

        // Setup APIC ISRs
        InstallInterruptHandler(0xFF, (uint64_t)SpuriousISRHandler);

        // Initialize LAPIC
        localAPIC[LAPIC_DESTINATION_FORMAT] = 0xFFFFFFFF;
        localAPIC[LAPIC_LOGICAL_DESTINATION] &= 0x00FFFFFF;
        localAPIC[LAPIC_LOGICAL_DESTINATION] |= 1;

        localAPIC[LAPIC_TIMER_LVT] = APIC_DISABLE;
        localAPIC[LAPIC_PERFORMANCE_MONITOR_COUNTER_LVT] = APIC_NMI;
        localAPIC[LAPIC_LINT0_LVT] = APIC_DISABLE;
        localAPIC[LAPIC_LINT1_LVT] = APIC_DISABLE;
        localAPIC[LAPIC_TASK_PRIORITY] = 0;

        EnableAPIC();

        // Initialize the IOAPICs

        // Initialize LAPIC timer
        // localAPIC[LAPIC_SPURIOUS_INTERRUPT_VECTOR] = 0xFF + APIC_SW_ENABLE;
        // localAPIC[LAPIC_TIMER_DIV] = 3;
        // localAPIC[LAPIC_TIMER_LVT] = 0x20020;
        // localAPIC[LAPIC_TIMER_INITCNT] = 1000;
    }

    void SetExceptionHandler(int exception, bool (*exceptionHandler)(CPUState, StackState)) {
        if (exception < 0 || exception >= 32)
            return;

        exceptionHandlers[exception] = exceptionHandler;
    }

    void SetInterruptHandler(int interrupt, void (*interruptHandler)(void*)) {
        if (interrupt < 32 || interrupt > 255)
            return;

        InstallInterruptHandler(interrupt, (uint64_t)interruptHandler);
    }

    void ClearInterruptHandler(int interrupt) {
        if (interrupt < 32 || interrupt > 255)
            return;

        idt[interrupt].offset1 = 0;
        idt[interrupt].selector = 0;
        idt[interrupt].ist = 0;
        idt[interrupt].typeAttr = 0;
        idt[interrupt].offset2 = 0;
        idt[interrupt].offset3 = 0;
    }

    void StopInterrupts() { DisableInterrupts(); }

    uint8_t SetAvailableIRQ(void (*irqHandler)(), bool mask, bool levelTriggered, bool activeLow) {
        for (int i = 0; i < numIRQ; i++) {
            if (interruptHandlers[i] == nullptr) {
                SetIRQ(i, irqHandler, mask, levelTriggered, activeLow);
                return i;
            }
        }

        return 0xFF;
    }

    void SetIRQ(uint8_t irq, void (*irqHandler)(), bool mask, bool levelTriggered, bool activeLow) {
        if (irq > numIRQ)
            return;

        InstallInterruptHandler(irqMapping[irq], (uint64_t)irqHandler);
        interruptHandlers[irq] = (void (*)(void*))irqHandler;

        // Unmask appropriate IRQ
        IOAPIC* ioapic = ioapics;
        while (ioapic != nullptr) {
            if (ioapic->irqBase <= irq && ioapic->irqBase + ioapic->numIRQ > irq) {
                uint32_t irqLow = ReadIOAPIC(ioapic, 0x10 + (irq - ioapic->irqBase) * 2);

                irqLow &= (1 << 14) | 0xFF;
                irqLow = irqLow | (activeLow ? 1 << 13 : 0) | (levelTriggered ? 1 << 15 : 0) | (mask ? 1 << 16 : 0);

                WriteIOAPIC(ioapic, 0x10 + (irq - ioapic->irqBase) * 2, irqLow);
                break;
            } else
                ioapic = ioapic->next;
        }
    }

    void SetIRQMask(uint8_t irq, bool mask) {
        IOAPIC* ioapic = ioapics;
        while (ioapic != nullptr) {
            if (ioapic->irqBase <= irq && ioapic->irqBase + ioapic->numIRQ > irq) {
                uint32_t irqLow = ReadIOAPIC(ioapic, 0x10 + (irq - ioapic->irqBase) * 2);

                if (mask)
                    irqLow |= 1 << 16;
                else
                    irqLow &= ~(1 << 16);

                WriteIOAPIC(ioapic, 0x10 + (irq - ioapic->irqBase) * 2, irqLow);
                break;
            } else
                ioapic = ioapic->next;
        }
    }

    void WriteEOI() { localAPIC[LAPIC_EOI] = 0; }
} // namespace InterruptHandler
