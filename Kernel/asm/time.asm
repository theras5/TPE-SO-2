global read_RTC

section .text
read_RTC:
    push rbp
    mov rbp, rsp

esperar:           ; esperar a que el RTC esté listo
    mov al, 0x0A
    out 0x70, al
    in  al, 0x71
    test al, 0x80      ; si bit 7 = 1 (UIP), está actualizando
    jnz  esperar       

    xor rax, rax        ; me aseguro que todos los bits de rax estén en 0
    mov al, dil   ; levantar valor a leer (sec, min, hour, day, month, year)

    out 0x70, al
    in  al, 0x71

    mov rsp, rbp   ; primero restauro RSP al valor de RBP
    pop rbp        ; luego deshago el push rbp
    ret



