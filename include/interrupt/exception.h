#pragma once

#include <stdint.h>

namespace Interrupt {
    struct Registers {
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

    struct ExceptionInfo {
        uint64_t interrupt;
        uint64_t errorCode;
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };

    typedef void (*ExceptionHandler)(Registers, ExceptionInfo);

    enum ExceptionType { DIVIDE_BY_ZERO = 0, DEBUG_EXCEPTION, NON_MASKABLE_INTERRUPT, BREAKPOINT, OVERFLOW, BOUND_RANGE_EXCEEDED, INVALID_OPCODE, DEVICE_NOT_AVAILABLE, DOUBLE_FAULT, INVALID_TSS = 9, SEGMENT_NOT_PRESENT, STACK_SEGEMENT_FAULT, GENERAL_PROTECTION_FAULT, PAGE_FAULT, X87_FLOATING_POINT_EXCEPTION = 16, ALIGNMENT_CHECK, MACHINE_CHECK, SIMD_FLOATING_POINT_EXCEPTION, VIRTUALIZATION_EXCEPTION, SECURITY_EXCEPTION = 30 };

    // Returns false if the exception is already taken
    bool InstallExceptionHandler(ExceptionType exception, ExceptionHandler handler);

    void RemoveExceptionHandler(ExceptionType exception);
} // namespace Interrupt