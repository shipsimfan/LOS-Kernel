BITS 64

GLOBAL GetCurrentPML4
GetCurrentPML4:
    mov rax, cr3
    ret

GLOBAL SetCurrentPML4
SetCurrentPML4:
    mov cr3, rdi
    ret