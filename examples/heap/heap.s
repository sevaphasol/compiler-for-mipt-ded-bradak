section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        mov [rel __global_data + 8000], 0
        ret
sift_up:
        push qword rbp
        mov rbp, rsp
        sub rsp, 16
        jmp .L1
.L2:
        push qword [rbp + 16]
        mov r11, 1
        pop qword r10
        sub r10, r11
        push qword r10
        mov r10, 2
        pop qword rax
        cqo
        idiv r10
        mov [rbp - 8], rax
        push qword 0
        push qword [rbp + 16]
        call smaller_or_eq_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L3
        mov rax, 0
        jmp .L0
.L3:
        mov r13, [rbp + 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        call bigger_or_eq_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L4
        mov rax, 0
        jmp .L0
.L4:
        mov [rbp - 16], 0
        mov r13, [rbp + 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        pop qword [rbp - 16]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp + 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 16]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp + 16]
        mov r11, 1
        pop qword r10
        sub r10, r11
        push qword r10
        mov r10, 2
        pop qword rax
        cqo
        idiv r10
        mov [rbp + 16], rax
.L1:
        mov rax, 1
        test rax, rax
        jne .L2
        mov rax, 0
.L0:
        add rsp, 16
        pop qword rbp
        ret
sift_down:
        push qword rbp
        mov rbp, rsp
        sub rsp, 32
        push qword [rbp + 16]
        pop qword [rbp - 8]
        push qword 2
        mov r11, [rbp + 16]
        pop qword r10
        imul r10, r11
        push qword r10
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 16], r10
        push qword 2
        mov r11, [rbp + 16]
        pop qword r10
        imul r10, r11
        push qword r10
        mov r11, 2
        pop qword r10
        add r10, r11
        mov [rbp - 24], r10
        push qword [rel __global_data + 8000]
        push qword [rbp - 16]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L6
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp - 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L7
        push qword [rbp - 16]
        pop qword [rbp - 8]
.L7:
.L6:
        push qword [rel __global_data + 8000]
        push qword [rbp - 24]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L8
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp - 24]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L9
        push qword [rbp - 24]
        pop qword [rbp - 8]
.L9:
.L8:
        push qword [rbp + 16]
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L10
        mov [rbp - 32], 0
        mov r13, [rbp + 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        pop qword [rbp - 32]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp + 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 32]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 8]
        call sift_down
        add rsp, 8
.L10:
        mov rax, 0
.L5:
        add rsp, 32
        pop qword rbp
        ret
heap_push:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rbp + 16]
        mov r13, [rel __global_data + 8000]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rel __global_data + 8000]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rel __global_data + 8000], r10
        push qword [rel __global_data + 8000]
        mov r11, 1
        pop qword r10
        sub r10, r11
        push qword r10
        call sift_up
        add rsp, 8
        mov rax, 0
.L11:
        add rsp, 0
        pop qword rbp
        ret
heap_pop:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword 0
        push qword [rel __global_data + 8000]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L13
        mov rax, 0
        jmp .L12
.L13:
        push qword [rel __global_data + 8000]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rel __global_data + 8000], r10
        mov r13, [rel __global_data + 8000]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        mov r13, [rel __global_data + 8000]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        call sift_down
        add rsp, 8
        mov rax, 0
.L12:
        add rsp, 0
        pop qword rbp
        ret
heap_top:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 0]
        add rbx, r13
        mov rax, [rbx + 0]
        jmp .L14
        mov rax, 0
.L14:
        add rsp, 0
        pop qword rbp
        ret
fill_heap:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 10
        jmp .L16
.L17:
        call scan_num_cdecl
        add rsp, 0
        push qword rax
        call heap_push
        add rsp, 8
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
.L16:
        mov rax, [rbp - 8]
        test rax, rax
        jne .L17
        mov rax, 0
        jmp .L15
        mov rax, 0
.L15:
        add rsp, 8
        pop qword rbp
        ret
print_and_clear_heap:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        push qword [rel __global_data + 8000]
        pop qword [rbp - 8]
        jmp .L19
.L20:
        call heap_top
        add rsp, 0
        push qword rax
        call print_num_cdecl
        add rsp, 8
        call heap_pop
        add rsp, 0
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        sub r10, r11
        mov [rbp - 8], r10
.L19:
        mov rax, [rbp - 8]
        test rax, rax
        jne .L20
        mov rax, 0
        jmp .L18
        mov rax, 0
.L18:
        add rsp, 8
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        call fill_heap
        add rsp, 0
        call print_and_clear_heap
        add rsp, 0
        mov rax, 0
        jmp .L21
        mov rax, 0
.L21:
        add rsp, 0
        pop qword rbp
        ret
