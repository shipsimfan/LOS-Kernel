BITS 64

GLOBAL IRQ14
EXTERN IRQ14Handler
IRQ14:
    call IRQ14Handler
    iretq

GLOBAL IRQ15
EXTERN IRQ15Handler
IRQ15:
    call IRQ15Handler
    iretq