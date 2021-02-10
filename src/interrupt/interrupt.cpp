#include <interrupt.h>

#include <console.h>
#include <dev.h>
#include <kernel/keyboard.h>
#include <kernel/time.h>
#include <logger.h>
#include <mem/defs.h>
#include <mem/virtual.h>
#include <proc.h>
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

#define NUM_INTERRUPTS 256
#define NUM_EXCEPTIONS 32
#define NUM_PIC_IRQ 16
#define IRQ_BASE NUM_EXCEPTIONS
#define SPURIOUS_IVT 0xFF

#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_COMMAND 0xA0
#define SLAVE_PIC_DATA 0xA1

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

extern "C" void IRQHandler0();
extern "C" void IRQHandler1();
extern "C" void IRQHandler2();
extern "C" void IRQHandler3();
extern "C" void IRQHandler4();
extern "C" void IRQHandler5();
extern "C" void IRQHandler6();
extern "C" void IRQHandler7();
extern "C" void IRQHandler8();
extern "C" void IRQHandler9();
extern "C" void IRQHandler10();
extern "C" void IRQHandler11();
extern "C" void IRQHandler12();
extern "C" void IRQHandler13();
extern "C" void IRQHandler14();
extern "C" void IRQHandler15();

extern "C" void InstallIDT();
extern "C" void InstallGDT();

extern "C" void CLIFunc();
extern "C" void STIFunc();

extern "C" void SpuriousISRHandler();
extern "C" void TimerISRHandler();

extern "C" void SystemCall();

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

    enum InterruptType { UNUSED, EXCEPTION, IRQ, SPURIOUS };

    const char* exceptions[] = {"Divide by zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available", "Double fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment not present", "Stack-segmentation", "General protection", "Page", "", "x87 Floating-Point", "Alignment Check", "Machine Check", "SIMD Floating-Point", "Virtualization"};

    bool (*exceptionHandlers[NUM_EXCEPTIONS])(CPUState, StackState);
    void (*irqHandlers[NUM_PIC_IRQ])();

    InterruptType interrupts[NUM_INTERRUPTS];

    GDTDescr gdt[7];
    TSS tss;

    IDTDescr idt[NUM_INTERRUPTS];
    uint32_t* localAPIC;

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

    extern "C" void CommonIRQHandler(uint64_t irqNumber) {
        if (irqHandlers[irqNumber] != nullptr)
            irqHandlers[irqNumber]();
        else
            warningLogger.Log("IRQ Number: %i", irqNumber);

        EndIRQ(irqNumber);
    }

    extern "C" uint64_t SystemCallHandler(uint64_t num, uint64_t arg1, uint64_t arg2) {
        switch (num) {
        case 0:
            ProcessManager::Exit(arg1);
            errorLogger.Log("We shouldn't be here!");
            while (1)
                ;

        case 1:
            if (arg1 >= KERNEL_VMA)
                break;

            return Console::DisplayString((const char*)arg1);

        case 2:
            if (arg1 >= KERNEL_VMA)
                break;

            return ReadKey((char*)arg1, arg2);

        case 3:
            if (arg1 >= KERNEL_VMA)
                break;

            return ProcessManager::Execute((const char*)arg1);

        case 4:
            if (arg2 >= KERNEL_VMA)
                break;

            ProcessManager::WaitPID(arg1, (uint64_t*)arg2);
            break;

        case 5:
            return printf("%#llX", arg1);

        default:
            warningLogger.Log("System call number: %#llx", num);
        }

        return 0;
    }

    void InstallInterruptHandler(uint8_t interrupt, uint64_t offset) {
        idt[interrupt].offset1 = offset & 0xFFFF;
        idt[interrupt].offset2 = (offset >> 16) & 0xFFFF;
        idt[interrupt].offset3 = (offset >> 32) & 0xFFFFFFFF;

        idt[interrupt].selector = 0x8;
        idt[interrupt].typeAttr = 0b11101110;

        idt[interrupt].ist = 0;
        idt[interrupt].zero = 0;
    }

    void Init() {
        infoLogger.Log("Initializing interrupt handler . . .");

        uint32_t low = (uint64_t)SystemCall & 0xFFFFFFFF;
        uint32_t high = (uint64_t)SystemCall >> 32;
        asm volatile("wrmsr" : : "c"(0xC0000082), "a"(low), "d"(high));

        // Setup GDT
        // NULL descriptor
        gdt[0].limitLow = 0;
        gdt[0].baseLow = 0;
        gdt[0].baseMid = 0;
        gdt[0].access = 0;
        gdt[0].write = 0;
        gdt[0].conforming = 0;
        gdt[0].executable = 0;
        gdt[0].code = 0;
        gdt[0].dpl = 0;
        gdt[0].present = 0;
        gdt[0].limitHigh = 0;
        gdt[0].zero = 0;
        gdt[0].code64 = 0;
        gdt[0].size = 0;
        gdt[0].granularity = 0;
        gdt[0].baseHigh = 0;

        // Code 0 descriptor
        gdt[1].limitLow = 0xFFFF;
        gdt[1].baseLow = 0;
        gdt[1].baseMid = 0;
        gdt[1].access = 0;
        gdt[1].write = 1;
        gdt[1].conforming = 0;
        gdt[1].executable = 1;
        gdt[1].code = 1;
        gdt[1].dpl = 0;
        gdt[1].present = 1;
        gdt[1].limitHigh = 0xF;
        gdt[1].zero = 0;
        gdt[1].code64 = 1;
        gdt[1].size = 0;
        gdt[1].granularity = 1;
        gdt[1].baseHigh = 0;

        // Data 0 descriptor
        gdt[2].limitLow = 0xFFFF;
        gdt[2].baseLow = 0;
        gdt[2].baseMid = 0;
        gdt[2].access = 0;
        gdt[2].write = 1;
        gdt[2].conforming = 0;
        gdt[2].executable = 0;
        gdt[2].code = 1;
        gdt[2].dpl = 0;
        gdt[2].present = 1;
        gdt[2].limitHigh = 0xF;
        gdt[2].zero = 0;
        gdt[2].code64 = 0;
        gdt[2].size = 1;
        gdt[2].granularity = 1;
        gdt[2].baseHigh = 0;

        // Code 3 descriptor
        gdt[4].limitLow = 0xFFFF;
        gdt[4].baseLow = 0;
        gdt[4].baseMid = 0;
        gdt[4].access = 0;
        gdt[4].write = 1;
        gdt[4].conforming = 0;
        gdt[4].executable = 1;
        gdt[4].code = 1;
        gdt[4].dpl = 3;
        gdt[4].present = 1;
        gdt[4].limitHigh = 0xF;
        gdt[4].zero = 0;
        gdt[4].code64 = 1;
        gdt[4].size = 0;
        gdt[4].granularity = 1;
        gdt[4].baseHigh = 0;

        // Data 3 descriptor
        gdt[3].limitLow = 0xFFFF;
        gdt[3].baseLow = 0;
        gdt[3].baseMid = 0;
        gdt[3].access = 0;
        gdt[3].write = 1;
        gdt[3].conforming = 0;
        gdt[3].executable = 0;
        gdt[3].code = 1;
        gdt[3].dpl = 3;
        gdt[3].present = 1;
        gdt[3].limitHigh = 0xF;
        gdt[3].zero = 0;
        gdt[3].code64 = 0;
        gdt[3].size = 1;
        gdt[3].granularity = 1;
        gdt[3].baseHigh = 0;

        // TSS descriptor
        uint64_t tssBase = (uint64_t)(&tss);
        gdt[5].limitLow = sizeof(TSS);
        gdt[5].baseLow = tssBase & 0xFFFF;
        gdt[5].baseMid = (tssBase >> 16) & 0xFF;
        gdt[5].access = 1;
        gdt[5].write = 0;
        gdt[5].conforming = 0;
        gdt[5].executable = 1;
        gdt[5].code = 0;
        gdt[5].dpl = 3;
        gdt[5].present = 1;
        gdt[5].limitHigh = 0;
        gdt[5].zero = 0;
        gdt[5].code64 = 0;
        gdt[5].size = 0;
        gdt[5].granularity = 1;
        gdt[5].baseHigh = (tssBase >> 24) & 0xFF;
        gdt[6].limitLow = (tssBase >> 32) & 0xFFFF;
        gdt[6].baseLow = (tssBase >> 48) & 0xFFFF;
        gdt[6].baseMid = 0;
        gdt[6].access = 0;
        gdt[6].write = 0;
        gdt[6].conforming = 0;
        gdt[6].executable = 0;
        gdt[6].code = 0;
        gdt[6].dpl = 0;
        gdt[6].present = 0;
        gdt[6].limitHigh = 0;
        gdt[6].zero = 0;
        gdt[6].code64 = 0;
        gdt[6].size = 0;
        gdt[6].granularity = 0;
        gdt[6].baseHigh = 0;

        // TSS
        tss.reserved0 = 0;
        tss.rsp0 = 0xFFFFFFFFFFFFFFF0;
        tss.rsp1 = 0;
        tss.rsp2 = 0;
        tss.reserved1 = 0;
        tss.ist1 = 0;
        tss.ist2 = 0;
        tss.ist3 = 0;
        tss.ist4 = 0;
        tss.ist5 = 0;
        tss.ist6 = 0;
        tss.ist7 = 0;
        tss.reserved2 = 0;
        tss.reserved3 = 0;

        InstallGDT();

        // Setup Interrupts
        for (int i = 0; i < 256; i++)
            interrupts[i] = InterruptType::UNUSED;

        for (int i = 0; i < 32; i++) {
            exceptionHandlers[i] = nullptr;
            interrupts[i] = InterruptType::EXCEPTION;
        }

        for (int i = 0; i < NUM_PIC_IRQ; i++) {
            irqHandlers[i] = nullptr;
            interrupts[i + IRQ_BASE] = IRQ;
        }

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

    bool InitAPIC(void* madtv) {
        MADT* madt = (MADT*)madtv;

        // Save and allocate LAPIC address
        localAPIC = (uint32_t*)((uint64_t)madt->localAPICAddress + KERNEL_VMA);
        MemoryManager::Virtual::AllocatePage(localAPIC, madt->localAPICAddress, true);

        // Initialize the Local APIC
        interrupts[0xFF] = InterruptType::SPURIOUS;
        InstallInterruptHandler(0xFF, (uint64_t)SpuriousISRHandler);
        localAPIC[LAPIC_SPURIOUS_INTERRUPT_VECTOR] = 0x1FF;
        localAPIC[LAPIC_TASK_PRIORITY] = 0;

        // Verify 8259 PICs
        if ((madt->flags & 1) == 0) {
            errorLogger.Log("No 8259 PICs installed!");
            return false;
        }

        // Find and mask all IO-APICS
        MADTEntry* entry = &madt->firstEntry;
        uint64_t entryI = (uint64_t)entry;
        while (entryI < (uint64_t)madt + madt->length) {
            if (entry->type == 1) {
                IOAPICEntry* ioapic = (IOAPICEntry*)entry;
                uint32_t* selectReg = (uint32_t*)((uint64_t)ioapic->ioAPICAddress + KERNEL_VMA);
                uint32_t* dataReg = (uint32_t*)((uint64_t)selectReg + 0x10);

                *selectReg = 1; // Select IOAPICVER
                uint8_t numIRQ = ((*dataReg) >> 16) + 1;
                for (int i = 0; i < numIRQ; i++) {
                    *selectReg = 0x10 + 2 * i;
                    *dataReg = APIC_DISABLE;
                }
            }

            entryI += entry->length;
            entry = (MADTEntry*)entryI;
        }

        // Initialize the PICs
        uint8_t masterMask, slaveMask;
        masterMask = DeviceManager::inb(MASTER_PIC_DATA);
        slaveMask = DeviceManager::inb(SLAVE_PIC_DATA);

        DeviceManager::outb(MASTER_PIC_COMMAND, 0x11);
        DeviceManager::IOWait();
        DeviceManager::outb(SLAVE_PIC_COMMAND, 0x11);
        DeviceManager::IOWait();
        DeviceManager::outb(MASTER_PIC_DATA, IRQ_BASE);
        DeviceManager::IOWait();
        DeviceManager::outb(SLAVE_PIC_DATA, IRQ_BASE + 8);
        DeviceManager::IOWait();
        DeviceManager::outb(MASTER_PIC_DATA, 4);
        DeviceManager::IOWait();
        DeviceManager::outb(SLAVE_PIC_DATA, 2);
        DeviceManager::IOWait();

        DeviceManager::outb(MASTER_PIC_DATA, 0x01);
        DeviceManager::IOWait();
        DeviceManager::outb(SLAVE_PIC_DATA, 0x01);
        DeviceManager::IOWait();

        DeviceManager::outb(MASTER_PIC_DATA, masterMask);
        DeviceManager::outb(SLAVE_PIC_DATA, slaveMask);

        // Install interrupt handlers
        InstallInterruptHandler(IRQ_BASE + 0, (uint64_t)IRQHandler0);
        InstallInterruptHandler(IRQ_BASE + 1, (uint64_t)IRQHandler1);
        InstallInterruptHandler(IRQ_BASE + 2, (uint64_t)IRQHandler2);
        InstallInterruptHandler(IRQ_BASE + 3, (uint64_t)IRQHandler3);
        InstallInterruptHandler(IRQ_BASE + 4, (uint64_t)IRQHandler4);
        InstallInterruptHandler(IRQ_BASE + 5, (uint64_t)IRQHandler5);
        InstallInterruptHandler(IRQ_BASE + 6, (uint64_t)IRQHandler6);
        InstallInterruptHandler(IRQ_BASE + 7, (uint64_t)IRQHandler7);
        InstallInterruptHandler(IRQ_BASE + 8, (uint64_t)IRQHandler8);
        InstallInterruptHandler(IRQ_BASE + 9, (uint64_t)IRQHandler9);
        InstallInterruptHandler(IRQ_BASE + 10, (uint64_t)IRQHandler10);
        InstallInterruptHandler(IRQ_BASE + 11, (uint64_t)IRQHandler11);
        InstallInterruptHandler(IRQ_BASE + 12, (uint64_t)IRQHandler12);
        InstallInterruptHandler(IRQ_BASE + 13, (uint64_t)IRQHandler13);
        InstallInterruptHandler(IRQ_BASE + 14, (uint64_t)IRQHandler14);
        InstallInterruptHandler(IRQ_BASE + 15, (uint64_t)IRQHandler15);

        // Unmask all irqs
        for (int i = 0; i < NUM_PIC_IRQ; i++)
            ClearMask(i);

        return true;
    }

    void InitPreemptTimer(void (*handler)()) {
        // Set the interrupt
        localAPIC[LAPIC_TIMER_LVT] = APIC_DISABLE;
        InstallInterruptHandler(PREEMPT_TIMER_INTERRUPT, (uint64_t)handler);

        // Set the divider
        localAPIC[LAPIC_TIMER_DIV] = 0x3;

        // Wait till the start of the millisecond
        uint64_t start = getCurrentTimeMillis() + 1;
        while (getCurrentTimeMillis() < start)
            ;

        localAPIC[LAPIC_TIMER_LVT] = PREEMPT_TIMER_INTERRUPT;
        localAPIC[LAPIC_TIMER_INITCNT] = 0xFFFFFFFF;

        while (getCurrentTimeMillis() < start + 10)
            ;

        localAPIC[LAPIC_TIMER_LVT] = APIC_DISABLE;

        uint32_t ticksIn10ms = 0xFFFFFFFF - localAPIC[LAPIC_TIMER_CURCNT];

        localAPIC[LAPIC_TIMER_LVT] = PREEMPT_TIMER_INTERRUPT | 0x20000;
        localAPIC[LAPIC_TIMER_INITCNT] = ticksIn10ms;
    }

    void SetExceptionHandler(uint8_t exception, bool (*exceptionHandler)(CPUState, StackState)) {
        if (exception >= NUM_EXCEPTIONS)
            return;

        exceptionHandlers[exception] = exceptionHandler;
    }

    void SetInterruptHandler(uint8_t interrupt, void (*interruptHandler)()) {
        if (interrupt < NUM_EXCEPTIONS || interrupt >= NUM_INTERRUPTS)
            return;

        InstallInterruptHandler(interrupt, (uint64_t)interruptHandler);
    }

    void ClearInterruptHandler(uint8_t interrupt) {
        if (interrupt < NUM_EXCEPTIONS || interrupt >= NUM_INTERRUPTS)
            return;

        idt[interrupt].offset1 = 0;
        idt[interrupt].selector = 0;
        idt[interrupt].ist = 0;
        idt[interrupt].typeAttr = 0;
        idt[interrupt].offset2 = 0;
        idt[interrupt].offset3 = 0;
    }

    void DisableInterrupts() { CLIFunc(); }
    void EnableInterrupts() { STIFunc(); }

    void SetIRQ(uint8_t irq, void (*irqHandler)()) {
        if (irq >= NUM_PIC_IRQ)
            return;

        irqHandlers[irq] = irqHandler;
    }

    void SetTSS(uint64_t newRSP) { tss.rsp0 = newRSP; }

    void SetMask(uint8_t irq) {
        uint16_t port = MASTER_PIC_DATA;
        if (irq >= 8) {
            port = SLAVE_PIC_DATA;
            irq -= 8;
        }

        uint8_t value = DeviceManager::inb(port) | (1 << irq);
        DeviceManager::outb(port, value);
    }

    void ClearMask(uint8_t irq) {
        uint16_t port = MASTER_PIC_DATA;
        if (irq >= 8) {
            port = SLAVE_PIC_DATA;
            irq -= 8;
        }

        uint8_t value = DeviceManager::inb(port) & ~(1 << irq);
        DeviceManager::outb(port, value);
    }

    void EndInterrupt() { localAPIC[LAPIC_EOI] = 0; }

    void EndIRQ(uint8_t irq) {
        EndInterrupt();

        if (irq >= 8)
            DeviceManager::outb(SLAVE_PIC_COMMAND, 0x20);

        DeviceManager::outb(MASTER_PIC_COMMAND, 0x20);
    }
} // namespace InterruptHandler
