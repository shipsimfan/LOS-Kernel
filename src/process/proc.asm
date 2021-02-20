EXTERN currentProcess

; uint64_t ProperFork(rdi - Process* child)
GLOBAL ProperFork
ProperFork:
    ; Save parent stack
    mov r8, rsp

    ; Copy current stack to child stack
    ; rax <-- currentProcess->stack
    mov rsi, currentProcess
    mov rcx, [rsi]
    add rcx, 8
    mov rax, [rcx]

    ; rdx <-- child->stack
    add rdi, 8
    mov rdx, [rdi]

    ; rcx <-- count
    mov rcx, 32768

    .copy:
        mov rsi, [rax]
        mov [rdx], rsi

        sub rax, 8
        sub rdx, 8
        sub rcx, 8
        jnz .copy

    ; Switch stack
    mov rdx, [rdi]
    
    mov rsi, currentProcess
    mov rcx, [rsi]
    add rcx, 8
    mov rax, [rcx]

    sub rax, rsp
    sub rdx, rax

    mov [rdi], rdx
    mov rsp, rdx

    ; Modify return for child
    mov rax, .child
    push rax

    ; Save registers
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

    ; Save child stack pointer
    sub rdi, 8
    mov [rdi], rsp

    ; Switch back to parent stack
    mov rsp, r8

    ; Return for parent
    ret

    ; Return for child
    .child:
        xor rax, rax
        ret

GLOBAL TaskSwitch
TaskSwitch:
    ; Save registers
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

    ; Save stack pointer
    mov rax, currentProcess
    mov rbx, [rax]
    mov [rbx], rsp

    ; Switch current process
    mov [rax], rdi

    ; Retrieve stack pointer
    mov rsp, [rdi]

    ; Restore registers
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

    ; Return
    ret

GLOBAL SetStackPointer
SetStackPointer:
    pop rax
    mov rsp, rdi
    push rax
    ret

GLOBAL TaskExit
TaskExit:
    pop rax

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

    ret