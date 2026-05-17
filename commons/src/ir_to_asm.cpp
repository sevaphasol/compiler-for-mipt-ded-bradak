#include "custom_assert.h"
#include "ir.h"

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_to_asm(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    //--------------------------------------------------------------------------

    RAW_ASM("section .text\n");
    RAW_ASM("global _start\n\n");

    size_t size = ctx->ir_buf.size / sizeof(ir_instr_t);

    ir_instr_t* ir_buf = (ir_instr_t*) ctx->ir_buf.data;

    for (size_t i = 0; i < size; i++) {
        ir_instr_t instr = ir_buf[i];
        IrOpAsmFuncsTable[instr.opc](ctx, &instr);
    }

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t zeroary_opcode_ir_to_asm(lang_ctx_t* ctx, ir_instr_t* instr)
{
    ASSERT(ctx);
    ASSERT(instr);

    //--------------------------------------------------------------------------

    ir_opc_t opcode = instr->opc;

    if (opcode == IR_OPC_LOCAL_LABEL) {
        RAW_ASM(".L%ld:\n", instr->opd1.value.local_label_number);
    } else if (opcode == IR_OPC_GLOBAL_LABEL) {
        RAW_ASM("%s:\n", instr->opd1.value.global_label_name);
    } else {
        TAB_ASM("%s\n", IrOpAsmNamesTable[instr->opc]);
    }

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t operand_ir_to_asm(lang_ctx_t* ctx, ir_opd_t* opd)
{
    ASSERT(ctx);

    //--------------------------------------------------------------------------

    switch(opd->type) {
        case IR_OPD_REGISTER: {
            RAW_ASM("%s", RegNames[opd->value.reg]);
            break;
        }
        case IR_OPD_IMMEDIATE: {
            RAW_ASM("%d", opd->value.imm);
            break;
        }
        case IR_OPD_MEMORY: {
            if (opd->value.offset < 0) {
                RAW_ASM("[rbp - %d]", -opd->value.offset)
            } else {
                RAW_ASM("[rbp + %d]", opd->value.offset);
            }
            break;
        }
        case IR_OPD_GLOBAL_MEMORY: {
            RAW_ASM("[rel __global_data + %d]", opd->value.offset);
            break;
        }
        case IR_OPD_STRING_LITERAL: {
            RAW_ASM("[rel __strings + \"%s\"]", opd->value.string_literal);
            break;
        }
        case IR_OPD_LOCAL_LABEL: {
            RAW_ASM(".L%ld", opd->value.local_label_number);
            break;
        }
        case IR_OPD_GLOBAL_LABEL: {
            RAW_ASM("%s", opd->value.global_label_name);
            break;
        }
        
        case IR_OPD_STDLIB_LABEL: {
            RAW_ASM("%s", opd->value.global_label_name);
            break;
        }

        default: {
            return LANG_ERROR;
        }
    }

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t unary_opcode_ir_to_asm(lang_ctx_t* ctx, ir_instr_t* instr)
{
    ASSERT(ctx);
    ASSERT(instr);

    //--------------------------------------------------------------------------

    TAB_ASM("%s ", IrOpAsmNamesTable[instr->opc]);
    operand_ir_to_asm(ctx, &instr->opd1);
    RAW_ASM("\n");

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t binary_opcode_ir_to_asm(lang_ctx_t* ctx, ir_instr_t* instr)
{
    ASSERT(ctx);
    ASSERT(instr);

    //--------------------------------------------------------------------------

    TAB_ASM("%s ", IrOpAsmNamesTable[instr->opc]);
    operand_ir_to_asm(ctx, &instr->opd1);
    RAW_ASM(", ");
    operand_ir_to_asm(ctx, &instr->opd2);
    RAW_ASM("\n");

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t idiv_ir_to_asm(lang_ctx_t* ctx, ir_instr_t* instr)
{
    ASSERT(ctx);
    ASSERT(instr);

    //--------------------------------------------------------------------------

    TAB_ASM("%s ", IrOpAsmNamesTable[instr->opc]);
    operand_ir_to_asm(ctx, &instr->opd2);
    RAW_ASM("\n");

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//==============================================================================

lang_status_t lib_func_ir_to_asm(lang_ctx_t* ctx, ir_instr_t* instr)
{
    ASSERT(ctx);
    ASSERT(instr);

    //--------------------------------------------------------------------------

    TAB_ASM("%s \n", IrOpAsmNamesTable[instr->opc]);

    //--------------------------------------------------------------------------

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————
