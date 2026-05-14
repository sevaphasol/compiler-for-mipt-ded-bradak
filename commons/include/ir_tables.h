#ifndef _IR_TABLES_H__
#define _IR_TABLES_H__

//——————————————————————————————————————————————————————————————————————————————

#include "ir_opcodes.h"
#include "ir_to_asm_funcs.h"
#include "encode_funcs.h"

//——————————————————————————————————————————————————————————————————————————————

/* IR operation asm names. Need this for building .s file (-s flag). */
const char* const IrOpAsmNamesTable[] = {
    [IR_OPC_NOP]          = "nop",
    [IR_OPC_ADD]          = "add",
    [IR_OPC_SUB]          = "sub",
    [IR_OPC_IMUL]         = "imul",
    [IR_OPC_IDIV]         = "idiv",
    [IR_OPC_MOV]          = "mov",
    [IR_OPC_PUSH]         = "push qword",
    [IR_OPC_POP]          = "pop qword",
    [IR_OPC_CALL]         = "call",
    [IR_OPC_RET]          = "ret",
    [IR_OPC_SYSCALL]      = "syscall",
    [IR_OPC_JMP]          = "jmp",
    [IR_OPC_JE]           = "je",
    [IR_OPC_JNE]          = "jne",
    [IR_OPC_TEST]         = "test",
    [IR_OPC_LOCAL_LABEL]  = "local_label",
    [IR_OPC_GLOBAL_LABEL] = "global_label",
    [IR_OPC_FILDL]        = "fildl",
    [IR_OPC_FSQRT]        = "fsqrt",
    [IR_OPC_FISTPL]       = "fistpl",
    [IR_OPC_CQO]          = "cqo",
};

//——————————————————————————————————————————————————————————————————————————————

typedef lang_status_t (*asm_func_t)(lang_ctx_t* ctx, ir_instr_t* ir_instr);

const asm_func_t IrOpAsmFuncsTable[] = {
    [IR_OPC_NOP]          = zeroary_opcode_ir_to_asm,
    [IR_OPC_ADD]          = binary_opcode_ir_to_asm,
    [IR_OPC_SUB]          = binary_opcode_ir_to_asm,
    [IR_OPC_IMUL]         = binary_opcode_ir_to_asm,
    [IR_OPC_IDIV]         = idiv_ir_to_asm,
    [IR_OPC_MOV]          = binary_opcode_ir_to_asm,
    [IR_OPC_PUSH]         = unary_opcode_ir_to_asm,
    [IR_OPC_POP]          = unary_opcode_ir_to_asm,
    [IR_OPC_CALL]         = unary_opcode_ir_to_asm,
    [IR_OPC_RET]          = zeroary_opcode_ir_to_asm,
    [IR_OPC_SYSCALL]      = zeroary_opcode_ir_to_asm,
    [IR_OPC_JMP]          = unary_opcode_ir_to_asm,
    [IR_OPC_JE]           = unary_opcode_ir_to_asm,
    [IR_OPC_JNE]          = unary_opcode_ir_to_asm,
    [IR_OPC_TEST]         = binary_opcode_ir_to_asm,
    [IR_OPC_LOCAL_LABEL]  = zeroary_opcode_ir_to_asm,
    [IR_OPC_GLOBAL_LABEL] = zeroary_opcode_ir_to_asm,
    [IR_OPC_FILDL]        = unary_opcode_ir_to_asm,
    [IR_OPC_FSQRT]        = zeroary_opcode_ir_to_asm,
    [IR_OPC_FISTPL]       = unary_opcode_ir_to_asm,
    [IR_OPC_CQO]          = zeroary_opcode_ir_to_asm,
};

//——————————————————————————————————————————————————————————————————————————————

typedef lang_status_t (*encode_func_t)(lang_ctx_t*  ctx,
                                       ir_instr_t*  ir_instr,
                                       bin_instr_t* bin_instr);

const encode_func_t IrOpEncodeFuncsTable[] = {
    [IR_OPC_NOP]          = encode_nop,
    [IR_OPC_ADD]          = encode_add,
    [IR_OPC_SUB]          = encode_sub,
    [IR_OPC_IMUL]         = encode_imul,
    [IR_OPC_IDIV]         = encode_idiv,
    [IR_OPC_MOV]          = encode_mov,
    [IR_OPC_PUSH]         = encode_push,
    [IR_OPC_POP]          = encode_pop,
    [IR_OPC_CALL]         = encode_call,
    [IR_OPC_RET]          = encode_ret,
    [IR_OPC_SYSCALL]      = encode_syscall,
    [IR_OPC_JMP]          = encode_jmp,
    [IR_OPC_JE]           = encode_je,
    [IR_OPC_JNE]          = encode_jne,
    [IR_OPC_TEST]         = encode_test,
    [IR_OPC_LOCAL_LABEL]  = encode_local_label,
    [IR_OPC_GLOBAL_LABEL] = encode_global_label,
    [IR_OPC_FILDL]        = encode_fildl,
    [IR_OPC_FSQRT]        = encode_fsqrt,
    [IR_OPC_FISTPL]       = encode_fistpl,
    [IR_OPC_CQO]          = encode_cqo,
};

//——————————————————————————————————————————————————————————————————————————————

#endif // _IR_TABLES_H__
