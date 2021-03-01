GLOBAL _ZN8SpinlockC1Ev
_ZN8SpinlockC1Ev:
    mov QWORD [rdi], 0
    ret

GLOBAL _ZN8Spinlock7AcquireEv
_ZN8Spinlock7AcquireEv:
    lock bts QWORD [rdi], 0        ;Attempt to acquire the lock (in case lock is uncontended)
    jc .spin_with_pause
    ret
 
.spin_with_pause:
    pause                    ; Tell CPU we're spinning
    test QWORD [rdi], 1      ; Is the lock free?
    jnz .spin_with_pause     ; no, wait
    jmp _ZN8Spinlock7AcquireEv          ; retry
 
GLOBAL _ZN8Spinlock7ReleaseEv
_ZN8Spinlock7ReleaseEv:
    mov QWORD [rdi],0
    ret