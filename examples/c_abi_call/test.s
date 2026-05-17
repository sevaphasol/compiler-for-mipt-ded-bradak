section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 63
        push qword [rbp - 8]
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
