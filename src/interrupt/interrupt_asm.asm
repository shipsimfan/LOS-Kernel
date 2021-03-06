GLOBAL InstallGDT
InstallGDT:
    lgdt [rdi]

    mov ds, si
    mov es, si
    mov fs, si
    mov gs, si
    mov ss, si

    ltr dx

    ret

GLOBAL InstallIDT
InstallIDT:
    lidt [rdi]

    ret

GLOBAL SpuriousIRQHandler
SpuriousIRQHandler:
    iretq

%macro no_error_code_interrupt_handler 1
GLOBAL ExceptionHandler%1
ExceptionHandler%1:
    push QWORD 0
    push QWORD %1
    jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
GLOBAL ExceptionHandler%1
ExceptionHandler%1:
    push QWORD %1
    jmp common_interrupt_handler
%endmacro

%macro irq_handler 1
GLOBAL IRQHandler%1
IRQHandler%1:
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

    mov rdi, %1
    call CommonIRQHandler

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
    pop rax

    iretq
%endmacro

EXTERN CommonIRQHandler

EXTERN CommonExceptionHandler
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

    call CommonExceptionHandler

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