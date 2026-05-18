#ifndef _ENCODE_UTILS_H__
#define _ENCODE_UTILS_H__

//——————————————————————————————————————————————————————————————————————————————

#define BIN_BUF_INIT_CAPACITY 4096
#define TABLE_INIT_CAPACITY   10
#define REX_PREFIX            0b0100
#define OPERANDS_64_BIT       1

//——————————————————————————————————————————————————————————————————————————————

#include "lang_status.h"
#include "bin_instr.h"
#include "ir.h"

bool reg_expand(reg_t reg);
reg_t trim_reg(reg_t reg);
void set_mod_and_disp_size(int32_t disp, uint8_t* mod, uint8_t* disp_size);
modrm_t build_modrm(uint8_t mod, uint8_t reg, uint8_t rm);
rex_t build_rex(uint8_t r, uint8_t b);
lang_status_t build_rex_rr(bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_modrm_rr(bin_instr_t* bin_instr, ir_instr_t*  ir_instr);
lang_status_t build_rex_rm(bin_instr_t* bin_instr, ir_instr_t*  ir_instr);
lang_status_t build_modrm_rm(lang_ctx_t* ctx, bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_rex_mr(bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_modrm_mr(lang_ctx_t* ctx, bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_rex_ri(bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_modrm_and_imm_ri(bin_instr_t* bin_instr,
                                     ir_instr_t*  ir_instr,
                                     uint8_t      modrm_reg);
lang_status_t build_rex_mi(bin_instr_t* bin_instr, ir_instr_t* ir_instr);
lang_status_t build_modrm_and_imm_mi(lang_ctx_t* ctx,
                                     bin_instr_t* bin_instr,
                                     ir_instr_t*  ir_instr,
                                     uint8_t      modrm_reg);
bool is_memory_operand(ir_opd_type_t type);
ir_instr_type_t get_ir_instr_type(ir_instr_t* ir_instr);
bool imm_fits_i32(int64_t imm);
void warn_imm64_not_implemented(const char* instr_name, int64_t imm);

//——————————————————————————————————————————————————————————————————————————————

#endif // _ENCODE_UTILS_H__
