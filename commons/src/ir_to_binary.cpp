#include <string.h>
#include "ir.h"
#include "custom_assert.h"
#include "x86_64_codes.h"
#include "x86_64_opc_tables.h"
#include "buffer.h"
#include "fixup_table.h"
#include "encode_utils.h"
#include "lib_call_funcs.h"

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_instr(lang_ctx_t*  ctx,
                           ir_instr_t*  ir_instr,
                           bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    return IrOpEncodeFuncsTable[ir_instr->opc](ctx, ir_instr, bin_instr);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t emit_bin_instr(buffer_t* bin_buf, const bin_instr_t* instr)
{
    ASSERT(bin_buf);
    ASSERT(instr);

    if (instr->info.has_rex)
        buf_write(bin_buf, &instr->rex, sizeof(instr->rex));

    buf_write(bin_buf, &instr->opc, instr->info.opcode_size);

    if (instr->info.has_modrm)
        buf_write(bin_buf, &instr->modrm, sizeof(instr->modrm));

    if (instr->info.has_disp)
        buf_write(bin_buf, &instr->disp, instr->info.disp_size);

    if (instr->info.has_imm)
        buf_write(bin_buf, &instr->imm, instr->info.imm_size);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_to_binary(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    size_t size = ctx->ir_buf.size / sizeof(ir_instr_t);
    ir_instr_t* ir_buf = (ir_instr_t*) ctx->ir_buf.data;

    for (size_t i = 0; i < size; i++) {
        ir_instr_t ir_instr = ir_buf[i];
        bin_instr_t bin_instr = {};

        bin_instr.info.opcode_size = 1;
        encode_instr(ctx, &ir_instr, &bin_instr);

        if (ir_instr.opc != IR_OPC_GLOBAL_LABEL &&
            ir_instr.opc != IR_OPC_LOCAL_LABEL) {
            emit_bin_instr(&ctx->bin_buf, &bin_instr);
        }
    }

    fixup_table_apply(&ctx->fixups, &ctx->label_table, &ctx->bin_buf);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_nop(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc              = X86_64_NOP;
    bin_instr->info.opcode_size = 1;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_binary_op(ir_instr_t*                    ir_instr,
                               bin_instr_t*                   bin_instr,
                               const bin_instr_x86_64_info_t* info_table)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);
    ASSERT(info_table);

    ir_instr_type_t ir_instr_type = get_ir_instr_type(ir_instr);

    bin_instr_x86_64_info_t info = info_table[ir_instr_type];

    uint8_t modrm_reg = info.modrm_reg;

    bin_instr->opc = info.opc;

    switch(ir_instr_type) {
        case IR_INSTR_TYPE_REG_REG: {
            build_rex_rr  (bin_instr, ir_instr);
            build_modrm_rr(bin_instr, ir_instr);
            break;
        }
        case IR_INSTR_TYPE_REG_MEM: {
            build_rex_rm  (bin_instr, ir_instr);
            build_modrm_rm(bin_instr, ir_instr);
            break;
        }
        case IR_INSTR_TYPE_MEM_REG: {
            build_rex_mr  (bin_instr, ir_instr);
            build_modrm_mr(bin_instr, ir_instr);
            break;
        }
        case IR_INSTR_TYPE_REG_IMM: {
            build_rex_ri          (bin_instr, ir_instr);
            build_modrm_and_imm_ri(bin_instr, ir_instr, modrm_reg);
            break;
        }
        case IR_INSTR_TYPE_MEM_IMM: {
            build_rex_mi          (bin_instr, ir_instr);
            build_modrm_and_imm_mi(bin_instr, ir_instr, modrm_reg);
            break;
        }
        default: {
            return LANG_ERROR;
        }
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_add(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    return encode_binary_op(ir_instr, bin_instr, AddOpcInfo);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_sub(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    return encode_binary_op(ir_instr, bin_instr, SubOpcInfo);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_mov(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    return encode_binary_op(ir_instr, bin_instr, MovOpcInfo);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_test(lang_ctx_t*  ctx,
                          ir_instr_t*  ir_instr,
                          bin_instr_t* bin_instr)
{
    return encode_binary_op(ir_instr, bin_instr, TestOpcInfo);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_imul(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    bin_instr->info.opcode_size = 2;

    encode_binary_op(ir_instr, bin_instr, ImulOpcInfo);

    if (get_ir_instr_type(ir_instr) == IR_INSTR_TYPE_REG_REG) {
        uint8_t tmp          = bin_instr->modrm.rm;
        bin_instr->modrm.rm  = bin_instr->modrm.reg;
        bin_instr->modrm.reg = (uint8_t)(tmp & 0b11);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_idiv_rr(ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    reg_t divider = ir_instr->opd2.value.reg;

    bin_instr->rex = build_rex(REX_R_UNUSED, reg_expand(divider));
    bin_instr->info.has_rex = true;

    bin_instr->modrm = build_modrm(X86_64_MOD_RR,
                                   X86_64_DIV_MODRM_REG,
                                   trim_reg(divider));

    bin_instr->info.has_modrm = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_idiv_ri(ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->rex = build_rex(REX_R_UNUSED, REX_B_UNUSED);
    bin_instr->info.has_rex = true;

    int32_t disp = ir_instr->opd2.value.offset;

    uint8_t mod       = 0;
    uint8_t disp_size = 0;

    set_mod_and_disp_size(disp, &mod, &disp_size);

    bin_instr->modrm = build_modrm(mod, X86_64_DIV_MODRM_REG,
                                        trim_reg(REG_RBP));

    bin_instr->info.has_modrm = true;
    bin_instr->info.has_disp  = true;
    bin_instr->disp           = disp;
    bin_instr->info.disp_size = disp_size;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_idiv(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc = X86_64_IDIV_OPCODE;

    ir_instr_type_t ir_instr_type = get_ir_instr_type(ir_instr);

    if         (ir_instr_type == IR_INSTR_TYPE_REG_REG) {
        return encode_idiv_rr(ir_instr, bin_instr);
    }  else if (ir_instr_type == IR_INSTR_TYPE_REG_IMM) {
        return encode_idiv_ri(ir_instr, bin_instr);
    }

    return LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_push_or_pop_reg(lang_ctx_t*     ctx,
                                     ir_instr_t*     ir_instr,
                                     bin_instr_t*    bin_instr,
                                     x86_64_opcode_t opc)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    reg_t reg = ir_instr->opd1.value.reg;

    if (reg >= REG_R8) {
        bin_instr->rex = build_rex(REX_R_UNUSED, 1);
        bin_instr->info.has_rex = true;
    }

    bin_instr->opc = (uint16_t) (opc + trim_reg(reg));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_push_or_pop_mem(lang_ctx_t*        ctx,
                                     ir_instr_t*        ir_instr,
                                     bin_instr_t*       bin_instr,
                                     x86_64_opcode_t    opc,
                                     x86_64_modrm_reg_t modrm_reg)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc  = opc;
    int32_t    disp = ir_instr->opd1.value.offset;

    uint8_t disp_size = 0;
    uint8_t mod       = 0;

    set_mod_and_disp_size(disp, &mod, &disp_size);

    bin_instr->info.has_disp  = true;
    bin_instr->disp           = disp;
    bin_instr->info.disp_size = disp_size;

    bin_instr->modrm          = build_modrm(mod, modrm_reg, REG_RBP);
    bin_instr->info.has_modrm = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_push_imm(lang_ctx_t*  ctx,
                              ir_instr_t*  ir_instr,
                              bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc           = X86_64_PUSH_I_OPCODE;
    bin_instr->info.has_imm  = true;
    bin_instr->info.imm_size = 4;
    bin_instr->imm           = ir_instr->opd1.value.imm;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_push(lang_ctx_t*  ctx,
                          ir_instr_t*  ir_instr,
                          bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    switch(ir_instr->opd1.type) {
        case IR_OPD_REGISTER: {
            return encode_push_or_pop_reg(ctx, ir_instr, bin_instr,
                                          X86_64_PUSH_R_OPCODE);
        }
        case IR_OPD_MEMORY: {
            return encode_push_or_pop_mem(ctx, ir_instr, bin_instr,
                                          X86_64_PUSH_M_OPCODE,
                                          X86_64_PUSH_M_MODRM_REG);
        }
        case IR_OPD_IMMEDIATE: {
            return encode_push_imm(ctx, ir_instr, bin_instr);
        }
        default: {
            return LANG_ERROR;
        }
    }
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_pop(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    switch(ir_instr->opd1.type) {
        case IR_OPD_REGISTER: {
            return encode_push_or_pop_reg(ctx, ir_instr, bin_instr,
                                          X86_64_POP_R_OPCODE);
        }
        case IR_OPD_MEMORY: {
            return encode_push_or_pop_mem(ctx, ir_instr, bin_instr,
                                          X86_64_POP_M_OPCODE,
                                          X86_64_POP_M_MODRM_REG);
        }
        default:
            return LANG_ERROR;
    }
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_lib_func(lang_ctx_t*  ctx,
                              ir_instr_t*  ir_instr,
                              bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc = X86_64_CALL_REL32_OPCODE;
    bin_instr->info.has_imm = true;
    bin_instr->info.imm_size = 4;
    bin_instr->imm = 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_call(lang_ctx_t*  ctx,
                          ir_instr_t*  ir_instr,
                          bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    if (ir_instr->opd1.type == IR_OPD_STDLIB_LABEL) {
        const char* name = ir_instr->opd1.value.global_label_name;
        add_lib_call_request(&ctx->lib_calls_table,
                         name,
                         ctx->bin_buf.size + 1);

        return encode_lib_func(ctx, ir_instr, bin_instr);
    }

    ASSERT(ir_instr->opd1.type == IR_OPD_GLOBAL_LABEL);

    const char* name = ir_instr->opd1.value.global_label_name;

    bin_instr->opc = X86_64_CALL_REL32_OPCODE;
    bin_instr->info.has_imm = true;
    bin_instr->info.imm_size = 4;
    bin_instr->imm = 0;

    add_fixup(&ctx->fixups, name, 0, (uint32_t) (ctx->bin_buf.size + 1));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_ret(lang_ctx_t*  ctx,
                         ir_instr_t*  ir_instr,
                         bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc = X86_64_RET;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_syscall(lang_ctx_t*  ctx,
                             ir_instr_t*  ir_instr,
                             bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc = X86_64_SYSCALL_OPCODE;
    bin_instr->info.opcode_size = 2;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_jumps(lang_ctx_t*  ctx,
                           ir_instr_t*  ir_instr,
                           bin_instr_t* bin_instr,
                           uint16_t     opc,
                           size_t       opc_size)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    ASSERT(ir_instr->opd1.type == IR_OPD_LOCAL_LABEL);

    size_t label_num = ir_instr->opd1.value.local_label_number;

    bin_instr->opc = opc;
    bin_instr->info.opcode_size = opc_size;

    bin_instr->info.has_imm = true;
    bin_instr->info.imm_size = 4;
    bin_instr->imm = 0;

    add_fixup(&ctx->fixups, NULL, label_num, (uint32_t) (ctx->bin_buf.size + opc_size));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_jmp(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    uint16_t opc      = X86_64_JMP_REL32_OPCODE;
    uint8_t  opc_size = 1;

    return encode_jumps(ctx, ir_instr, bin_instr, opc, opc_size);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_je(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    uint16_t opc      = X86_64_JE_REL32_OPCODE;
    uint8_t  opc_size = 2;

    return encode_jumps(ctx, ir_instr, bin_instr, opc, opc_size);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_jne(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    uint16_t opc      = X86_64_JNE_REL32_OPCODE;
    uint8_t  opc_size = 2;

    return encode_jumps(ctx, ir_instr, bin_instr, opc, opc_size);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_local_label(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    size_t label_num = ir_instr->opd1.value.local_label_number;
    size_t curr_addr = ctx->bin_buf.size;

    label_table_add_local(&ctx->label_table, label_num, curr_addr);

    return LANG_SUCCESS;

}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_global_label(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    const char* name = ir_instr->opd1.value.global_label_name;
    size_t curr_addr = ctx->bin_buf.size;

    label_table_add_global(&ctx->label_table, name, curr_addr);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_fpu(lang_ctx_t*        ctx,
                         ir_instr_t*        ir_instr,
                         bin_instr_t*       bin_instr,
                         x86_64_opcode_t    opc,
                         x86_64_modrm_reg_t modrm_reg)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc = opc;

    int32_t disp = ir_instr->opd1.value.offset;

    uint8_t disp_size = 0;
    uint8_t mod       = 0;

    set_mod_and_disp_size(disp, &mod, &disp_size);

    bin_instr->info.has_disp  = true;
    bin_instr->disp           = disp;
    bin_instr->info.disp_size = disp_size;

    bin_instr->modrm          = build_modrm(mod, modrm_reg, REG_RBP);
    bin_instr->info.has_modrm = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_fildl(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    return encode_fpu(ctx, ir_instr, bin_instr,
                      X86_64_FILDL_OPCODE, X86_64_FILDL_MODRM_REG);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_fistpl(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    return encode_fpu(ctx, ir_instr, bin_instr,
                      X86_64_FISTPL_OPCODE, X86_64_FISTPL_MODRM_REG);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_fsqrt(lang_ctx_t* ctx, ir_instr_t* ir_instr, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc              = X86_64_FSQRT_OPCODE;
    bin_instr->info.opcode_size = 2;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t encode_cqo(lang_ctx_t* ctx,  ir_instr_t*  ir_instr, bin_instr_t* bin_instr)
{

    ASSERT(ctx);
    ASSERT(ir_instr);
    ASSERT(bin_instr);

    bin_instr->opc              = X86_64_CQO_OPCODE;
    bin_instr->info.opcode_size = 2;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
