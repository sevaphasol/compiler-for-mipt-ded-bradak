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
        sub rsp, 16
        mov [rbp - 8], 0
        call scan_num_cdecl
        add rsp, 0
        mov [rbp - 16], rax
        push qword [rbp - 8]
        call print_num_cdecl
        add rsp, 8
        push qword [rbp - 16]
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 16
        pop qword rbp
        ret
        mov rax, 0
        add rsp, 16
        pop qword rbp
        ret
