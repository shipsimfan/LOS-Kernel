GLOBAL CompareExchange
CompareExchange:
    mov rax, rsi
    cmpxchg [rdi], rdx
    jz .true

    xor rax, rax
    ret

    .true:
        mov rax, 1
        ret