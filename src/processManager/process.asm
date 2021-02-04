BITS 64

GLOBAL SystemReturn
SystemReturn:
    mov rcx, rdi
    mov r11, (1 << 9)
    mov rsp, 0x7FFFFFFFFFF0
    
    o64 sysret
