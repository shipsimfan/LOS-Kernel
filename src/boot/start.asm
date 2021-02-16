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
stackTop:

GLOBAL gopInfo
gopInfo: resq 1

GLOBAL mmap
mmap: resq 1

GLOBAL rdsp
rdsp: resq 1

SECTION .text.low

;======================================
; BOOT ENTRY POINT
;======================================
GLOBAL _start

EXTERN kmain

EXTERN InitConsole
EXTERN InitExceptions
EXTERN InitPhysicalMemory
EXTERN InitVirtualMemory
EXTERN InitHeap

_start:
    ; Disable interrupts
    cli

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
    add rdx, rax

    ; Save arguments
    mov rbx, mmap
    mov [rbx], rdi

    mov rbx, gopInfo
    mov [rbx], rsi

    mov rbx, rdsp
    mov [rbx], rdx

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

    ; Call kernel main
    mov rax, kmain
    call rax