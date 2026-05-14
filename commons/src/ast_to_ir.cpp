#include "lang.h"
#include "custom_assert.h"
#include "ir.h"

#define _DSL_DEFINE_
#include "dsl.h"

#include "ir_dsl.h"

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_emit_instr(buffer_t* ir_buf, ir_instr_t ir_instr)
{
    ir_instr_t* ir_instr_ptr = &ir_instr;

    return buf_write(ir_buf, ir_instr_ptr, sizeof(ir_instr_t));
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t build_ir(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    lang_status_t status = LANG_SUCCESS;

    EMIT(OP_GLOBAL_LABEL("_start"));
    EMIT(OP_CALL(OPD_GLOBAL_LABEL("main")));
    EMIT(OP_MOV(OPD_REG(REG_RAX), OPD_IMM(60)));
    EMIT(OP_SYSCALL);

    return node_to_ir(ctx, ctx->tree);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t node_to_ir(lang_ctx_t* ctx,
                         node_t*     node)
{
    ASSERT(ctx);
    ASSERT(node);

    switch(node->value_type) {
        case OPERATOR:
            (*_OP(node).to_ir_func)(ctx, node);
            break;
        case IDENTIFIER:
            var_to_ir(ctx, node);
            break;
        case NUMBER:
            EMIT(OP_PUSH(OPD_IMM(node->value.number)));
            break;
        default:
            return LANG_ERROR;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t var_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    identifier_t var = _ID(node);

    EMIT(OP_PUSH(OPD_MEM(-var.addr)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t passing_func_params_to_ir(lang_ctx_t* ctx,
                                        node_t*     node,
                                        size_t      n_params)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* cur_param        = node;
    node_t* params[n_params] = {};

    for (size_t i = 0; i < n_params; i++) {
        ASSERT(cur_param);
        params[i] = cur_param;
        cur_param = cur_param->right;
    }

    for (size_t i = n_params; i != 0; i--) {
        node_to_ir(ctx, params[i - 1]->left);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t call_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* call         = node;
    node_t* func         = call->left;
    node_t* func_params  = func->left;

    identifier_t func_id = ctx->name_table.ids[func->value.id_index];

    if (func_id.type == FUNC_DEF) {
        passing_func_params_to_ir(ctx, func_params, func_id.n_params);

        EMIT(OP_CALL(OPD_GLOBAL_LABEL(func_id.name)));

        int32_t allocated_memory = VAR_SIZE * func_id.n_params;

        EMIT(OP_ADD(OPD_REG(REG_RSP), OPD_IMM(allocated_memory)));
        EMIT(OP_PUSH(OPD_REG(REG_RAX)));
        return LANG_SUCCESS;
    } 
    
     if (func_id.type == FUNC_DECL) {
        passing_func_params_to_ir(ctx, func_params, func_id.n_params);
        EMIT(OP_CALL(OPD_GLOBAL_LABEL(func_id.name)));

        int32_t allocated_memory = VAR_SIZE * func_id.n_params;

        EMIT(OP_ADD(OPD_REG(REG_RSP), OPD_IMM(allocated_memory)));
        EMIT(OP_PUSH(OPD_REG(REG_RAX)));
        return LANG_SUCCESS;
    }

    fprintf(stderr, "call_to_ir : unknown func_id type: %d\n", func_id.type);
    
    return LANG_ERROR;   
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t ir_emit_binary_operation(buffer_t* ir_buf,
                                       operator_code_t ast_opc,
                                       ir_opd_t opd1,
                                       ir_opd_t opd2)
{
    ASSERT(ir_buf);

    ir_opc_t ir_opc = IR_OPC_NOP;

    switch (ast_opc) {
        case ADD:
            ir_opc = IR_OPC_ADD;
            break;
        case SUB:
            ir_opc = IR_OPC_SUB;
            break;
        case MUL:
            ir_opc = IR_OPC_IMUL;
            break;
        case DIV:
            ir_opc = IR_OPC_IDIV;
            break;
        default:
            return LANG_ERROR;
    }

    return ir_emit_instr(ir_buf, {ir_opc, opd1, opd2});
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t binary_operation_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_to_ir(ctx, node->left);
    node_to_ir(ctx, node->right);

    EMIT(OP_POP(OPD_REG(REG_R11)));
    EMIT(OP_POP(OPD_REG(REG_R10)));

    ir_emit_binary_operation(&ctx->ir_buf, node->value.operator_code,
                             OPD_REG(REG_R10), OPD_REG(REG_R11));

    EMIT(OP_PUSH(OPD_REG(REG_R10)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t div_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_to_ir(ctx, node->left);
    node_to_ir(ctx, node->right);

    EMIT(OP_POP(OPD_REG(REG_R10)));
    EMIT(OP_POP(OPD_REG(REG_RAX)));
    EMIT(OP_CQO);

    ir_emit_binary_operation(&ctx->ir_buf, node->value.operator_code,
                             OPD_REG(REG_RAX), OPD_REG(REG_R10));

    EMIT(OP_PUSH(OPD_REG(REG_RAX)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t assignment_to_ir(lang_ctx_t* ctx, node_t* node) // TODO global vars
{
    ASSERT(ctx);
    ASSERT(node);

    identifier_t var = _ID(node->left);

    node_to_ir(ctx, node->right);

    EMIT(OP_POP(OPD_MEM(-var.addr)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t statement_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    if (node->left) {
        node_to_ir(ctx, node->left);
    }

    if (node->right) {
        node_to_ir(ctx, node->right);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t new_var_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t*       assignment = node->left;
    node_t*       var        = assignment->left;
    identifier_t* var_id     = &_ID(var);

    if (!var_id->addr) {
        var_id->addr = VAR_SIZE * (++ctx->n_locals);
    }

    assignment_to_ir(ctx, assignment);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t receiving_func_params_to_ir(lang_ctx_t* ctx,
                                          node_t*     node,
                                          size_t      n_params)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* cur_param = node;

    for (size_t i = 0; i < n_params; i++) {
        _ID(cur_param->left->left).addr = -VAR_SIZE * ((int) i + 2);
        cur_param = cur_param->right;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t new_func_to_ir(lang_ctx_t* ctx,
                             node_t*     node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t*      func_declaration = node;
    node_t*      func             = func_declaration->left;
    node_t*      func_params      = func->left;
    node_t*      func_body        = func->right;
    identifier_t func_id          = _ID(func);

    EMIT(OP_GLOBAL_LABEL(func_id.name));
    EMIT(OP_PUSH(OPD_REG(REG_RBP)));
    EMIT(OP_MOV(OPD_REG(REG_RBP), OPD_REG(REG_RSP)));

    size_t sub_rsp_position = ctx->ir_buf.size;

    ctx->ir_buf.size += sizeof(ir_instr_t);

    if (func_params) {
        receiving_func_params_to_ir(ctx, func_params, func_id.n_params);
    }

    node_to_ir(ctx, func_body);

    size_t cur_position = ctx->ir_buf.size;
    int32_t allocated_memory = VAR_SIZE * ctx->n_locals;

    ctx->ir_buf.size = sub_rsp_position;
    EMIT(OP_SUB(OPD_REG(REG_RSP), OPD_IMM(allocated_memory)));

    ctx->ir_buf.size = cur_position;

    ctx->n_locals = 0;
    ctx->level    = 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t func_decl_to_ir(lang_ctx_t* ctx,
                             node_t*     node)
{
    ASSERT(ctx);
    ASSERT(node);

    // EMPTY

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t return_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* ret       = node;
    node_t* ret_value = ret->left;

    if (ret_value) {
        node_to_ir(ctx, ret_value);
        EMIT(OP_POP(OPD_REG(REG_RAX)));
    }

    int32_t allocated_memory = VAR_SIZE * ctx->n_locals;

    EMIT(OP_ADD(OPD_REG(REG_RSP), OPD_IMM(allocated_memory)));
    EMIT(OP_POP(OPD_REG(REG_RBP)));
    EMIT(OP_RET);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t if_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* statement = node->left;
    node_t* body      = node->right;

    node_to_ir(ctx, statement);

    EMIT(OP_POP(OPD_REG(REG_RAX)));
    EMIT(OP_TEST(OPD_REG(REG_RAX), OPD_REG(REG_RAX)));

    size_t label_num = ctx->n_labels++;

    EMIT(OP_JE(OPD_LOCAL_LABEL(label_num)));
    node_to_ir(ctx, body);
    EMIT(OP_LOCAL_LABEL(label_num));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t while_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* statement = node->left;
    node_t* body      = node->right;

    size_t check_label_num = ctx->n_labels++;
    EMIT(OP_JMP(OPD_LOCAL_LABEL(check_label_num)));

    size_t body_label_num = ctx->n_labels++;
    EMIT(OP_LOCAL_LABEL(body_label_num));
    node_to_ir(ctx, body);

    EMIT(OP_LOCAL_LABEL(check_label_num));
    node_to_ir(ctx, statement);

    EMIT(OP_POP(OPD_REG(REG_RAX)));
    EMIT(OP_TEST(OPD_REG(REG_RAX), OPD_REG(REG_RAX)));
    EMIT(OP_JNE(OPD_LOCAL_LABEL(body_label_num)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t in_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    identifier_t var = _ID(node->left->left);

    EMIT(OP_IN);
    EMIT(OP_MOV(OPD_MEM(-var.addr), OPD_REG(REG_RAX)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t out_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    node_t* arg = node->left->left;

    node_to_ir(ctx, arg);

    EMIT(OP_OUT);
    EMIT(OP_ADD(OPD_REG(REG_RSP), OPD_IMM(VAR_SIZE)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t exit_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    ir_opd_t return_val = OPD_REG(REG_RAX);
    ir_opd_t exit_code  = OPD_IMM(60);

    EMIT(OP_MOV(OPD_REG(REG_RAX), OPD_IMM(60)));
    EMIT(OP_SYSCALL);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t sqrt_to_ir(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    identifier_t var = _ID(node->left->left);

    EMIT(OP_FILDL(OPD_MEM(-var.addr)));
    EMIT(OP_FSQRT);
    EMIT(OP_FISTPL(OPD_MEM(-var.addr)));
    EMIT(OP_PUSH(OPD_MEM(-var.addr)));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"
