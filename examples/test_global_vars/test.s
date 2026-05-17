section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        push qword 1
        pop qword [rel __global_data + 8]
        push qword 2
        pop qword [rel __global_data + 16]
        push qword 3
        pop qword [rel __global_data + 24]
        push qword 2
        pop qword [rel __global_data + 32]
        ret
print_global_var:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rel __global_data + 16]
        call print
        add rsp, 8
        push qword [rbp + 16]
        call print
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rel __global_data + 8]
        call print
        add rsp, 8
        push qword [rel __global_data + 24]
        call print_global_var
        add rsp, 8
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
