BITS 64

EXTERN currentProcess

; Leave the current task and start the new task
GLOBAL TaskEnter
TaskEnter:
    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Save rsp
    mov rax, currentProcess
    mov rbx, [rax]
    mov [rbx], rsp

    ; Switch process
    mov rbx, currentProcess
    mov [rbx], rsi

    ; Switch stack to userspace stack
    mov rsp, 0x7FFFFFFFFFF0

    ; Set registers
    mov rcx, rdi
    mov r11, (1 << 9)

    ; Enter the new process    
    o64 sysret

; Leave this now empty task and return back
GLOBAL TaskExit
TaskExit:
    ; Switch stack
    mov rbx, currentProcess
    mov rax, [rbx]
    mov rsp, [rax]

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
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Enter
    ret

GLOBAL TaskSwitch
EXTERN Schedule
TaskSwitch:
    ; LEAVE CURRENT TASK

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Save rsp
    mov rax, currentProcess
    mov rbx, [rax]
    mov [rbx], rsp

    ; SCHEDULE
    call Schedule

    ; ENTER NEXT TASK
    ; Switch stack
    mov rbx, currentProcess
    mov rax, [rbx]
    mov rsp, [rax]

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
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Enter
    ret
