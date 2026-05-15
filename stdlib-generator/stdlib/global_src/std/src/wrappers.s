.intel_syntax noprefix
.text

.global print_cdecl
.type print_cdecl, @function
print_cdecl:
    mov edi, DWORD PTR [rsp + 8]   # read argument before touching rsp
    push rbx
    mov rbx, rsp
    and rsp, -16                   # align for the C call
    call print_V_amd64
    mov rsp, rbx
    pop rbx
    ret

.global scan_cdecl
.type scan_cdecl, @function
scan_cdecl:
    push rbx
    mov rbx, rsp
    and rsp, -16
    call scan_V_amd64
    mov rsp, rbx
    pop rbx
    ret


.global print_53_cdecl
.type print_53_cdecl, @function
print_53_cdecl:
    call    print_53_detail
    ret
