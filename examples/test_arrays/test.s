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
        sub rsp, 24
        push qword 11
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 22
        mov r13, 1
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        pop qword [rbx + 0]
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, 1
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        mov r11, [rbx + 0]
        pop qword r10
        add r10, r11
        push qword r10
        mov r13, 2
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        pop qword [rbx + 0]
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        push qword [rbx + 0]
        call print_num_cdecl
        add rsp, 8
        mov r13, 1
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        push qword [rbx + 0]
        call print_num_cdecl
        add rsp, 8
        mov r13, 2
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rbp - 24]
        add rbx, r13
        push qword [rbx + 0]
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 24
        pop qword rbp
        ret
