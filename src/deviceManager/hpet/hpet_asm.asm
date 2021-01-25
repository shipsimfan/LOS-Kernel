BITS 64

GLOBAL HPETTimerIRQHandler
EXTERN HPETTimerIRQ
HPETTimerIRQHandler:
    call HPETTimerIRQ
    iretq