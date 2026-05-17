section .text
global _start

_start:
        call main
        mov rax, 60
        syscall
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        call launch_localhost_client_3000
        add rsp, 0
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
