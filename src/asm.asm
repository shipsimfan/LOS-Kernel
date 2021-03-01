GLOBAL CompareExchange
CompareExchange:
    mov rax, rsi
    lock cmpxchg [rdi], rdx
    jz .true

    xor rax, rax
    ret

    .true:
        mov rax, 1
        ret

GLOBAL Increament
Increament:
    inc QWORD [rdi]
    ret