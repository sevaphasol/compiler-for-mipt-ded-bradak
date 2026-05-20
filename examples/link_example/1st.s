section .text
global _start

main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        call external_func
        add rsp, 0
        mov rax, 0
        jmp .L0
        mov rax, 0
.L0:
        add rsp, 0
        pop qword rbp
        ret
