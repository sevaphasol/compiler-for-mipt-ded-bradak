#ifndef _IR_DSL_H__
#define _IR_DSL_H__

#include "ir.h"

//——————————————————————————————————————————————————————————————————————————————

/* IR operands */

#define OPD_REG(_reg) (ir_opd_t){.type = IR_OPD_REGISTER,  .value = {.reg    = _reg}}
#define OPD_STFRAME_MEM(_off) (ir_opd_t){.type = IR_OPD_STFRAME_MEMORY, .value = {.offset = _off}}
#define OPD_ARR_OFFSET_MEM(_off) (ir_opd_t){.type = IR_OPD_ARR_OFFSET_MEMORY, .value = {.offset = _off}}
#define OPD_GLOBAL_MEM(_off) \
    (ir_opd_t){.type = IR_OPD_GLOBAL_MEMORY, .value = {.offset = _off}}
#define OPD_IMM(_imm) (ir_opd_t){.type = IR_OPD_IMMEDIATE, .value = {.imm    = _imm}}

#define OPD_GLOBAL_LABEL(_name) \
    (ir_opd_t){.type = IR_OPD_GLOBAL_LABEL, .value = {.global_label_name  = _name}}
#define OPD_STRING(_str) \
    (ir_opd_t){.type = IR_OPD_STRING_LITERAL, .value = {.string_literal = _str}}
#define OPD_EXTERNAL_LABEL(_name) \
    (ir_opd_t){.type = IR_OPD_EXTERNAL_LABEL, .value = {.global_label_name  = _name}}
#define OPD_LOCAL_LABEL(_num) \
    (ir_opd_t){.type = IR_OPD_LOCAL_LABEL,  .value = {.local_label_number = _num}}

//——————————————————————————————————————————————————————————————————————————————

/* IR operations */

#define OP_ADD(_opd1, _opd2)  (ir_instr_t){IR_OPC_ADD,  _opd1, _opd2}
#define OP_SUB(_opd1, _opd2)  (ir_instr_t){IR_OPC_SUB,  _opd1, _opd2}
#define OP_IMUL(_opd1, _opd2) (ir_instr_t){IR_OPC_IMUL, _opd1, _opd2}
#define OP_IDIV(_opd1, _opd2) (ir_instr_t){IR_OPC_IDIV, _opd1, _opd2}
#define OP_MOV(_opd1, _opd2)  (ir_instr_t){IR_OPC_MOV,  _opd1, _opd2}
#define OP_LEA(_opd1, _opd2)  (ir_instr_t){IR_OPC_LEA,  _opd1, _opd2}
#define OP_TEST(_opd1, _opd2) (ir_instr_t){IR_OPC_TEST, _opd1, _opd2}

#define OP_PUSH(_opd) (ir_instr_t){IR_OPC_PUSH, _opd, {}}
#define OP_POP(_opd)  (ir_instr_t){IR_OPC_POP,  _opd, {}}
#define OP_JMP(_opd)  (ir_instr_t){IR_OPC_JMP,  _opd, {}}
#define OP_CALL(_opd) (ir_instr_t){IR_OPC_CALL, _opd, {}}
#define OP_JE(_opd)   (ir_instr_t){IR_OPC_JE,   _opd, {}}
#define OP_JNE(_opd)  (ir_instr_t){IR_OPC_JNE,  _opd, {}}

#define OP_GLOBAL_LABEL(_name) (ir_instr_t){IR_OPC_GLOBAL_LABEL, OPD_GLOBAL_LABEL(_name)}
#define OP_LOCAL_LABEL(_name)  (ir_instr_t){IR_OPC_LOCAL_LABEL,  OPD_LOCAL_LABEL(_name)}

#define OP_NOP     (ir_instr_t){IR_OPC_NOP,     {}, {}}
#define OP_RET     (ir_instr_t){IR_OPC_RET,     {}, {}}
#define OP_SYSCALL (ir_instr_t){IR_OPC_SYSCALL, {}, {}}

#define OP_FILDL(_opd)  (ir_instr_t){IR_OPC_FILDL,  _opd, {}}
#define OP_FSQRT        (ir_instr_t){IR_OPC_FSQRT,  {},   {}}
#define OP_FISTPL(_opd) (ir_instr_t){IR_OPC_FISTPL, _opd, {}}
#define OP_CQO          (ir_instr_t){IR_OPC_CQO,    {},   {}}

//——————————————————————————————————————————————————————————————————————————————

#define EMIT(_ir_instr) ir_emit_instr(&ctx->ir_buf, _ir_instr)

//——————————————————————————————————————————————————————————————————————————————

#endif // _IR_DSL_H__
