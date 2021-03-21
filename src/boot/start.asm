SECTION .text.low

GLOBAL _start
_start:
    cli
    mov rax, 0xDEADBEEF

    .hang:
        hlt
        jmp .hang