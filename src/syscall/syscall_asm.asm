SystemCallHandler:
    o64 sysret

GLOBAL InitSystemCalls
InitSystemCalls:
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1
    wrmsr

    xor eax, eax
    mov edx, (0x10 << 16) | 0x08
    mov ecx, 0xC0000081
    wrmsr

    mov ecx, 0xC0000082
    mov rdx, SystemCallHandler
    mov rax, rdx
    shr rdx, 32
    wrmsr
    
    ret