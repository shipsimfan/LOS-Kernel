GLOBAL CompareExchange
CompareExchange:
    mov rax, rsi
    cmpxchg [rdi], rdx
    jnz .true

    xor rax, rax
    ret

    .true:
        mov rax, 1
        ret