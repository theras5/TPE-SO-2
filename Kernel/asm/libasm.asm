GLOBAL cpuVendor
GLOBAL outb
GLOBAL outw
GLOBAL inb


section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp


; void outb(uint16_t port, uint8_t val)
; Writes a byte to an I/O port
outb:
    mov rdx, rdi    ; First argument (port)
    mov rax, rsi    ; Second argument (value)
    out dx, al      ; Send byte to port
    ret

; void outw(uint16_t port, uint16_t val)
; Writes a word (2 bytes) to an I/O port
outw:
    mov rdx, rdi    ; First argument (port)
    mov rax, rsi    ; Second argument (value)
    out dx, ax      ; Send word to port
    ret

; uint8_t inb(uint16_t port)
; Reads a byte from an I/O port
inb:
    mov rdx, rdi    ; First argument (port)
    xor rax, rax    ; Clear RAX
    in al, dx       ; Read byte from port into AL
    ret

