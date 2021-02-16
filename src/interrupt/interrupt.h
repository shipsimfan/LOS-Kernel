#pragma once

#include <stdint.h>

#define NUM_EXCEPTIONS 32
#define NUM_IRQ 16
#define NUM_INTERRUPT_VECTOR 256

#define NUM_GDT_ENTRY 7

#define GDT_NULL 0x0
#define GDT_CODE0 0x8
#define GDT_DATA0 0x10
#define GDT_DATA3 0x18
#define GDT_CODE3 0x20
#define GDT_TSS 0x28

#define GDT_NULL_IDX 0
#define GDT_CODE0_IDX 1
#define GDT_DATA0_IDX 2
#define GDT_DATA3_IDX 3
#define GDT_CODE3_IDX 4
#define GDT_TSS_IDX 5

#define SPURIOUS_INTERRUPT_VECTOR 0xFF

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

struct CPUPointer {
    uint16_t limit;
    void* pointer;
};

#pragma pack(pop)

extern "C" void ExceptionHandler0();
extern "C" void ExceptionHandler1();
extern "C" void ExceptionHandler2();
extern "C" void ExceptionHandler3();
extern "C" void ExceptionHandler4();
extern "C" void ExceptionHandler5();
extern "C" void ExceptionHandler6();
extern "C" void ExceptionHandler7();
extern "C" void ExceptionHandler8();
extern "C" void ExceptionHandler9();
extern "C" void ExceptionHandler10();
extern "C" void ExceptionHandler11();
extern "C" void ExceptionHandler12();
extern "C" void ExceptionHandler13();
extern "C" void ExceptionHandler14();
extern "C" void ExceptionHandler15();
extern "C" void ExceptionHandler16();
extern "C" void ExceptionHandler17();
extern "C" void ExceptionHandler18();
extern "C" void ExceptionHandler19();
extern "C" void ExceptionHandler20();
extern "C" void ExceptionHandler21();
extern "C" void ExceptionHandler22();
extern "C" void ExceptionHandler23();
extern "C" void ExceptionHandler24();
extern "C" void ExceptionHandler25();
extern "C" void ExceptionHandler26();
extern "C" void ExceptionHandler27();
extern "C" void ExceptionHandler28();
extern "C" void ExceptionHandler29();
extern "C" void ExceptionHandler30();
extern "C" void ExceptionHandler31();

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

extern "C" void SpuriousIRQHandler();

extern "C" void InstallIDT(CPUPointer* idtr);
extern "C" void InstallGDT(CPUPointer* gdtr, uint16_t data0, uint16_t tss);
