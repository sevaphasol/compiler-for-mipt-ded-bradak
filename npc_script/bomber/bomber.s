section .text
global _start

_start:
        call __global_init
        call main
        mov rdi, rax
        mov rax, 60
        syscall
__global_init:
        mov [rel __global_data + 0], 0
        mov [rel __global_data + 8], 1
        mov [rel __global_data + 16], 0
        mov [rel __global_data + 24], 0
        mov [rel __global_data + 32], 0
        mov [rel __global_data + 40], 0
        mov [rel __global_data + 145520], 0
        mov [rel __global_data + 145528], 0
        mov [rel __global_data + 145536], 0
        ret
iabs:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword 0
        push qword [rbp + 16]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L1
        push qword 0
        mov r11, [rbp + 16]
        pop qword r10
        sub r10, r11
        mov rax, r10
        jmp .L0
.L1:
        mov rax, [rbp + 16]
        jmp .L0
        mov rax, 0
.L0:
        add rsp, 0
        pop qword rbp
        ret
sign:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword 0
        push qword [rbp + 16]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L3
        mov rax, 1
        jmp .L2
.L3:
        push qword 0
        push qword [rbp + 16]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L4
        mov rax, -1
        jmp .L2
.L4:
        mov rax, 0
        jmp .L2
        mov rax, 0
.L2:
        add rsp, 0
        pop qword rbp
        ret
max2:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rbp + 24]
        push qword [rbp + 16]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L6
        mov rax, [rbp + 16]
        jmp .L5
.L6:
        mov rax, [rbp + 24]
        jmp .L5
        mov rax, 0
.L5:
        add rsp, 0
        pop qword rbp
        ret
long_dist:
        push qword rbp
        mov rbp, rsp
        sub rsp, 16
        push qword [rbp + 16]
        mov r11, [rbp + 32]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 8], rax
        push qword [rbp + 24]
        mov r11, [rbp + 40]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 16], rax
        push qword [rbp - 16]
        push qword [rbp - 8]
        call max2
        add rsp, 16
        mov rax, rax
        jmp .L7
        mov rax, 0
.L7:
        add rsp, 16
        pop qword rbp
        ret
wall_known:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 0
        jmp .L9
.L10:
        push qword [rbp + 16]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 6192]
        add rbx, r13
        push qword [rbx + 0]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L11
        push qword [rbp + 24]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 71728]
        add rbx, r13
        push qword [rbx + 0]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L12
        mov rax, 1
        jmp .L8
.L12:
.L11:
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 8], r10
.L9:
        push qword [rel __global_data + 40]
        push qword [rbp - 8]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        jne .L10
        mov rax, 0
        jmp .L8
        mov rax, 0
.L8:
        add rsp, 8
        pop qword rbp
        ret
add_wall:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        push qword [rbp + 24]
        push qword [rbp + 16]
        call wall_known
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L14
        mov rax, 0
        jmp .L13
.L14:
        push qword 8192
        push qword [rel __global_data + 40]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L15
        push qword [rbp + 16]
        mov r13, [rel __global_data + 40]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 6192]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp + 24]
        mov r13, [rel __global_data + 40]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 71728]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rel __global_data + 40]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rel __global_data + 40], r10
.L15:
        mov rax, 0
        jmp .L13
        mov rax, 0
.L13:
        add rsp, 0
        pop qword rbp
        ret
add_visible:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov [rbp - 8], 0
        jmp .L17
.L18:
        push qword [rbp + 16]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 48]
        add rbx, r13
        push qword [rbx + 0]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L19
        push qword [rbp + 24]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp + 32]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        pop qword [rbx + 0]
        mov rax, 0
        jmp .L16
.L19:
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 8], r10
.L17:
        push qword [rel __global_data + 32]
        push qword [rbp - 8]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        jne .L18
        push qword 256
        push qword [rel __global_data + 32]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L20
        push qword [rbp + 16]
        mov r13, [rel __global_data + 32]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 48]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp + 24]
        mov r13, [rel __global_data + 32]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp + 32]
        mov r13, [rel __global_data + 32]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rel __global_data + 32]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rel __global_data + 32], r10
.L20:
        mov rax, 0
        jmp .L16
        mov rax, 0
.L16:
        add rsp, 8
        pop qword rbp
        ret
clear_tick_state:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        mov [rel __global_data + 32], 0
        mov rax, 0
        jmp .L21
        mov rax, 0
.L21:
        add rsp, 0
        pop qword rbp
        ret
try_move:
        push qword rbp
        mov rbp, rsp
        sub rsp, 16
        push qword 0
        push qword [rbp + 16]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L23
        push qword 0
        push qword [rbp + 24]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L24
        mov rax, 0
        jmp .L22
.L24:
.L23:
        push qword [rel __global_data + 16]
        mov r11, [rbp + 16]
        pop qword r10
        add r10, r11
        mov [rbp - 8], r10
        push qword [rel __global_data + 24]
        mov r11, [rbp + 24]
        pop qword r10
        add r10, r11
        mov [rbp - 16], r10
        mov rax, [rel __global_data + 0]
        test rax, rax
        je .L25
        push qword [rbp - 16]
        push qword [rbp - 8]
        call wall_known
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L26
        mov rax, 0
        jmp .L22
.L26:
.L25:
        push qword 0
        push qword [rbp + 24]
        push qword [rbp + 16]
        lea r12, [rel __strings + "bomber"]
        push qword r12
        call lang_move_cdecl
        add rsp, 24
        push qword rax
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L27
        call lang_client_set_dead_cdecl
        add rsp, 0
        mov rax, 1
        jmp .L22
.L27:
        mov rax, 1
        jmp .L22
        mov rax, 0
.L22:
        add rsp, 16
        pop qword rbp
        ret
get_closest_one:
        push qword rbp
        mov rbp, rsp
        sub rsp, 32
        mov [rbp - 8], 0
        mov rax, [rel __global_data + 0]
        test rax, rax
        je .L29
        mov [rbp - 16], 0
        jmp .L30
.L31:
        mov r13, [rbp - 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 16]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 24], rax
        mov r13, [rbp - 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 32], rax
        push qword 36
        push qword [rbp - 24]
        mov r11, [rbp - 24]
        pop qword r10
        imul r10, r11
        push qword r10
        push qword [rbp - 32]
        mov r11, [rbp - 32]
        pop qword r10
        imul r10, r11
        mov r11, r10
        pop qword r10
        add r10, r11
        push qword r10
        call smaller_or_eq_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L32
        mov rax, [rbp - 16]
        jmp .L28
.L32:
        push qword [rbp - 16]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 16], r10
.L30:
        push qword [rel __global_data + 32]
        push qword [rbp - 16]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        jne .L31
.L29:
        mov rax, -1
        jmp .L28
        mov rax, 0
.L28:
        add rsp, 32
        pop qword rbp
        ret
attack_if_possible:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        mov rax, [rel __global_data + 145536]
        test rax, rax
        je .L34
        mov rax, 0
        jmp .L33
.L34:
        call get_closest_one
        add rsp, 0
        mov [rbp - 8], rax
        push qword -1
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L35
        mov rax, 0
        jmp .L33
.L35:
        mov [rel __global_data + 145536], 1
        push qword 0
        lea r12, [rel __strings + "bomber"]
        push qword r12
        call lang_pan_send_bomb_cdecl
        add rsp, 8
        push qword rax
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L36
        mov rax, 0
        jmp .L33
.L36:
        mov rax, 1
        jmp .L33
        mov rax, 0
.L33:
        add rsp, 8
        pop qword rbp
        ret
run_if_possible:
        push qword rbp
        mov rbp, rsp
        sub rsp, 64
        mov rax, [rel __global_data + 0]
        test rax, rax
        je .L38
        push qword 0
        push qword [rel __global_data + 32]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L39
        mov [rbp - 8], 0
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        push qword [rbx + 0]
        push qword [rel __global_data + 24]
        push qword [rel __global_data + 16]
        call long_dist
        add rsp, 32
        mov [rbp - 16], rax
        mov [rbp - 24], 1
        jmp .L40
.L41:
        mov r13, [rbp - 24]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp - 24]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        push qword [rbx + 0]
        push qword [rel __global_data + 24]
        push qword [rel __global_data + 16]
        call long_dist
        add rsp, 32
        mov [rbp - 32], rax
        push qword [rbp - 16]
        push qword [rbp - 32]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L42
        push qword [rbp - 24]
        pop qword [rbp - 8]
        push qword [rbp - 32]
        pop qword [rbp - 16]
.L42:
        push qword [rbp - 24]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 24], r10
.L40:
        push qword [rel __global_data + 32]
        push qword [rbp - 24]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        jne .L41
        push qword 6
        push qword [rbp - 16]
        call bigger_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L43
        mov rax, 0
        jmp .L37
.L43:
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 16]
        pop qword r10
        sub r10, r11
        push qword r10
        call sign
        add rsp, 8
        mov [rbp - 40], rax
        push qword 0
        mov r11, [rbp - 40]
        pop qword r10
        sub r10, r11
        mov [rbp - 40], r10
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        call sign
        add rsp, 8
        mov [rbp - 48], rax
        push qword 0
        mov r11, [rbp - 48]
        pop qword r10
        sub r10, r11
        mov [rbp - 48], r10
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 2096]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 16]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 56], rax
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 4144]
        add rbx, r13
        push qword [rbx + 0]
        mov r11, [rel __global_data + 24]
        pop qword r10
        sub r10, r11
        push qword r10
        call iabs
        add rsp, 8
        mov [rbp - 64], rax
        push qword [rbp - 64]
        push qword [rbp - 56]
        call bigger_or_eq_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L44
        push qword [rbp - 48]
        push qword 0
        call try_move
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L45
        call clear_tick_state
        add rsp, 0
        mov rax, 1
        jmp .L37
.L45:
        push qword 0
        push qword [rbp - 40]
        call try_move
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L46
        call clear_tick_state
        add rsp, 0
        mov rax, 1
        jmp .L37
.L46:
.L44:
        push qword [rbp - 64]
        push qword [rbp - 56]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L47
        push qword 0
        push qword [rbp - 40]
        call try_move
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L48
        call clear_tick_state
        add rsp, 0
        mov rax, 1
        jmp .L37
.L48:
        push qword [rbp - 48]
        push qword 0
        call try_move
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L49
        call clear_tick_state
        add rsp, 0
        mov rax, 1
        jmp .L37
.L49:
.L47:
.L39:
.L38:
        mov rax, 0
        jmp .L37
        mov rax, 0
.L37:
        add rsp, 64
        pop qword rbp
        ret
random_move:
        push qword rbp
        mov rbp, rsp
        sub rsp, 64
        push qword 1
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        mov r13, 0
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        pop qword [rbx + 0]
        push qword -1
        mov r13, 1
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        mov r13, 1
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        mov r13, 2
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 1
        mov r13, 2
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        pop qword [rbx + 0]
        push qword 0
        mov r13, 3
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        pop qword [rbx + 0]
        push qword -1
        mov r13, 3
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        pop qword [rbx + 0]
        mov [rbp - 8], 0
        mov [rbp - 16], 0
        jmp .L51
.L52:
        mov r13, [rbp - 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        push qword [rbx + 0]
        pop qword [rbp - 24]
        mov r13, [rbp - 16]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        push qword [rbx + 0]
        pop qword [rbp - 32]
        push qword [rel __global_data + 16]
        mov r11, [rbp - 24]
        pop qword r10
        add r10, r11
        mov [rbp - 40], r10
        push qword [rel __global_data + 24]
        mov r11, [rbp - 32]
        pop qword r10
        add r10, r11
        mov [rbp - 48], r10
        mov [rbp - 56], 1
        mov rax, [rel __global_data + 0]
        test rax, rax
        je .L53
        push qword [rbp - 48]
        push qword [rbp - 40]
        call wall_known
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L54
        mov [rbp - 56], 0
.L54:
.L53:
        mov rax, [rbp - 56]
        test rax, rax
        je .L55
        push qword [rbp - 24]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 32]
        mov r13, [rbp - 8]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        pop qword [rbx + 0]
        push qword [rbp - 8]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 8], r10
.L55:
        push qword [rbp - 16]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rbp - 16], r10
.L51:
        push qword 4
        push qword [rbp - 16]
        call smaller_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        jne .L52
        push qword 0
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L56
        call clear_tick_state
        add rsp, 0
        mov rax, 0
        jmp .L50
.L56:
        push qword [rbp - 8]
        call lang_random_mod_cdecl
        add rsp, 8
        mov [rbp - 64], rax
        push qword 0
        mov r13, [rbp - 64]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143440]
        add rbx, r13
        push qword [rbx + 0]
        mov r13, [rbp - 64]
        add r13, r13
        add r13, r13
        add r13, r13
        lea rbx, [rel __global_data + 143408]
        add rbx, r13
        push qword [rbx + 0]
        lea r12, [rel __strings + "bomber"]
        push qword r12
        call lang_move_cdecl
        add rsp, 24
        push qword rax
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L57
        call lang_client_set_dead_cdecl
        add rsp, 0
        mov rax, -1
        jmp .L50
.L57:
        call clear_tick_state
        add rsp, 0
        mov rax, 0
        jmp .L50
        mov rax, 0
.L50:
        add rsp, 64
        pop qword rbp
        ret
do_random_action:
        push qword rbp
        mov rbp, rsp
        sub rsp, 0
        call attack_if_possible
        add rsp, 0
        mov rax, rax
        test rax, rax
        je .L59
        mov rax, 0
        jmp .L58
.L59:
        mov [rel __global_data + 145536], 0
        call run_if_possible
        add rsp, 0
        mov rax, rax
        test rax, rax
        je .L60
        mov rax, 0
        jmp .L58
.L60:
        call random_move
        add rsp, 0
        mov rax, 0
        jmp .L58
        mov rax, 0
.L58:
        add rsp, 0
        pop qword rbp
        ret
handle_person:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        call lang_pan_len_cdecl
        add rsp, 0
        mov [rbp - 8], rax
        lea r12, [rel __strings + "tick"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L62
        push qword 0
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L63
        call do_random_action
        add rsp, 0
.L63:
        mov [rel __global_data + 32], 0
        mov rax, 0
        jmp .L61
.L62:
        lea r12, [rel __strings + "hp"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L64
        push qword 4
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L65
        push qword 0
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        mov [rel __global_data + 8], rax
        push qword 0
        push qword [rel __global_data + 8]
        call smaller_or_eq_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L66
        call lang_client_set_dead_cdecl
        add rsp, 0
.L66:
.L65:
        mov rax, 0
        jmp .L61
.L64:
        lea r12, [rel __strings + "at"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L67
        push qword 8
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L68
        push qword 0
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        mov [rel __global_data + 16], rax
        push qword 4
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        mov [rel __global_data + 24], rax
        mov [rel __global_data + 0], 1
.L68:
        mov rax, 0
        jmp .L61
.L67:
        lea r12, [rel __strings + "root"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        push qword rax
        lea r12, [rel __strings + "enemy"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov r11, rax
        pop qword r10
        add r10, r11
        mov rax, r10
        test rax, rax
        je .L69
        push qword 4
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        push qword rax
        push qword 0
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        push qword rax
        push qword 8
        call lang_pan_payload_u32_cdecl
        add rsp, 8
        push qword rax
        call add_visible
        add rsp, 24
        mov rax, 0
        jmp .L61
.L69:
        lea r12, [rel __strings + "wall"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L70
        push qword 8
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L71
        push qword 4
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        push qword rax
        push qword 0
        call lang_pan_payload_i32_cdecl
        add rsp, 8
        push qword rax
        call add_wall
        add rsp, 16
.L71:
        mov rax, 0
        jmp .L61
.L70:
        lea r12, [rel __strings + "ability"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L72
        push qword [rel __global_data + 145528]
        mov r11, 1
        pop qword r10
        add r10, r11
        mov [rel __global_data + 145528], r10
        mov rax, 0
        jmp .L61
.L72:
        mov rax, 0
        jmp .L61
        mov rax, 0
.L61:
        add rsp, 8
        pop qword rbp
        ret
handle_srv:
        push qword rbp
        mov rbp, rsp
        sub rsp, 32
        call lang_pan_len_cdecl
        add rsp, 0
        mov [rbp - 8], rax
        lea r12, [rel __strings + "hasPref"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L74
        push qword 8
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L75
        mov rax, 0
        jmp .L73
.L75:
        mov rax, 0
        jmp .L73
.L74:
        lea r12, [rel __strings + "name"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L76
        mov rax, 0
        jmp .L73
.L76:
        lea r12, [rel __strings + "id"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L77
        push qword 4
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L78
        push qword 0
        call lang_pan_payload_u32_cdecl
        add rsp, 8
        mov [rbp - 16], rax
.L78:
        mov rax, 0
        jmp .L73
.L77:
        lea r12, [rel __strings + "level"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L79
        push qword 8
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L80
        mov rax, 0
        jmp .L73
.L80:
        mov rax, 0
        jmp .L73
.L79:
        lea r12, [rel __strings + "r.setLvl"]
        push qword r12
        call lang_pan_type_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L81
        push qword 5
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L82
        push qword 0
        call lang_pan_payload_u32_cdecl
        add rsp, 8
        mov [rbp - 24], rax
        push qword 4
        call lang_pan_payload_bool_cdecl
        add rsp, 8
        mov [rbp - 32], rax
.L82:
        mov rax, 0
        jmp .L73
.L81:
        mov rax, 0
        jmp .L73
        mov rax, 0
.L73:
        add rsp, 32
        pop qword rbp
        ret
main:
        push qword rbp
        mov rbp, rsp
        sub rsp, 8
        lea r12, [rel __strings + "bomber"]
        push qword r12
        lea r12, [rel __strings + "3000"]
        push qword r12
        lea r12, [rel __strings + "localhost"]
        push qword r12
        call lang_client_connect_cdecl
        add rsp, 24
        mov [rbp - 8], rax
        push qword 0
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L84
        mov rax, 1
        jmp .L83
.L84:
        lea r12, [rel __strings + "bomber"]
        push qword r12
        call lang_choose_role_cdecl
        add rsp, 8
        jmp .L85
.L86:
        call lang_client_read_next_cdecl
        add rsp, 0
        mov [rbp - 8], rax
        push qword 2
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L87
        call lang_client_close_cdecl
        add rsp, 0
        mov rax, 0
        jmp .L83
.L87:
        push qword 0
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L88
        push qword 1
        push qword [rbp - 8]
        call not_equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L89
        call lang_client_close_cdecl
        add rsp, 0
        mov rax, 1
        jmp .L83
.L89:
.L88:
        push qword 0
        push qword [rbp - 8]
        call equal_cdecl
        add rsp, 16
        mov rax, rax
        test rax, rax
        je .L90
        lea r12, [rel __strings + "bomber"]
        push qword r12
        call lang_pan_prefix_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L91
        call handle_person
        add rsp, 0
.L91:
        lea r12, [rel __strings + "srv"]
        push qword r12
        call lang_pan_prefix_is_cdecl
        add rsp, 8
        mov rax, rax
        test rax, rax
        je .L92
        call handle_srv
        add rsp, 0
.L92:
.L90:
.L85:
        call lang_client_alive_cdecl
        add rsp, 0
        mov rax, rax
        test rax, rax
        jne .L86
        call lang_client_close_cdecl
        add rsp, 0
        mov rax, 0
        jmp .L83
        mov rax, 0
.L83:
        add rsp, 8
        pop qword rbp
        ret
