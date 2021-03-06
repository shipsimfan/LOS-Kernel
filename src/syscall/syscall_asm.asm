EXTERN SystemCall
EXTERN currentProcess

SystemCallHandler:
    ; RCX contains RIP, R11 contains RFLAGS
    mov rbx, currentProcess
    mov rax, [rbx]
    add rax, 16
    mov [rax], rsp

    sub rax, 8
    mov rsp, [rax]

    push rcx
    push r11

    call SystemCall

    pop r11
    pop rcx

    mov rdi, currentProcess
    mov rsi, [rdi]
    add rsi, 16
    mov rsp, [rsi]

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