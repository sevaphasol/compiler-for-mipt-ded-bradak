#include <string.h>
#include "node_allocator.h"
#include "custom_assert.h"

//———————————————————————————————————————————————————————————————————//

#define _DSL_DEFINE_
#include "dsl.h"
#include "color_print.h"

// #define _STACK_DUMP

//———————————————————————————————————————————————————————————————————//

lang_status_t syntax_analysis (lang_ctx_t* ctx);

//-------------------------------------------------------------------//

static lang_status_t get_global_statement     (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_declaration          (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_func_declaration     (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_body                 (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_var_declaration      (lang_ctx_t* ctx, node_t** ret_node,
                                               bool        is_global);
static lang_status_t get_statement            (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_standart_operator    (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_if                   (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_return               (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_hlt                  (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_func                 (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_call                 (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_scan                 (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_assignment           (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_expression           (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_mul_div_expression   (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_in_parent_expression (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_single_expression    (lang_ctx_t* ctx, node_t** ret_node);
static lang_status_t get_func_params          (lang_ctx_t* ctx, node_t** ret_node,
                                               int*        n_params);
static lang_status_t get_func_use_params      (lang_ctx_t* ctx, node_t** ret_node,
                                               int*        n_params);

//-------------------------------------------------------------------//

static lang_status_t add_new_id          (lang_ctx_t*       ctx,
                                          identifier_type_t type,
                                          node_t*           node,
                                          bool              is_global);

static const char*   type_name           (value_type_t type);
static lang_status_t stack_push          (lang_ctx_t* ctx,
                                          size_t      val);
static lang_status_t push_new_id_counter (lang_ctx_t* ctx);
static lang_status_t check_var           (lang_ctx_t* ctx,
                                          size_t*     ind,
                                          int         mode);
static lang_status_t pop_locales         (lang_ctx_t* ctx);
static lang_status_t stack_dump          (lang_ctx_t* ctx);

//———————————————————————————————————————————————————————————————————//

lang_status_t check_var(lang_ctx_t* ctx, size_t* ind, int mode)
{
    ASSERT(ctx);
    ASSERT(ind);

    //——————————————————————————————————————————————————————————————————————//
    // (mode == 0) ≡ (checking for redeclaration  - macro _ON_REDECLARAION) //
    // (mode == 1) ≡ (checking for initialization - macro _ON_INIT)         //
    //——————————————————————————————————————————————————————————————————————//

    //---------------------------------------------------------------//

    stack_t        id_stack = ctx->id_stack;
    name_t*        names    = ctx->name_table.names;
    identifier_t*  ids      = ctx->name_table.ids;

    //---------------------------------------------------------------//

    for (int id_ind = 0; id_ind < id_stack.top; id_ind++)
    {
        if (strcmp(names[*ind].name, ids[id_stack.data[id_ind]].name) == 0)
        {
            if (mode == 0) return LANG_REDECLARATION_ERROR;

    //————————————————————————————————————————————————————————————————————//
    // If there are two variables with same names in differenet scopes,   //
    // ind != id_stack.data[id_ind] because id_ind is an index in array   //
    // names. But we need to know index in array ids for further          //
    // compiling in backend, because memory addres depends on that index. //
    //————————————————————————————————————————————————————————————————————//

            *ind = id_stack.data[id_ind];

            return LANG_SUCCESS;
        }
    }

    //---------------------------------------------------------------//

    if (mode == 0) return LANG_SUCCESS;

    return LANG_NOT_INIT_ERROR;
}

//===================================================================//

lang_status_t syntax_analysis(lang_ctx_t* ctx)
{
    ASSERT(ctx)
    ASSERT(ctx->nodes)

    //---------------------------------------------------------------//

    VERIFY(push_new_id_counter(ctx),
           return LANG_PUSH_NEW_ID_COUNTER_ERROR);

    //---------------------------------------------------------------//

    VERIFY(get_global_statement(ctx, &ctx->nodes[0]),
           return LANG_GET_GLOBAL_STATEMENT_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_global_statement(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _CHECK_OPERATOR(STATEMENT);
    _NEXT_POS;

    //---------------------------------------------------------------//

    VERIFY(get_declaration(ctx, &(*ret_node)->left),
           return LANG_GET_DECLARATION_ERROR);

    //---------------------------------------------------------------//

    if (_IS_OPERATOR(STATEMENT))
    {
        VERIFY(get_global_statement(ctx, &(*ret_node)->right),
               return LANG_GET_GLOBAL_STATEMENT_ERROR);
    }

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_declaration(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    if (_IS_OPERATOR(NEW_FUNC))
    {
        VERIFY(get_func_declaration(ctx, ret_node),
               return LANG_GET_FUNC_DECLARATION_ERROR);

        #ifdef _STACK_DUMP
            printf(_TURQUOISE("end of func declaration\n"));
            stack_dump(ctx);
        #endif

        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(NEW_VAR))
    {
        VERIFY(get_var_declaration(ctx, ret_node, true),
               return LANG_GET_VAR_DECLARATION);



        _ID((*ret_node)->left->left).is_global = 1;
        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    _EXPECTED("declaration");
}

//===================================================================//

lang_status_t get_func_declaration(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _CHECK_OPERATOR(NEW_FUNC);
    _NEXT_POS

    //---------------------------------------------------------------//

    (*ret_node)->left = _CURRENT_NODE;
    _CHECK_TYPE(IDENTIFIER);

    VERIFY(check_var(ctx, &_ID_IND((*ret_node)->left), _ON_REDECLARATION),
           _REDECLARATION_MESSAGE((*ret_node)->left));

    VERIFY(add_new_id(ctx, FUNC, (*ret_node)->left, true),
           return LANG_ADD_NEW_ID_ERROR);
    _NEXT_POS

    //---------------------------------------------------------------//

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("pushing new func\n"));
        stack_dump(ctx);
    #endif

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(OPEN_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(push_new_id_counter(ctx),
           return LANG_PUSH_NEW_ID_COUNTER_ERROR);

    //---------------------------------------------------------------//

    int n_params = 0;

    VERIFY(get_func_params(ctx, &(*ret_node)->left->left, &n_params),
           return LANG_GET_FUNC_PARAMS_ERROR);

    _ID((*ret_node)->left).n_params = n_params;

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(CLOSE_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//
    
    VERIFY(get_body(ctx, &(*ret_node)->left->right),
           return LANG_GET_BODY_ERROR);

    // node_t* body = (*ret_node)->left->right;
    // if (body->left == NULL && body->right == NULL) {
    //     // body is empty: { }
    //     _ID((*ret_node)->left).is_stdlib = true;
    // }

    //---------------------------------------------------------------//

    VERIFY(pop_locales(ctx),
           return LANG_POP_LOCALES_ERROR);

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("end of function, poping locales(func params)\n"));
        stack_dump(ctx);
    #endif

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_func_params(lang_ctx_t* ctx, node_t** ret_node,
                              int* n_params)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //--------------------------------------------------------------//

    if (!(_IS_OPERATOR(PARAM_LINKER))) { return LANG_SUCCESS; }

    (*n_params)++;

    //--------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _NEXT_POS

    //--------------------------------------------------------------//

    _CHECK_TYPE(OPERATOR);
    _CHECK_OPERATOR(NEW_VAR);

    (*ret_node)->left = _CURRENT_NODE;
    _NEXT_POS

    //--------------------------------------------------------------//

    (*ret_node)->left->left = _CURRENT_NODE;

    _CHECK_TYPE(IDENTIFIER);

    VERIFY(check_var(ctx, &_ID_IND(_CURRENT_NODE), _ON_REDECLARATION),
           _REDECLARATION_MESSAGE(_CURRENT_NODE));

    VERIFY(add_new_id(ctx, VAR, (*ret_node)->left->left, false),
           return LANG_ADD_NEW_ID_ERROR);

    //--------------------------------------------------------------//

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("pushing func param\n"));
        stack_dump(ctx);
    #endif

    //--------------------------------------------------------------//

    _NEXT_POS
    VERIFY(get_func_params(ctx, &(*ret_node)->right, n_params),
           return LANG_GET_FUNC_PARAMS_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_var_declaration(lang_ctx_t* ctx, node_t** ret_node, bool is_global)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    node_t* var_declaration = _CURRENT_NODE;
    _CHECK_OPERATOR(NEW_VAR);
    _NEXT_POS

    //---------------------------------------------------------------//

    node_t* var_name = _CURRENT_NODE;
    _CHECK_TYPE(IDENTIFIER);
    VERIFY(check_var(ctx, &_ID_IND(_CURRENT_NODE), _ON_REDECLARATION),
           _REDECLARATION_MESSAGE(_CURRENT_NODE));
    _NEXT_POS

    //---------------------------------------------------------------//

    node_t* assignment = _CURRENT_NODE;
    _CHECK_OPERATOR(ASSIGNMENT);
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(get_expression(ctx, &assignment->right),
           return LANG_GET_EXPRESSION_ERROR);

    //---------------------------------------------------------------//

    var_declaration->left = assignment;
    assignment->left      = var_name;
    *ret_node = var_declaration;

    //---------------------------------------------------------------//

    VERIFY(add_new_id(ctx, VAR, (*ret_node)->left->left, is_global),
           return LANG_ADD_NEW_ID_ERROR);

    //---------------------------------------------------------------//

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("pushing new var\n"));
        stack_dump(ctx);
    #endif

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_body(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    VERIFY(push_new_id_counter(ctx),
           return LANG_PUSH_NEW_ID_COUNTER_ERROR);

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("body start (pushing new id counter)\n"));
        stack_dump(ctx);
    #endif

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(BODY_START);
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(STATEMENT);

    *ret_node              = _CURRENT_NODE;
    node_t* cur_statement  = _CURRENT_NODE;

    _NEXT_POS
    VERIFY(get_statement(ctx, &cur_statement->left),
           return LANG_GET_STATEMENT_ERROR);

    //---------------------------------------------------------------//

    while (_IS_OPERATOR(STATEMENT))
    {
        cur_statement->right = _CURRENT_NODE;
        cur_statement        = _CURRENT_NODE;

        _NEXT_POS
        VERIFY(get_statement(ctx, &cur_statement->left),
               return LANG_GET_STATEMENT_ERROR);
    }

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(BODY_END);
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(pop_locales(ctx),
           return LANG_POP_LOCALES_ERROR);

    #ifdef _STACK_DUMP
        printf(_TURQUOISE("body end, poping locales\n"));
        stack_dump(ctx);
    #endif

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_statement(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    if (_IS_TYPE(OPERATOR))
    {
        VERIFY(get_standart_operator(ctx, ret_node),
               return LANG_GET_STANDART_FUNC_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_TYPE(IDENTIFIER))
    {
        if(!(_CURRENT_ID.type == VAR)) { _NOT_INIT_ERROR; } //???

        VERIFY(get_assignment(ctx, ret_node),
               return LANG_GET_ASSIGNMENT_ERROR);
        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    _EXPECTED("OPERATOR or IDENTIFIER");
}

//===================================================================//

lang_status_t get_standart_operator(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    _CHECK_TYPE(OPERATOR);

    if (_IS_OPERATOR(IF) ||
        _IS_OPERATOR(WHILE))
    {
        VERIFY(get_if(ctx, ret_node),
               return LANG_GET_IF_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(RET))
    {
        VERIFY(get_return(ctx, ret_node),
               return LANG_GET_RETURN_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(COS) ||
        _IS_OPERATOR(SIN) ||
        _IS_OPERATOR(OUT) ||
        _IS_OPERATOR(SQRT))
    {
        VERIFY(get_func(ctx, ret_node),
               return LANG_GET_FUNC_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(IN))
    {
        VERIFY(get_scan(ctx, ret_node),
               return LANG_GET_SCAN_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(NEW_VAR))
    {
        VERIFY(get_var_declaration(ctx, ret_node, false),
               return LANG_GET_VAR_DECLARATION_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(CALL))
    {
        VERIFY(get_call(ctx, ret_node),
               return LANG_GET_CALL_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(HLT))
    {
        VERIFY(get_hlt(ctx, ret_node),
               return LANG_GET_HLT_ERROR);
        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    _SYNTAX_ERROR("Expected standart func. Got "
                  _PURPLE("%s") " instead.",
                  _CURRENT_OP.name);
}

//===================================================================//

lang_status_t get_if(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;

    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(OPEN_BRACKET);
    _NEXT_POS

    VERIFY(get_expression(ctx, &(*ret_node)->left),
           return LANG_GET_EXPRESSION_ERROR);

    _CHECK_OPERATOR(CLOSE_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(get_body(ctx, &(*ret_node)->right),
           return LANG_GET_BODY_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_return(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(RET);
    *ret_node = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    if (!(_IS_OPERATOR(BODY_END)))
    {
        VERIFY(get_expression(ctx, &(*ret_node)->left),
               return LANG_GET_EXPRESSION_ERROR);
    }

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_hlt(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(HLT);
    *ret_node = _CURRENT_NODE;
    _NEXT_POS;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_func(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;

    _NEXT_POS

    _CHECK_OPERATOR(OPEN_BRACKET);
    _NEXT_POS

    _CHECK_OPERATOR(PARAM_LINKER);
    (*ret_node)->left = _CURRENT_NODE;
    _NEXT_POS

    VERIFY(get_expression(ctx, &(*ret_node)->left->left),
           return LANG_GET_EXPRESSION_ERROR);

    _CHECK_OPERATOR(CLOSE_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

// ===================================================================//

lang_status_t get_func_use_params(lang_ctx_t* ctx, node_t** ret_node,
                                  int* n_params)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    if (!(_IS_OPERATOR(PARAM_LINKER))) return LANG_SUCCESS;

    (*n_params)++;

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(get_expression(ctx, &(*ret_node)->left),
           return LANG_GET_EXPRESSION_ERROR);

    VERIFY(get_func_use_params(ctx, &(*ret_node)->right, n_params),
           return LANG_GET_FUNC_USE_PARAMS_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_scan(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(OPEN_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(PARAM_LINKER);
    (*ret_node)->left = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_TYPE(IDENTIFIER);

    if (_ID(_CURRENT_NODE).type != VAR) { _EXPECTED("var"); }

    VERIFY(check_var(ctx, &_ID_IND(_CURRENT_NODE), _ON_INITED),
           _NOT_INIT_ERROR);

    (*ret_node)->left->left = _CURRENT_NODE;

    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(CLOSE_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_assignment(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    _CHECK_TYPE(IDENTIFIER);

    if (_ID(_CURRENT_NODE).type != VAR) { _EXPECTED("var"); }

    node_t* var = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(ASSIGNMENT);
    node_t* assignment = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    VERIFY(get_expression(ctx, &assignment->right),
           return LANG_GET_EXPRESSION_ERROR);

    //---------------------------------------------------------------//

    assignment->left = var;
    (*ret_node) = assignment;

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_call(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    *ret_node = _CURRENT_NODE;
    _NEXT_POS

    _CHECK_TYPE(IDENTIFIER);

    // if (_CURRENT_ID.type != FUNC) { _EXPECTED("func"); }

    VERIFY(check_var(ctx, &_ID_IND(_CURRENT_NODE), _ON_INITED),
           _NOT_INIT_ERROR);

    (*ret_node)->left = _CURRENT_NODE;
    _NEXT_POS

    //---------------------------------------------------------------//

    _CHECK_OPERATOR(OPEN_BRACKET);
    _NEXT_POS

    int n_params = 0;

    VERIFY(get_func_use_params(ctx, &(*ret_node)->left->left, &n_params),
           return LANG_GET_FUNC_PARAMS_ERROR);

    _CHECK_OPERATOR(CLOSE_BRACKET);
    _NEXT_POS

    //---------------------------------------------------------------//

    if (n_params != _ID((*ret_node)->left).n_params)
    {
        _INVALID_PARAMS_MESSAGE((*ret_node));
    }

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_expression(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    node_t* cur_expression = nullptr;

    VERIFY(get_mul_div_expression(ctx, &cur_expression),
           return LANG_GET_MUL_DIV_EXPRESSION_ERROR);

    while (_IS_OPERATOR(ADD) ||
           _IS_OPERATOR(SUB))
    {
        node_t* operation = _CURRENT_NODE;
        operation->left   = cur_expression;
        _NEXT_POS

        VERIFY(get_mul_div_expression(ctx, &operation->right),
               return LANG_GET_MUL_DIV_EXPRESSION);

        cur_expression = operation;
    }

    *ret_node = cur_expression;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_mul_div_expression(lang_ctx_t* ctx,
                                     node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    node_t* cur_mul_div_expression = nullptr;

    VERIFY(get_in_parent_expression(ctx, &cur_mul_div_expression),
           return LANG_GET_IN_PARENT_EXPRESSION_ERROR);

    while (_IS_OPERATOR(MUL) ||
           _IS_OPERATOR(DIV))
    {
        node_t* operation = _CURRENT_NODE;
        operation->left   = cur_mul_div_expression;
        _NEXT_POS

        VERIFY(get_in_parent_expression(ctx, &operation->right),
               return LANG_GET_IN_PARENT_EXPRESSION_ERROR);

        cur_mul_div_expression = operation;
    }

    *ret_node = cur_mul_div_expression;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_in_parent_expression(lang_ctx_t* ctx,
                                       node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    if (_IS_OPERATOR(OPEN_BRACKET))
    {
        _NEXT_POS

        VERIFY(get_expression(ctx, ret_node),
               return LANG_GET_EXPRESSION_ERROR);

        _CHECK_OPERATOR(CLOSE_BRACKET);
        _NEXT_POS;

        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    VERIFY(get_single_expression(ctx, ret_node),
           return LANG_GET_SINGLE_EXPRESSION_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t get_single_expression(lang_ctx_t* ctx, node_t** ret_node)
{
    ASSERT(ctx);
    ASSERT(ret_node);

    //---------------------------------------------------------------//

    if (_IS_TYPE(NUMBER))
    {
        *ret_node = _CURRENT_NODE;
        _NEXT_POS

        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    if(_IS_TYPE(IDENTIFIER))
    {
        VERIFY(check_var(ctx, &_ID_IND(_CURRENT_NODE), _ON_INITED),
               _NOT_INIT_ERROR);

        *ret_node = _CURRENT_NODE;
        _NEXT_POS

        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    if (_IS_OPERATOR(SIN) ||
        _IS_OPERATOR(COS) ||
        _IS_OPERATOR(SQRT))
    {
        VERIFY(get_func(ctx, ret_node),
               return LANG_GET_FUNC_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(CALL))
    {
        VERIFY(get_call(ctx, ret_node),
               return LANG_GET_CALL_ERROR);
        return LANG_SUCCESS;
    }

    if (_IS_OPERATOR(RET))
    {
        VERIFY(get_return(ctx, ret_node),
               return LANG_GET_RETURN_ERROR);
        return LANG_SUCCESS;
    }

    //---------------------------------------------------------------//

    _SYNTAX_ERROR("invalid operation");
}

//===================================================================//

lang_status_t add_new_id(lang_ctx_t*       ctx,
                         identifier_type_t type,
                         node_t*           node,
                         bool              is_global)
{
    ASSERT(ctx);

    //---------------------------------------------------------------//

    ctx->name_table.ids[ctx->name_table.n_ids]
            = {.type      = type,
               .name      = ctx->name_table.names[_ID_IND(node)].name,
               .len       = ctx->name_table.names[_ID_IND(node)].len,
               .n_params  = 0,
               .is_inited = true,
               .is_global = is_global};

    node->value.id_index = ctx->name_table.n_ids;

    stack_push(ctx, ctx->name_table.n_ids);
    ctx->name_table.n_ids++;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

const char* type_name(value_type_t type)
{
    switch (type)
    {
        case OPERATOR:   return "OPERATOR";
        case NUMBER:     return "NUMBER";
        case IDENTIFIER: return "IDENTIFIER";
        default:         return "GOUDA";
    }
}

//===================================================================//

lang_status_t stack_push(lang_ctx_t* ctx, size_t val)
{
    ASSERT(ctx);

    //---------------------------------------------------------------//

    stack_t* id_stack         = &ctx->id_stack;
    stack_t* id_counter_stack = &ctx->id_counter_stack;

    //---------------------------------------------------------------//

    VERIFY(id_stack->top >= id_stack->size,
           return LANG_ID_STACK_OVERFLOW_ERROR);

    id_stack->data[id_stack->top++] = val;
    id_counter_stack->data[id_counter_stack->top - 1]++;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t push_new_id_counter(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    //---------------------------------------------------------------//

    stack_t* id_counter_stack = &ctx->id_counter_stack;

    VERIFY(id_counter_stack->top >= id_counter_stack->size,
           return LANG_ID_COUNTER_STACK_OVERFLOW_ERROR);

    id_counter_stack->data[id_counter_stack->top++] = 0;

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t pop_locales(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    //---------------------------------------------------------------//

    stack_t* id_stack         = &ctx->id_stack;
    stack_t* id_counter_stack = &ctx->id_counter_stack;

    //---------------------------------------------------------------//

    id_stack->top -= id_counter_stack->data[--id_counter_stack->top];

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//===================================================================//

lang_status_t stack_dump(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    //---------------------------------------------------------------//

    stack_t id_counter_stack = ctx->id_counter_stack;
    stack_t id_stack         = ctx->id_stack;

    printf(_GREEN("\nID_COUNTER_STACK: size = %ld"
                  " top = %ld\n\n"),
                  id_counter_stack.size,
                  id_counter_stack.top);

    for (size_t i = 0; i < id_counter_stack.top; i++)
    {
        printf(_GREEN("[%ld] = %ld\n"), i, id_counter_stack.data[i]);
    }

    //---------------------------------------------------------------//

    printf(_GREEN("\n\nID_STACK: size = %ld "
                  "top = %ld\n\n"),
                  id_stack.size,
                  id_stack.top);

    for (size_t i = 0; i < id_stack.top; i++)
    {
        printf(_GREEN("[%ld] = %ld\n"), i, id_stack.data[i]);
    }

    puts(_RED("\n//---------------------------------"
              "----------------------------------//\n"));

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//———————————————————————————————————————————————————————————————————//

#define _DSL_UNDEF_
#include "dsl.h"

//———————————————————————————————————————————————————————————————————//
