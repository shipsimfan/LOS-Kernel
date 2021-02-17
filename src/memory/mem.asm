GLOBAL SetCurrentPML4
SetCurrentPML4:
    mov cr3, rdi
    ret