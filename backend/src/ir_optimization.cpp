#include "lang.h"
#include "ir.h"
#include "ir_list.h"
#include "encode_utils.h"
#include "custom_assert.h"
#include "buffer.h"
#include "graph_dump.h"

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t ir_buf_to_ir_list        (lang_ctx_t* ctx);
static lang_status_t ir_list_to_ir_buf        (lang_ctx_t* ctx);
static lang_status_t remove_push_pop          (lang_ctx_t* ctx);

static lang_status_t replace_push_pop_with_mov(ir_node_t* push_node,
                                               ir_node_t* pop_node);

static lang_status_t remove_trivial_arithmetic(lang_ctx_t* ctx);

static bool ir_instr_is_trivial_arithmetic(ir_instr_t* ir_instr);

//——————————————————————————————————————————————————————————————————————————————

lang_status_t optimize_ir(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    VERIFY(ir_buf_to_ir_list(ctx),                  return LANG_ERROR);
    VERIFY(remove_push_pop          (ctx),          return LANG_ERROR);
    // VERIFY(remove_trivial_arithmetic(ctx),          return LANG_ERROR);
    VERIFY(ir_list_to_ir_buf        (ctx),          return LANG_ERROR);
    VERIFY(ir_list_dtor             (ctx->ir_list), return LANG_ERROR);

    VERIFY(ir_buffer_graph_dump(ctx, (ir_instr_t*) &ctx->ir_buf.data),
           return LANG_ERROR);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t remove_push_pop(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    ir_node_t* cur_node = ctx->ir_list;

    while(cur_node && cur_node->next) {
        ir_instr_t cur_instr  = cur_node->instr;
        ir_instr_t next_instr = cur_node->next->instr;

        bool is_push_pop  = cur_instr.opc  == IR_OPC_PUSH &&
                            next_instr.opc == IR_OPC_POP;

        bool cur_is_mem  = is_memory_operand(cur_instr.opd1.type);
        bool next_is_mem = is_memory_operand(next_instr.opd1.type);
        bool is_valid_mov = !(cur_is_mem && next_is_mem);

        if (is_push_pop && is_valid_mov) {
            replace_push_pop_with_mov(cur_node, cur_node->next);
        }

        cur_node = cur_node->next;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t replace_push_pop_with_mov(ir_node_t* push_node,
                                        ir_node_t* pop_node)
{
    ir_opd_t src = push_node->instr.opd1;
    ir_opd_t dst = pop_node->instr.opd1;

    cut_node(push_node);

    ir_instr_t* mov = &pop_node->instr;

    mov->opc  = IR_OPC_MOV;
    mov->opd1 = dst;
    mov->opd2 = src;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t remove_trivial_arithmetic(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    ir_node_t* cur_node = ctx->ir_list;

    while(cur_node) {
        if (ir_instr_is_trivial_arithmetic(&cur_node->instr)) {
            cut_node(cur_node);
        }

        cur_node = cur_node->next;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

bool ir_instr_is_trivial_arithmetic(ir_instr_t* ir_instr)
{
    ASSERT(ir_instr);

    ir_opc_t opc     = ir_instr->opc;

    bool opd1_is_num = ir_instr->opd1.type == IR_OPD_IMMEDIATE;
    bool opd2_is_num = ir_instr->opd2.type == IR_OPD_IMMEDIATE;

    int32_t opd1_num = ir_instr->opd1.value.imm;
    int32_t opd2_num = ir_instr->opd2.value.imm;

    bool case1 = (opc == IR_OPC_ADD || opc == IR_OPC_SUB) &&
                 ((opd1_is_num && opd1_num == 0) ||
                  (opd2_is_num && opd2_num == 0));

    bool case2 = (opc == IR_OPC_IMUL) &&
                 ((opd1_is_num && opd1_num == 1) ||
                  (opd2_is_num && opd2_num == 1));

    bool case3 = (opc == IR_OPC_IDIV) &&
                 (opd1_is_num && opd1_num == 1);

    return case1 || case2 || case3;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_buf_to_ir_list(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    ir_instr_t* ir_buf = (ir_instr_t*) ctx->ir_buf.data;
    size_t      size   = ctx->ir_buf.size / sizeof(ir_instr_t);

    ir_node_t* ir_list = new_ir_node();
    VERIFY(!ir_list, return LANG_STD_ALLOCATE_ERROR);

    ir_list->instr = ir_buf[0];
    ir_node_t* cur_node = ir_list;

    for (size_t i = 1; i < size; i++) {
        ir_node_t* next_node = new_ir_node();
        VERIFY(!next_node, return LANG_STD_ALLOCATE_ERROR);

        cur_node->next   = next_node;

        next_node->prev  = cur_node;
        next_node->instr = ir_buf[i];

        cur_node = next_node;
    }

    ctx->ir_list = ir_list;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_list_to_ir_buf(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    ctx->ir_buf.size = 0;

    ir_node_t* cur_node = ctx->ir_list;

    while(cur_node) {
        buf_write(&ctx->ir_buf, &cur_node->instr, sizeof(ir_instr_t));

        cur_node = cur_node->next;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
