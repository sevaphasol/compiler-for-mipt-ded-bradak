section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        mov [rel __global_data + 0], 1
        mov [rel __global_data + 8], 2
        mov [rel __global_data + 16], 3
        mov [rel __global_data + 24], 2
        ret
print_global_var:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rel __global_data + 8]
        call print_num_cdecl
        add rsp, 8
        push qword [rbp + 16]
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rel __global_data + 0]
        call print_num_cdecl
        add rsp, 8
        push qword [rel __global_data + 16]
        call print_global_var
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
