section .text
global _start

_start:
        call main
        mov rax, 60
        syscall
factorial:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        mov rax, [rbp + 16]
        test rax, rax
        je .L0
        push qword [rbp + 16]
        push qword [rbp + 16]
        mov r11, 1
        pop qword r10
        sub r10, r11
        push qword r10
        call factorial
        add rsp, 8
        mov r11, rax
        pop qword r10
        imul r10, r11
        mov rax, r10
        add rsp, 0
        pop qword rbp
        ret
.L0:
        mov rax, 1
        add rsp, 0
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 0
        call scan 
        mov [rbp - 8], rax
        push qword [rbp - 8]
        call factorial
        add rsp, 8
        push qword rax
        call print 
        add rsp, 8
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
