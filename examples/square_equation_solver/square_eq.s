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
solve_linear:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        mov rax, [rbp + 16]
        test rax, rax
        je .L0
        push qword 0
        mov r11, [rbp + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        mov r10, [rbp + 16]
        pop qword rax
        cqo
        idiv r10
        push qword rax
        call print_num_cdecl
        add rsp, 8
.L0:
        mov rax, 0
        add rsp, 0
        pop qword rbp
        ret
solve_square_eq:
        push qword rbp
        mov rbp, rsp
        sub rsp, 24
        mov rax, [rbp + 16]
        test rax, rax
        je .L1
        push qword [rbp + 24]
        mov r11, [rbp + 24]
        pop qword r10
        imul r10, r11
        push qword r10
        push qword 4
        mov r11, [rbp + 16]
        pop qword r10
        imul r10, r11
        push qword r10
        mov r11, [rbp + 32]
        pop qword r10
        imul r10, r11
        mov r11, r10
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
        push qword 0
        mov r11, [rbp + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        fildl [rbp - 8]
        fsqrt
        fistpl [rbp - 8]
        mov r11, [rbp - 8]
        pop qword r10
        add r10, r11
        push qword r10
        push qword 2
        mov r11, [rbp + 16]
        pop qword r10
        imul r10, r11
        mov r10, r10
        pop qword rax
        cqo
        idiv r10
        mov [rbp - 16], rax
        push qword 0
        mov r11, [rbp + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        fildl [rbp - 8]
        fsqrt
        fistpl [rbp - 8]
        mov r11, [rbp - 8]
        pop qword r10
        sub r10, r11
        push qword r10
        push qword 2
        mov r11, [rbp + 16]
        pop qword r10
        imul r10, r11
        mov r10, r10
        pop qword rax
        cqo
        idiv r10
        mov [rbp - 24], rax
        push qword [rbp - 16]
        call print_num_cdecl
        add rsp, 8
        push qword [rbp - 24]
        call print_num_cdecl
        add rsp, 8
        mov rax, 0
        add rsp, 24
        pop qword rbp
        ret
.L1:
        push qword [rbp + 32]
        push qword [rbp + 24]
        call solve_linear
        add rsp, 16
        mov rax, rax
        add rsp, 24
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 24
        mov [rbp - 8], 0
        mov [rbp - 16], 0
        mov [rbp - 24], 0
        call scan_num_cdecl
        add rsp, 0
        mov [rbp - 8], rax
        call scan_num_cdecl
        add rsp, 0
        mov [rbp - 16], rax
        call scan_num_cdecl
        add rsp, 0
        mov [rbp - 24], rax
        push qword [rbp - 24]
        push qword [rbp - 16]
        push qword [rbp - 8]
        call solve_square_eq
        add rsp, 24
        mov rax, rax
        add rsp, 24
        pop qword rbp
        ret
