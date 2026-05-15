section .text
global _start

_start:
        call main
        mov rax, 60
        syscall
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 5
        jmp .L0
.L1:
        push qword [rbp - 8]
        call print
        add rsp, 8
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
.L0:
        mov rax, [rbp - 8]
        test rax, rax
        jne .L1
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
