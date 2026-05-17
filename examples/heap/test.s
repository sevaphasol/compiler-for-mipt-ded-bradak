section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        mov [rel __global_data + 80], 10
        ret
fill_elems:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 10
        jmp .L0
.L1:
        call scan_num_cdecl
        add rsp, 0
        push qword rax
        push qword 10
        mov r11, [rbp - 8]
        pop qword r10
        sub r10, r11
        mov r13, r10
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
.L0:
        mov rax, [rbp - 8]
        test rax, rax
        jne .L1
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        call fill_elems
        add rsp, 0
        mov [rbp - 8], 10
        jmp .L2
.L3:
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        call print_num_cdecl
        add rsp, 8
.L2:
        mov rax, [rbp - 8]
        test rax, rax
        jne .L3
        mov rax, 0
        add rsp, 8
        pop qword rbp
        ret
