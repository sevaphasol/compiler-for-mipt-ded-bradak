#ifndef _IR_OPERANDS_H__
#define _IR_OPERANDS_H__

//——————————————————————————————————————————————————————————————————————————————

#include <stdint.h>
#include "x86_64_codes.h"

//——————————————————————————————————————————————————————————————————————————————

enum ir_opd_type_t {
    IR_OPD_NAN          = 0,
    IR_OPD_REGISTER     = 1,
    IR_OPD_MEMORY       = 2,
    IR_OPD_IMMEDIATE    = 3,
    IR_OPD_STDLIB_LABEL = 4,
    IR_OPD_GLOBAL_LABEL = 5,
    IR_OPD_LOCAL_LABEL  = 6,
    IR_OPD_GLOBAL_MEMORY = 7,
    IR_OPD_STRING_LITERAL = 8,
};

//——————————————————————————————————————————————————————————————————————————————

union ir_opd_value_t {
    reg_t       reg;
    int32_t     offset;
    int32_t     imm;
    const char* global_label_name;
    const char* string_literal;
    size_t      local_label_number;
};

//——————————————————————————————————————————————————————————————————————————————

struct ir_opd_t {
    ir_opd_type_t  type;
    ir_opd_value_t value;
};

//——————————————————————————————————————————————————————————————————————————————

ir_opd_t operand_register(reg_t reg);
ir_opd_t operand_memory(int addr);
ir_opd_t operand_immersive(int64_t imm);
ir_opd_t operand_global_label(const char* name);
ir_opd_t operand_local_label(uint64_t label_number);

//——————————————————————————————————————————————————————————————————————————————

#endif // _IR_OPERANDS_H__
