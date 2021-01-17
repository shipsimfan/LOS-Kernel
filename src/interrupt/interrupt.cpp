#include <interrupt.h>

#include <logger.h>
#include <stdio.h>

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

namespace InterruptHandler {
    const char* exceptions[] = {"Divide by zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available", "Double fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment not present", "Stack-segmentation", "General protection", "Page", "", "x87 Floating-Point", "Alignment Check", "Machine Check", "SIMD Floating-Point", "Virtualization"};

    IDTDescr idt[256];

    bool (*exceptionHandlers[32])(CPUState, StackState);

    void InfoDump(CPUState cpu, StackState stack) {
        printf("rax: 0x%llx    rbx: 0x%llx    rcx: 0x%llx\n", cpu.rax, cpu.rbx, cpu.rcx);
        printf("rdx: 0x%llx    rsi: 0x%llx    rdi: 0x%llx\n", cpu.rdx, cpu.rsi, cpu.rdi);
        printf("rsp: 0x%llx    rbp: 0x%llx    r8:  0x%llx\n", cpu.rsp, cpu.rbp, cpu.r8);
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
            errorLogger.Log("Error code: %x", stack.errorCode);

            errorLogger.Log("Core dump:");
            InfoDump(cpu, stack);
        }

        while (1)
            ;
    }

    void SetupExceptionHandler(int interrupt, uint64_t offset) {
        idt[interrupt].offset1 = offset & 0xFFFF;
        idt[interrupt].offset2 = (offset >> 16) & 0xFFFF;
        idt[interrupt].offset3 = (offset >> 32) & 0xFFFFFFFF;

        idt[interrupt].selector = 0x8;
        idt[interrupt].typeAttr = 0b10001110;

        idt[interrupt].ist = 0;
        idt[interrupt].zero = 0;
    }

    void Init() {
        infoLogger.Log("Setting up exception handlers . . .");

        for (int i = 0; i < 32; i++)
            exceptionHandlers[i] = nullptr;

        SetupExceptionHandler(0, (uint64_t)InterruptHandler0);
        SetupExceptionHandler(1, (uint64_t)InterruptHandler1);
        SetupExceptionHandler(2, (uint64_t)InterruptHandler2);
        SetupExceptionHandler(3, (uint64_t)InterruptHandler3);
        SetupExceptionHandler(4, (uint64_t)InterruptHandler4);
        SetupExceptionHandler(5, (uint64_t)InterruptHandler5);
        SetupExceptionHandler(6, (uint64_t)InterruptHandler6);
        SetupExceptionHandler(7, (uint64_t)InterruptHandler7);
        SetupExceptionHandler(8, (uint64_t)InterruptHandler8);
        SetupExceptionHandler(9, (uint64_t)InterruptHandler9);
        SetupExceptionHandler(10, (uint64_t)InterruptHandler10);
        SetupExceptionHandler(11, (uint64_t)InterruptHandler11);
        SetupExceptionHandler(12, (uint64_t)InterruptHandler12);
        SetupExceptionHandler(13, (uint64_t)InterruptHandler13);
        SetupExceptionHandler(14, (uint64_t)InterruptHandler14);
        SetupExceptionHandler(15, (uint64_t)InterruptHandler15);
        SetupExceptionHandler(16, (uint64_t)InterruptHandler16);
        SetupExceptionHandler(17, (uint64_t)InterruptHandler17);
        SetupExceptionHandler(18, (uint64_t)InterruptHandler18);
        SetupExceptionHandler(19, (uint64_t)InterruptHandler19);
        SetupExceptionHandler(20, (uint64_t)InterruptHandler20);
        SetupExceptionHandler(21, (uint64_t)InterruptHandler21);
        SetupExceptionHandler(22, (uint64_t)InterruptHandler22);
        SetupExceptionHandler(23, (uint64_t)InterruptHandler23);
        SetupExceptionHandler(24, (uint64_t)InterruptHandler24);
        SetupExceptionHandler(25, (uint64_t)InterruptHandler25);
        SetupExceptionHandler(26, (uint64_t)InterruptHandler26);
        SetupExceptionHandler(27, (uint64_t)InterruptHandler27);
        SetupExceptionHandler(28, (uint64_t)InterruptHandler28);
        SetupExceptionHandler(29, (uint64_t)InterruptHandler29);
        SetupExceptionHandler(30, (uint64_t)InterruptHandler30);
        SetupExceptionHandler(31, (uint64_t)InterruptHandler31);

        infoLogger.Log("Installing idt . . .");
        InstallIDT();

        infoLogger.Log("Enabling interrupts . . .");
        EnableInterrupts();

        infoLogger.Log("Interrupt handler initialized!");
    }

    void SetExceptionHandler(int exception, bool (*exceptionHandler)(CPUState, StackState)) {
        if (exception < 0 || exception >= 32)
            return;

        exceptionHandlers[exception] = exceptionHandler;
    }
} // namespace InterruptHandler
