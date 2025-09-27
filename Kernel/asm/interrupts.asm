GLOBAL _hlt
GLOBAL _cli
GLOBAL _sti

GLOBAL picMasterMask
GLOBAL picSlaveMask

GLOBAL haltcpu

GLOBAL load_idtr

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler

GLOBAL _irqWriteHandler
GLOBAL _syscallHandler

GLOBAL _int80Handler
GLOBAL get_registers
GLOBAL _exception0Handler ; Zero Division Exception
GLOBAL _exception6Handler ; Invalid Opcode Exception

GLOBAL reg_array
GLOBAL getEscPressed
GLOBAL getPressedKey

EXTERN int_write
EXTERN irqDispatcher
EXTERN exceptionDispatcher
EXTERN sysCallDispatcher
EXTERN getStackBase  

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro exceptionHandler 1
    saveRegisters
	mov byte [escPressedFlag], 1 ; Hardcode ESC pressed flag so that it can be printed in the exception handler

    mov rdi, %1 ; pasaje de parametro
    call exceptionDispatcher

    popState
    
    ; Now set up a proper iretq frame to return to userland/shell
    push qword 0x0      ; SS
    call getStackBase
    push rax            ; RSP - stack base address
    pushf               ; RFLAGS
    push qword 0x8      ; CS - assuming code segment is 0x8
    mov rax, userland
    push rax            ; RIP - return to userland/shell entry point
    
    iretq
%endmacro

%macro saveRegisters 0
    ; Save all registers to reg_array
    mov [reg_array + 0*8],  rax
    mov [reg_array + 1*8],  rbx
    mov [reg_array + 2*8],  rcx
    mov [reg_array + 3*8],  rdx
    mov [reg_array + 4*8],  rbp
    mov [reg_array + 5*8],  rdi
    mov [reg_array + 6*8],  rsi
    mov [reg_array + 7*8],  r8
    mov [reg_array + 8*8],  r9
    mov [reg_array + 9*8],  r10
    mov [reg_array + 10*8], r11
    mov [reg_array + 11*8], r12
    mov [reg_array + 12*8], r13
    mov [reg_array + 13*8], r14
    mov [reg_array + 14*8], r15
    
    ; Save interrupt frame
    mov rax, [rsp+0] ; RIP
    mov [reg_array + 15*8], rax
    mov rax, [rsp+8] ; CS
    mov [reg_array + 16*8], rax
    mov rax, [rsp+16] ; RFLAGS
    mov [reg_array + 17*8], rax
    mov rax, [rsp+24] ; RSP
    mov [reg_array + 18*8], rax
    mov rax, [rsp+32] ; SS
    mov [reg_array + 19*8], rax

	;int3 
%endmacro


_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
	mov rsp, rbp
    pop rbp
    ret

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
	mov rsp, rbp
    pop     rbp
    retn

	
; --- Carga la IDT desde [RDI] (pointer a estructura {limit, base})
load_idtr:
    ; Memoria en [RDI] = {uint16_t limit; uint64_t base;}
    lidt    [rdi]
    ret


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0


	
;Keyboard
_irq01Handler:
	push rax
	xor rax, rax
	in al, 60h ; Read from keyboard data port
	mov [pressedKey], al ; Store the pressed key
	cmp al, REGS_SNAPSHOT
	jne .dontCapture

	pop rax
	saveRegisters ; macro to save all registers to reg_array
	mov byte [escPressedFlag], 1 ; Set the ESC pressed flag
	jmp .continue

.dontCapture:
	pop rax

.continue:
	irqHandlerMaster 1

getPressedKey:
	mov rax, [pressedKey] ; Return the pressed key
	ret
	
getEscPressed:
    xor rax, rax
    mov al, [escPressedFlag]
    ret


;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5


_int80Handler:
	pushState
	mov r9, r8
	mov r8, r10
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi 
	mov rdi, rax
	
	call sysCallDispatcher
	mov [aux], rax
	popState
	mov rax, [aux]
	iretq

	;Zero Division Exception
_exception0Handler:
	;mov rdi, 0
	exceptionHandler 0
	iretq

	; Invalid Opcode Exception
_exception6Handler:
    ;mov   rdi, 6
    exceptionHandler 6
    iretq


haltcpu:
	cli
	hlt
	ret

section .bss
	aux resq 1
	reg_array resq 20 ; 20 registers * 8 bytes each = 160 bytes 
	pressedKey resb 1
	escPressedFlag resb 1 ; Flag to indicate if ESC key was pressed

section .rodata
    userland equ 0x400000
	REGS_SNAPSHOT equ 0x01 ; ESC key scancode

	