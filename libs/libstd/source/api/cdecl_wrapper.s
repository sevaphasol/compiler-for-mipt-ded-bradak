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

CDECL_WRAP memcpy, memcpy_vabi, 3
CDECL_WRAP memset, memset_vabi, 3
CDECL_WRAP strlen, strlen_vabi, 1
CDECL_WRAP strcmp, strcmp_vabi, 2
CDECL_WRAP streq, streq_vabi, 1
CDECL_WRAP print_num, print_num_vabi, 1
CDECL_WRAP print_str, print_str_vabi, 1
CDECL_WRAP scan_num, scan_num_vabi, 0
CDECL_WRAP equal, equal_vabi, 2
CDECL_WRAP not_equal, not_equal_vabi, 2
CDECL_WRAP smaller, smaller_vabi, 2
CDECL_WRAP bigger, bigger_vabi, 2
CDECL_WRAP smaller_or_eq, smaller_or_eq_vabi, 2
CDECL_WRAP bigger_or_eq, bigger_or_eq_vabi, 2
CDECL_WRAP random_mod, random_mod_vabi, 1

.section .note.GNU-stack,"",@progbits
