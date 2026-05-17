#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lang.h"
#include "custom_assert.h"
#include "graph_dump.h"
#include "node_allocator.h"
#include "io_interaction.h"

//———————————————————————————————————————————————————————————————————//

#define _DSL_DEFINE_
#include "dsl.h"

#define _PUT(...)    fprintf(ctx->output_file, ##__VA_ARGS__);
#define _SRC(_node_) src_node(ctx, _node_);

//———————————————————————————————————————————————————————————————————//

lang_status_t src_node(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    VERIFY(!node,
           return LANG_SRC_NODE_ERROR);

    //-------------------------------------------------------------------//

    switch (node->value_type)
    {
        case NUMBER:
        {
            _PUT("%d", node->value.number);
            break;
        }

        case STRING:
        {
            _PUT("\"%s\"", node->value.string);
            break;
        }

        case IDENTIFIER:
        {
            _PUT("%s", _ID(node).name);
            break;
        }

        case OPERATOR:
        {
            VERIFY((*_OP(node).src_func)(ctx, node),
                   return LANG_OP_SOURCE_CODE_ERROR);
            break;
        }

        default:
        {
            fprintf(stderr, "Invalid type\n");
            return LANG_SOURCE_CODE_ERROR;
        }
    }

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_binary_op(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left ||
           !cur_node->right,
           return LANG_SRC_BINARY_OP_ERROR);

    bool need_brackets =  (_NODE_IS_OPERATOR(cur_node, ADD) ||
                           _NODE_IS_OPERATOR(cur_node, SUB) ||
                           _NODE_IS_OPERATOR(cur_node, MUL) ||
                           _NODE_IS_OPERATOR(cur_node, DIV));

    if (need_brackets) {_PUT(" (");}

    _SRC(cur_node->left);
    _PUT(" %s ", _OP(cur_node).name);
    _SRC(cur_node->right);

    if (need_brackets) {_PUT(") ");}

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_unary_op(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left,
           return LANG_SRC_UNARY_OP_ERROR);

    _PUT("%s(", _OP(cur_node).name);
    _SRC(cur_node->left);
    _PUT(")");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_new_func(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left       ||
           !cur_node->left->right,
           fprintf(stderr, "type = %d \n cure_node.name = %s \nbad function: %s\n", cur_node->value_type, _OP(cur_node).name, _ID(cur_node->left).name);
           return LANG_SRC_NEW_FUNC_ERROR);

    _PUT("%s %s(", _OP(cur_node).name, _ID(cur_node->left).name);

    if (cur_node->left->left)
    {
        _SRC(cur_node->left->left);
    }

    _PUT(")\n");
    _PUT("{\n");
    _SRC(cur_node->left->right);
    _PUT("}\n");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_statement(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    while(cur_node)
    {
        _PUT("%s ", _OP(cur_node).name)

        VERIFY(!cur_node->left,
               return LANG_SRC_STATEMENT_ERROR);

        _SRC(cur_node->left)
        _PUT("\n");
        cur_node = cur_node->right;
    }

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_cond(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left ||
           !cur_node->right,
           return LANG_SRC_COND_ERROR);

    _PUT("%s (", _OP(cur_node).name);
    _SRC(cur_node->left);
    _PUT(")\n{\n");
    _SRC(cur_node->right);
    _PUT("}\n");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_ret(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left,
          return LANG_SRC_RET_ERROR);

    _PUT("%s ", _OP(cur_node).name);
    _SRC(cur_node->left);

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_param_linker(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    while (cur_node)
    {
        _PUT("%s ", _OP(cur_node).name);

        VERIFY(!cur_node->left,
               return LANG_SRC_PARAM_LINKER_ERROR);

        _SRC(cur_node->left);
        _PUT(" ");
        cur_node = cur_node->right;
    }

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_new_var(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left,
           return LANG_SRC_NEW_VAR_ERROR);

    _PUT("%s ", _OP(cur_node).name);
    _SRC(cur_node->left);

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_in(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left,
           return LANG_SRC_IN_ERROR);
    _PUT("%s(", _OP(cur_node).name);
    _SRC(cur_node->left);
    _PUT(")");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_out(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left,
           return LANG_SRC_OUT_ERROR);

    _PUT("%s(", _OP(cur_node).name);
    _SRC(cur_node->left);
    _PUT(")");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_call(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    VERIFY(!cur_node->left    ||
           !cur_node->left->left,
           return LANG_SRC_CALL_ERROR);

    _PUT("%s %s(", _OP(cur_node).name, _ID(cur_node->left).name);
    _SRC(cur_node->left->left);
    _PUT(") ");

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t src_hlt(lang_ctx_t* ctx, node_t* cur_node)
{
    ASSERT(ctx)
    ASSERT(cur_node)

    //-------------------------------------------------------------------//

    _PUT("%s", _OP(cur_node).name);

    //-------------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

#undef _PUT
#undef _SRC

//===================================================================//
