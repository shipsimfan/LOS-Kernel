;  LOS Kernel
;  start.asm
;
;  The entry point from the bootloader

SECTION .data.low

;======================================
; PAGE TABLES
;======================================
ALIGN 4096
pml4: times 512 dq 0

SECTION .bss

;======================================
; STACK
;======================================
stackBottom: resb 32768
GLOBAL stackTop
stackTop:

;======================================
; BOOTLOADER ARGUMENTS
;======================================
GLOBAL gopInfo
gopInfo: resq 1

GLOBAL mmap
mmap: resq 1

GLOBAL rsdp
rsdp: resq 1

ctorsAddr: resq 1
ctorsSize: resq 1

SECTION .text.low

;======================================
; BOOT ENTRY POINT
;======================================
GLOBAL _start

EXTERN kmain

EXTERN InitConsole
EXTERN InitDoubleBuffering
EXTERN InitExceptions
EXTERN InitPhysicalMemory
EXTERN InitVirtualMemory
EXTERN InitHeap
EXTERN InitACPITables
EXTERN InitIRQ
EXTERN InitSystemTimer

EXTERN __dso_handle

_start:
    ; Disable interrupts
    cli

    ; Save .ctors
    mov r14, rcx
    mov r15, r8

    ; Copy page structures from UEFI
    mov rbx, cr3
    mov r8, pml4
    mov rcx, 256

    .copyLow:
        mov rax, [rbx]
        mov [r8], rax
        add rbx, 8
        add r8, 8
        loop .copyLow

    ; Copy page structures into higher half
    mov rbx, cr3
    mov rcx, 256
    
    .copyHigh:
        mov rax, [rbx]
        mov [r8], rax
        add rbx, 8
        add r8, 8
        loop .copyHigh

    ; Set page structures
    mov rax, pml4
    mov cr3, rax

    mov rax, higherHalf
    jmp rax

SECTION .text

higherHalf:
    ; Setup the stack    
    mov rsp, stackTop

    ; Move arguments to higher half
    mov rax, 0xFFFF800000000000
    add rdi, rax
    add rsi, rax

    ; Save arguments
    mov rbx, mmap
    mov [rbx], rdi

    mov rbx, gopInfo
    mov [rbx], rsi

    mov rbx, rsdp
    mov [rbx], rdx

    mov rbx, ctorsAddr
    mov [rbx], r14
    
    mov rbx, ctorsSize
    mov [rbx], r15

    ; Init __dso_handle
    mov rax, __dso_handle
    mov QWORD [rax], 0

    ; Initialize the console
    mov rax, InitConsole
    call rax

    ; Initialize the exception handler
    mov rax, InitExceptions
    call rax

    ; Initalize memory
    mov rax, InitPhysicalMemory
    call rax

    mov rax, InitVirtualMemory
    call rax

    mov rax, InitHeap
    call rax

    ; Initialize double buffer
    mov rax, InitDoubleBuffering
    call rax

    ; Load ACPI tables
    mov rax, InitACPITables
    call rax

    ; Initialize IRQs
    mov rax, InitIRQ
    call rax

    ; Start the system timer
    mov rax, InitSystemTimer
    call rax

    ; Call global initializers
    mov rbx, ctorsAddr
    mov rax, [rbx]
    cmp rax, 0
    je .afterConstructors

    mov rbx, ctorsSize
    mov r15, [rbx]
    
    .loop:
        cmp r15, 0
        je .afterConstructors

        push rax
        push r15

        mov rbx, [rax]
        call rbx

        pop r15
        pop rax

        sub r15, 8
        add rax, 8

        jmp .loop

    .afterConstructors:

    ; Call kernel main
    mov rax, kmain
    call rax