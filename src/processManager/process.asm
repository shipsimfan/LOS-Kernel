BITS 64

GLOBAL SystemReturn
SystemReturn:
    mov rcx, rdi
    xor r11, r11
    
    mov rsp, 0x7FFFFFFFFFF0

    sti
    o64 sysret


    