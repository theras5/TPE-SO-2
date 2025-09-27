GLOBAL sys_call
GLOBAL _hlt
GLOBAL _cli
GLOBAL _sti

GLOBAL _div0
GLOBAL _ioc

section .text

; syscall handler
sys_call:
    mov rax, rdi        ; syscall number
    mov rdi, rsi        ; arg1
    mov rsi, rdx        ; arg2
    mov rdx, rcx        ; arg3
    mov r10, r8         ; arg4
    mov r8, r9          ; arg5 
    int 0x80
    ret
; wait for interrupt
_hlt:
	sti
	hlt
	ret

; disable interrupts
_cli:
	cli
	ret

; enable interrupts
_sti:
	sti
	ret

    ; divide by zero
_div0:
    mov rax, 0
    mov rdx, 0
    div rax
    ret


; invalid opcode 
_ioc:
    ud2
    ret