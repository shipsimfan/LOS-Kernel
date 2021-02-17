GLOBAL SetCurrentPML4
SetCurrentPML4:
    mov cr3, rdi
    ret

GLOBAL GetCR2
GetCR2:
    mov rax, cr2
    ret