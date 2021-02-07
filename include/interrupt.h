#pragma once

#include <types.h>

namespace InterruptHandler {
#pragma pack(push)
#pragma pack(1)

    struct GDTDescr {
        uint16_t limitLow;
        uint16_t baseLow;
        uint8_t baseMid;
        uint8_t access : 1;
        uint8_t write : 1;
        uint8_t conforming : 1;
        uint8_t executable : 1;
        uint8_t code : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
        uint8_t limitHigh : 4;
        uint8_t zero : 1;
        uint8_t code64 : 1;
        uint8_t size : 1;
        uint8_t granularity : 1;
        uint8_t baseHigh;
    };

    struct TSS {
        uint32_t reserved0;
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t reserved1;
        uint64_t ist1;
        uint64_t ist2;
        uint64_t ist3;
        uint64_t ist4;
        uint64_t ist5;
        uint64_t ist6;
        uint64_t ist7;
        uint64_t reserved2;
        uint32_t reserved3;
    };

    struct IDTDescr {
        uint16_t offset1;  // Offset bits 0 .. 15
        uint16_t selector; // Code Segment Selector
        uint8_t ist;       // Bits 0 .. 2 hold Interrupt Stack Table offset, rest is zero
        uint8_t typeAttr;  // Type and Attributes
        uint16_t offset2;  // Offset bits 16 .. 31
        uint32_t offset3;  // Offset bits 32 .. 63
        uint32_t zero;     // Reserved
    };

    struct CPUState {
        uint64_t cr2;
        uint64_t r15;
        uint64_t r14;
        uint64_t r13;
        uint64_t r12;
        uint64_t r11;
        uint64_t r10;
        uint64_t r9;
        uint64_t r8;
        uint64_t rbp;
        uint64_t rsp;
        uint64_t rdi;
        uint64_t rsi;
        uint64_t rdx;
        uint64_t rcx;
        uint64_t rbx;
        uint64_t rax;
    };

    struct StackState {
        uint64_t interrupt;
        uint64_t errorCode;
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };

#pragma pack(pop)

    extern "C" IDTDescr idt[256];
    extern "C" GDTDescr gdt[7];

    void Init();
    bool InitAPIC(void* madt);

    void SetExceptionHandler(uint8_t exception, bool (*exceptionHandler)(CPUState, StackState));
    void SetInterruptHandler(uint8_t interrupt, void (*interruptHandler)());
    void ClearInterruptHandler(uint8_t interrupt);

    void SetIRQ(uint8_t irq, void (*irqHandler)());

    void SetTSS(uint64_t newRSP);

    void SetMask(uint8_t irq);
    void ClearMask(uint8_t irq);

    void EndInterrupt();
    void EndIRQ(uint8_t interrupt);

    void DisableInterrupts();
    void EnableInterrupts();
} // namespace InterruptHandler