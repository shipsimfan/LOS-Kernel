BITS 64
SECTION .low_text

kernel_vma: equ 0xFFFF800000000000

;======================================
; GDT
;======================================
GDT64:
    .null:  equ $ - GDT64
        dq 0
    .code0:  equ $ - GDT64
        dw 0xFFFF       ; Limit 0:15
        dw 0x0000       ; Base 0:15
        db 0x00         ; Base 16:23
        db 0b10011010   ; Access: Present(1) - Ring 0 (00) - Code or Data(1) - Executable(1) - Conforming(0) - Read/Write(1) - Access(0)
        db 0b10101111   ; Flags and Limit 16:19
        db 0x00         ; Base 24:31
    .data0:  equ $ - GDT64
        dw 0xFFFF       ; Limit 0:15
        dw 0x0000       ; Base 0:15
        db 0x00         ; Base 16:23
        db 0b10010010   ; Access: Present(1) - Ring 0 (00) - Code or Data(1) - Executable(0) - Conforming(0) - Read/Write(1) - Access(0)
        db 0b11001111   ; Flags and Limit 16:19
        db 0x00         ; Base 24:31
    .code3:  equ $ - GDT64
        dw 0xFFFF       ; Limit 0:15
        dw 0x0000       ; Base 0:15
        db 0x00         ; Base 16:23
        db 0b11111010   ; Access: Present(1) - Ring 3 (11) - Code or Data(1) - Executable(1) - Conforming(0) - Read/Write(1) - Access(0)
        db 0b10101111   ; Flags and Limit 16:19
        db 0x00         ; Base 24:31
    .data3:  equ $ - GDT64
        dw 0xFFFF       ; Limit 0:15
        dw 0x0000       ; Base 0:15
        db 0x00         ; Base 16:23
        db 0b11110010   ; Access: Present(1) - Ring 3 (11) - Code or Data(1) - Executable(0) - Conforming(0) - Read/Write(1) - Access(0)
        db 0b11001111   ; Flags and Limit 16:19
        db 0x00         ; Base 24:31
    .tss:   equ $ - GDT64
        dw 0xFFFF       ; Limit 0:15
        dw 0x0000       ; Base 0:15
        db 0x00         ; Base 16:23
        db 0b10001001   ; Present(1) - Ring 0 (00) - 0 - Type (1001)
        db 0b10001111   ; Granularity(1) - 00 - Availability (0) - Limit 16:19
        db 0x00         ; Base 24:31
        dd 0x00000000   ; Base 63:32
        db 0x00         ; Reserved
        db 0x00         ; 0
        dw 0x0000       ; Reserved
    .pointer:
        dw $ - GDT64 - 1
        dq GDT64 + kernel_vma

mmap: dq 0
gmode: dq 0
rdsp: dq 0

ALIGN 4096
pml4: times 512 dq 0

;======================================
; BOOT ENTRY POINT
;======================================
GLOBAL _start
EXTERN kmain
_start:
    cli

    mov [mmap], rdi
    mov [gmode], rsi
    mov [rdsp], rdx

    mov rbx, cr3
    mov rsi, pml4
    mov rcx, 256

    .copyLoop1:
        mov rax, [rbx]
        mov [rsi], rax
        add rbx, 8
        add rsi, 8
        loop .copyLoop1

    mov rbx, cr3
    mov rcx, 256
    
    .copyLoop2:
        mov rax, [rbx]
        mov [rsi], rax
        add rbx, 8
        add rsi, 8
        loop .copyLoop2

    mov rax, pml4
    mov cr3, rax

    lgdt [GDT64.pointer]

    mov rax, GDT64.code0
    push rax
    mov rax, _startGDT
    push rax
    ret

_startGDT:
    
    mov ax, GDT64.data0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov rsp, stack_end

    ; Disable PIC
    ;mov al, 0xFF
    ;out 0xA1, al
    ;out 0x21, al

    mov rax, kmain
    mov rdi, [mmap]
    mov rsi, [gmode]
    mov rdx, [rdsp]
    mov rdx, cr3
    call rax

    .hang:
        cli
        hlt
        jmp .hang

SECTION .bss

stack_begin:
resb 8192
stack_end: