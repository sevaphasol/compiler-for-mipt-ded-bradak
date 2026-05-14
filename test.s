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
        mov [rbp - 8], 52
        push qword [rbp - 8]
        call print 
        add rsp, 8
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
