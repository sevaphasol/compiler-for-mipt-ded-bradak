#include "encode_utils.h"
#include "custom_assert.h"
#include "ir_operands.h"

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t build_memory_modrm(lang_ctx_t*    ctx,
                                        bin_instr_t*   bin_instr,
                                        ir_opd_t       mem_opd,
                                        uint8_t        reg_field);
static uint32_t get_disp_offset(lang_ctx_t* ctx, bin_instr_t* bin_instr);
static uint32_t get_disp_rel_base(lang_ctx_t* ctx, bin_instr_t* bin_instr);

bool reg_expand(reg_t reg)
{
    return reg >= REG_R8;
}

//——————————————————————————————————————————————————————————————————————————————

reg_t trim_reg(reg_t reg)
{
    return (reg_t) (reg & 0b111);
}

//——————————————————————————————————————————————————————————————————————————————

void set_mod_and_disp_size(int32_t disp, uint8_t* mod, uint8_t* disp_size)
{
    ASSERT(mod);
    ASSERT(disp_size);

    if (disp == 0) {
        *mod       = X86_64_MOD_M_NO_DISP;
        *disp_size = 0;
    } else if (disp >= -128 && disp <= 127) {
        *mod       = X86_64_MOD_M_DISP8;
        *disp_size = 1;
    } else {
        *mod       = X86_64_MOD_M_DISP32;
        *disp_size = 4;
    }
}

//——————————————————————————————————————————————————————————————————————————————

modrm_t build_modrm(uint8_t mod, uint8_t reg, uint8_t rm)
{
    return (modrm_t) {
        .rm  = (uint8_t)(rm  & 0b111),
        .reg = (uint8_t)(reg & 0b111),
        .mod = (uint8_t)(mod & 0b11)
    };
}

//——————————————————————————————————————————————————————————————————————————————

rex_t build_rex(uint8_t r, uint8_t b)
{
    return (rex_t) {
        .b      = (uint8_t)(b & 1),
        .x      = 0, // SIB is never used in this implementation
        .r      = (uint8_t)(r & 1),
        .w      = 1, // Always 64 bit operands in this implementation
        .prefix = 0b0100 // fixed constant
    };
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_rex_rr(bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst = ir_instr->opd1.value.reg;
    reg_t src = ir_instr->opd2.value.reg;

    bin_instr->rex = build_rex(reg_expand(src), reg_expand(dst));

    bin_instr->info.has_rex = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_modrm_rr(bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst = ir_instr->opd1.value.reg;
    reg_t src = ir_instr->opd2.value.reg;

    bin_instr->modrm = build_modrm(X86_64_MOD_RR, trim_reg(src),
                                                  trim_reg(dst));

    bin_instr->info.has_modrm = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_rex_rm(bin_instr_t* bin_instr, ir_instr_t*  ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst = ir_instr->opd1.value.reg;

    bin_instr->rex = build_rex(reg_expand(dst), REX_B_UNUSED);

    bin_instr->info.has_rex = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_modrm_rm(lang_ctx_t* ctx, bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(ctx);
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    return build_memory_modrm(ctx, bin_instr, ir_instr->opd2,
                              trim_reg(ir_instr->opd1.value.reg));
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_rex_mr(bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst = ir_instr->opd2.value.reg;

    bin_instr->rex = build_rex(reg_expand(dst), REX_B_UNUSED);

    bin_instr->info.has_rex = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_modrm_mr(lang_ctx_t* ctx, bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(ctx);
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    return build_memory_modrm(ctx, bin_instr, ir_instr->opd1,
                              trim_reg(ir_instr->opd2.value.reg));
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_rex_ri(bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst = ir_instr->opd1.value.reg;

    bin_instr->rex = build_rex(REX_R_UNUSED, reg_expand(dst));

    bin_instr->info.has_rex = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_modrm_and_imm_ri(bin_instr_t* bin_instr,
                                     ir_instr_t*  ir_instr,
                                     uint8_t      modrm_reg)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    reg_t dst    = ir_instr->opd1.value.reg;
    number_t imm = ir_instr->opd2.value.imm;

    bin_instr->modrm = build_modrm(X86_64_MOD_RI, modrm_reg, trim_reg(dst));

    bin_instr->info.has_modrm   = true;
    bin_instr->info.has_imm     = true;
    bin_instr->info.imm_size    = 4;
    bin_instr->imm              = imm;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_rex_mi(bin_instr_t* bin_instr, ir_instr_t* ir_instr)
{
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    bin_instr->rex = build_rex(REX_R_UNUSED, REX_B_UNUSED);
    bin_instr->info.has_rex = true;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_modrm_and_imm_mi(lang_ctx_t* ctx,
                                     bin_instr_t* bin_instr,
                                     ir_instr_t*  ir_instr,
                                     uint8_t      modrm_reg)
{
    ASSERT(ctx);
    ASSERT(bin_instr);
    ASSERT(ir_instr);

    bin_instr->info.has_imm     = true;
    bin_instr->info.imm_size    = 4;
    bin_instr->imm              = ir_instr->opd2.value.imm;

    VERIFY(build_memory_modrm(ctx, bin_instr, ir_instr->opd1, modrm_reg),
           return LANG_ERROR);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

bool is_memory_operand(ir_opd_type_t type)
{
    return type == IR_OPD_STFRAME_MEMORY ||
           type == IR_OPD_GLOBAL_MEMORY  ||
           type == IR_OPD_ARR_OFFSET_MEMORY;
}

//——————————————————————————————————————————————————————————————————————————————

ir_instr_type_t get_ir_instr_type(ir_instr_t* ir_instr)
{
    ASSERT(ir_instr);

	if (ir_instr->opd1.type == IR_OPD_REGISTER) {
		switch (ir_instr->opd2.type) {
			case IR_OPD_REGISTER: return IR_INSTR_TYPE_REG_REG;
			case IR_OPD_IMMEDIATE: return IR_INSTR_TYPE_REG_IMM;
			default: return is_memory_operand(ir_instr->opd2.type) ?
				IR_INSTR_TYPE_REG_MEM :
				IR_INSTR_TYPE_UNDEFINED;
		}
		} else if (is_memory_operand(ir_instr->opd1.type)) {
			switch (ir_instr->opd2.type) {
				case IR_OPD_REGISTER: return IR_INSTR_TYPE_MEM_REG;
				case IR_OPD_IMMEDIATE: return IR_INSTR_TYPE_MEM_IMM;
				default: return IR_INSTR_TYPE_UNDEFINED;
			}
		}

	    return IR_INSTR_TYPE_UNDEFINED;
}

//——————————————————————————————————————————————————————————————————————————————

static uint32_t get_disp_offset(lang_ctx_t* ctx, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(bin_instr);

    return (uint32_t) (ctx->bin_buf.size +
                       (bin_instr->info.has_rex ? 1 : 0) +
                       bin_instr->info.opcode_size +
                       1);
}

//——————————————————————————————————————————————————————————————————————————————

static uint32_t get_disp_rel_base(lang_ctx_t* ctx, bin_instr_t* bin_instr)
{
    ASSERT(ctx);
    ASSERT(bin_instr);

    return (uint32_t) (get_disp_offset(ctx, bin_instr) +
                       bin_instr->info.disp_size +
                       (bin_instr->info.has_imm ? bin_instr->info.imm_size : 0));
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t build_memory_modrm(lang_ctx_t*    ctx,
                                        bin_instr_t*   bin_instr,
                                        ir_opd_t       mem_opd,
                                        uint8_t        reg_field)
{
    ASSERT(ctx);
    ASSERT(bin_instr);

    bin_instr->info.has_modrm = true;

    if (mem_opd.type == IR_OPD_GLOBAL_MEMORY) {
        bin_instr->modrm = build_modrm(X86_64_MOD_M_NO_DISP, reg_field, 5);
        bin_instr->info.has_disp = true;
        bin_instr->info.disp_size = 4;
        bin_instr->disp = 0;

        add_fixup(&ctx->global_data_fixups, NULL,
                  (size_t) mem_opd.value.offset,
                  get_disp_offset(ctx, bin_instr),
                  get_disp_rel_base(ctx, bin_instr));

        return LANG_SUCCESS;
    }

    reg_t base_reg = REG_INV;
    if (mem_opd.type == IR_OPD_STFRAME_MEMORY) {
        base_reg = REG_RBP;
    } else if (mem_opd.type == IR_OPD_ARR_OFFSET_MEMORY) {
        base_reg = REG_RBX;
    } else {
        return LANG_ERROR;
    }

    int32_t disp = mem_opd.value.offset;
    uint8_t mod = 0;
    uint8_t disp_size = 0;

    set_mod_and_disp_size(disp, &mod, &disp_size);

    if (base_reg == REG_RBP && disp_size == 0) {
        mod = X86_64_MOD_M_DISP8;
        disp_size = 1;
    }

    bin_instr->modrm = build_modrm(mod, reg_field, trim_reg(base_reg));
    bin_instr->disp = disp;
    bin_instr->info.disp_size = disp_size;
    bin_instr->info.has_disp = disp_size != 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
