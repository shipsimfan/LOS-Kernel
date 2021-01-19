#pragma once

#include <types.h>

namespace InterruptHandler {
#pragma pack(push)
#pragma pack(1)

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

    void Init();

    void SetExceptionHandler(int exception, bool (*exceptionHandler)(CPUState, StackState));
} // namespace InterruptHandler