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
        sub rsp, 0
        push qword 1
        push qword 0
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L0
        push qword 0
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
.L0:
        push qword 1
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
