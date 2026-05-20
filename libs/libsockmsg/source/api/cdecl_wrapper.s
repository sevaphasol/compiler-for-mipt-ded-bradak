.intel_syntax noprefix
.text

.macro CALL_WITH_ALIGNED_STACK target
    push rbx
    mov rbx, rsp
    and rsp, -16
    call \target
    mov rsp, rbx
    pop rbx
    ret
.endm

.macro CDECL_WRAP name, target, argc
.global \name
.type \name, @function
\name:
    .if \argc >= 1
        mov rdi, QWORD PTR [rsp + 8]
    .endif
    .if \argc >= 2
        mov rsi, QWORD PTR [rsp + 16]
    .endif
    .if \argc >= 3
        mov rdx, QWORD PTR [rsp + 24]
    .endif
    .if \argc >= 4
        mov rcx, QWORD PTR [rsp + 32]
    .endif
    .if \argc >= 5
        mov r8,  QWORD PTR [rsp + 40]
    .endif
    .if \argc >= 6
        mov r9,  QWORD PTR [rsp + 48]
    .endif

    CALL_WITH_ALIGNED_STACK \target
.endm

CDECL_WRAP socket_connect_raw, socket_connect_raw_vabi, 2
CDECL_WRAP socket_alive, socket_alive_vabi, 0
CDECL_WRAP socket_set_dead, socket_set_dead_vabi, 0
CDECL_WRAP socket_read_next, socket_read_next_vabi, 0
CDECL_WRAP socket_close, socket_close_vabi, 0
CDECL_WRAP socket_fd, socket_fd_vabi, 0

CDECL_WRAP binmsg_begin, binmsg_begin_vabi, 2
CDECL_WRAP binmsg_write_i8, binmsg_write_i8_vabi, 1
CDECL_WRAP binmsg_write_i16, binmsg_write_i16_vabi, 1
CDECL_WRAP binmsg_write_i32, binmsg_write_i32_vabi, 1
CDECL_WRAP binmsg_write_i64, binmsg_write_i64_vabi, 1
CDECL_WRAP binmsg_write_id, binmsg_write_id_vabi, 1
CDECL_WRAP binmsg_write_bool, binmsg_write_bool_vabi, 1
CDECL_WRAP binmsg_write_string, binmsg_write_string_vabi, 1
CDECL_WRAP binmsg_write_char64, binmsg_write_char64_vabi, 1
CDECL_WRAP binmsg_send, binmsg_send_vabi, 0

CDECL_WRAP binmsg_prefix, binmsg_prefix_vabi, 0
CDECL_WRAP binmsg_type, binmsg_type_vabi, 0
CDECL_WRAP binmsg_prefix_is, binmsg_prefix_is_vabi, 1
CDECL_WRAP binmsg_type_is, binmsg_type_is_vabi, 1
CDECL_WRAP binmsg_id, binmsg_id_vabi, 0
CDECL_WRAP binmsg_len, binmsg_len_vabi, 0
CDECL_WRAP binmsg_flags, binmsg_flags_vabi, 0
CDECL_WRAP binmsg_read_i8, binmsg_read_i8_vabi, 1
CDECL_WRAP binmsg_read_i16, binmsg_read_i16_vabi, 1
CDECL_WRAP binmsg_read_i32, binmsg_read_i32_vabi, 1
CDECL_WRAP binmsg_read_i64, binmsg_read_i64_vabi, 1
CDECL_WRAP binmsg_read_u32, binmsg_read_u32_vabi, 1
CDECL_WRAP binmsg_read_bool, binmsg_read_bool_vabi, 1
CDECL_WRAP binmsg_string_eq, binmsg_string_eq_vabi, 2
CDECL_WRAP binmsg_string_size, binmsg_string_size_vabi, 1
CDECL_WRAP binmsg_char64_eq, binmsg_char64_eq_vabi, 2

.section .note.GNU-stack,"",@progbits
