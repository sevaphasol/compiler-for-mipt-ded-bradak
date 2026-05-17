section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        lea rax, [rel __strings + "belominon"]
        push qword rax
        pop qword [rel __global_data + 8]
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 16
        call scan_str_cdecl
        add rsp, 0
        mov [rbp - 8], rax
        push qword [rel __global_data + 8]
        push qword [rbp - 8]
        call str_cmp
        add rsp, 16
        mov [rbp - 16], rax
        mov rax, [rbp - 16]
        test rax, rax
        je .L0
        push qword 0
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 16
        pop qword rbp
        ret
.L0:
        push qword 1
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 16
        pop qword rbp
        ret
