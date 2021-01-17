BITS 32
SECTION .low_text

kernel_vma: equ 0xFFFF800000000000

;======================================
; MULTIBOOT HEADER
;======================================
ALIGN 8
multiboot_header:
    .magic:         dd  0xE85250D6
    .architecture:  dd  0
    .header_length: dd  (multiboot_header_end - multiboot_header)
    .checksum:      dd  -(0xE85250D6 + (multiboot_header_end - multiboot_header))
    .tags:
        .information_request:
            .ir_type:  dw  1
            .ir_flags: dw  0
            .ir_size:  dd  0x10
            .mbi_tags:
                dd  6 ; Memory Map
                dd  4 ; Basic Memory Info
        .end:
            .type:  dw  0
            .flags: dw  0
            .size:  dd  8

multiboot_header_end:

;======================================
; RESERVED MEMORY
;======================================
ALIGN 4096
pml4:   times 512 dq 0
pdpt:   times 512 dq 0
pd:     times 512 dq 0
pt1:    times 512 dq 0
pt2:    times 512 dq 0
pt3:    times 512 dq 0
pt4:    times 512 dq 0
pt5:    times 512 dq 0
pt6:    times 512 dq 0
pt7:    times 512 dq 0
pt8:    times 512 dq 0
pt9:    times 512 dq 0
pt10:   times 512 dq 0
pt11:   times 512 dq 0
pt12:   times 512 dq 0
pt13:   times 512 dq 0
pt14:   times 512 dq 0
pt15:   times 512 dq 0
pt16:   times 512 dq 0

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
        
multibootInfo:  dd  0

;======================================
; BOOT ENTRY POINT
;======================================
GLOBAL _start
_start:
    ; Save EBX register
    mov [multibootInfo], ebx

    ; Clear previous paging
    mov eax, cr0
    and eax, 01111111111111111111111111111111b
    mov cr0, eax

    ; Clear our page tables
    mov edi, pml4
    mov cr3, edi
    xor eax, eax
    mov ecx, 8192
    rep stosd
    mov edi, cr3

    ; Setup higher page entries
    mov DWORD [edi], (pdpt + 3)
    add edi, 0x800
    mov DWORD [edi], (pdpt + 3)
    add edi, 0x800
    mov DWORD [edi], (pd + 3)
    add edi, 0x1000
    mov DWORD [edi], (pt1 + 3)
    add edi, 8
    mov DWORD [edi], (pt2 + 3)
    add edi, 8
    mov DWORD [edi], (pt3 + 3)
    add edi, 8
    mov DWORD [edi], (pt4 + 3)
    add edi, 8
    mov DWORD [edi], (pt5 + 3)
    add edi, 8
    mov DWORD [edi], (pt6 + 3)
    add edi, 8
    mov DWORD [edi], (pt7 + 3)
    add edi, 8
    mov DWORD [edi], (pt8 + 3)
    add edi, 8
    mov DWORD [edi], (pt9 + 3)
    add edi, 8
    mov DWORD [edi], (pt10 + 3)
    add edi, 8
    mov DWORD [edi], (pt11 + 3)
    add edi, 8
    mov DWORD [edi], (pt12 + 3)
    add edi, 8
    mov DWORD [edi], (pt13 + 3)
    add edi, 8
    mov DWORD [edi], (pt14 + 3)
    add edi, 8
    mov DWORD [edi], (pt15 + 3)
    add edi, 8
    add DWORD [edi], (pt16 + 3)
    add edi, (0x1000 - 120)
 
    ; Identity map the first sixteen megabytes
    mov ebx, 3
    mov ecx, 8192

    .set_entry:
        mov DWORD [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop .set_entry

    ; Enable PAE paging
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable the long mode bit
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load GDT and jump to 64-bit code
    lgdt [GDT64.pointer]
    jmp GDT64.code0:_start64

BITS 64
EXTERN kmain
_start64:
    cli

    lgdt [GDT64.pointer]

    mov ax, GDT64.data0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov rsp, stack_end

    ; Disable PIC
    mov al, 0xFF
    out 0xA1, al
    out 0x21, al

    mov rax, kmain
    xor rdi, rdi
    mov edi, [multibootInfo]
    call rax

    .hang:
        cli
        hlt
        jmp .hang

SECTION .bss

stack_begin:
resb 8192
stack_end: