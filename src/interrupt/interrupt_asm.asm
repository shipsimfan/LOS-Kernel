BITS 64

systemCallRSP: dq 0

GLOBAL SystemCall
EXTERN SystemCallHandler
SystemCall:
    ; RCX contains RIP, R11 contains RFLAGS
    mov rax, systemCallRSP
    mov [rax], rsp
    mov rsp, 0xFFFFFFFFFFFFFFF0

    push rcx
    push r11

    call SystemCallHandler

    pop r11
    pop rcx

    mov rdi, systemCallRSP
    mov rsp, [rdi]

    o64 sysret

idtr:
    .limit: dw  256 * 16 - 1
    .pointer: dq 0

GLOBAL InstallIDT
EXTERN idt
InstallIDT:
    push rax
    push rbx
    mov rbx, idtr.pointer
    mov rax, idt
    mov [rbx], rax

    mov rbx, idtr
    lidt [rbx]

    pop rbx
    pop rax

    ret

GLOBAL CLIFunc
CLIFunc:
    cli
    ret

GLOBAL STIFunc
STIFunc:
    sti
    ret

GLOBAL SpuriousISRHandler
SpuriousISRHandler:
    iretq

GLOBAL EnableAPIC
EnableAPIC:
    push rdx
    push rcx
    push rax

    mov ecx, 0x1B
    rdmsr

    xor edx, edx
    or eax, 1
    wrmsr

    pop rax
    pop rcx
    pop rdx

    ret

%macro no_error_code_interrupt_handler 1
GLOBAL InterruptHandler%1
InterruptHandler%1:
    push QWORD 0
    push QWORD %1
    jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
GLOBAL InterruptHandler%1
InterruptHandler%1:
    push QWORD %1
    jmp common_interrupt_handler
%endmacro

%macro irq_handler 1
GLOBAL IRQHandler%1
IRQHandler%1:
    push rdi
    mov rdi, %1
    call CommonIRQHandler
    pop rdi
    iretq
%endmacro

EXTERN CommonIRQHandler

EXTERN ExceptionHandler
common_interrupt_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rsp
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, cr2
    push rax

    call ExceptionHandler

    pop rax ;cr2
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx

    mov ax, ss
    cmp ax, 0
    je .end

    mov QWORD [rsp + 32], 0x08

    .end:
        pop rax
        add rsp, 16

    iretq

no_error_code_interrupt_handler 0
no_error_code_interrupt_handler 1
no_error_code_interrupt_handler 2
no_error_code_interrupt_handler 3
no_error_code_interrupt_handler 4
no_error_code_interrupt_handler 5
no_error_code_interrupt_handler 6
no_error_code_interrupt_handler 7
error_code_interrupt_handler 8
no_error_code_interrupt_handler 9
error_code_interrupt_handler 10
error_code_interrupt_handler 11
error_code_interrupt_handler 12
error_code_interrupt_handler 13
error_code_interrupt_handler 14
no_error_code_interrupt_handler 15
no_error_code_interrupt_handler 16
error_code_interrupt_handler 17
no_error_code_interrupt_handler 18
no_error_code_interrupt_handler 19
no_error_code_interrupt_handler 20
no_error_code_interrupt_handler 21
no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27
no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
error_code_interrupt_handler 30
no_error_code_interrupt_handler 31

irq_handler 0
irq_handler 1
irq_handler 2
irq_handler 3
irq_handler 4
irq_handler 5
irq_handler 6
irq_handler 7
irq_handler 8
irq_handler 9
irq_handler 10
irq_handler 11
irq_handler 12
irq_handler 13
irq_handler 14
irq_handler 15