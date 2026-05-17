#ifndef LANG_H__
#define LANG_H__

//———————————————————————————————————————————————————————————————————//

#include <stdio.h>
#include <inttypes.h>

#include "argv_parser.h"
#include "buffer.h"
#include "lang_status.h"
#include "fixup_table.h"
#include "lib_calls_table.h"
#include "ir_list.h"

//———————————————————————————————————————————————————————————————————//

const int MaxStrLength = 100;

const char* const FrontendDefaultInput  = "source.txt";
const char* const FrontendDefaultOutput = "frontend.txt";

const char* const BackendDefaultInput   = FrontendDefaultOutput;
const char* const BackendDefaultOutput  = "backend.s";

//———————————————————————————————————————————————————————————————————//

enum value_type_t
{
    POISON     = 0,
    OPERATOR   = 1,
    NUMBER     = 2,
    IDENTIFIER = 3,
    STRING     = 4,
};

//———————————————————————————————————————————————————————————————————//

enum operator_code_t
{
    UNDEFINED     = 0,
    ADD           = 1,
    SUB           = 2,
    MUL           = 3,
    DIV           = 4,
    COS           = 5,
    SIN           = 6,
    SQRT          = 7,
    POW           = 8,
    BIGGER        = 9,
    SMALLER       = 10,
    ASSIGNMENT    = 11,
    OPEN_BRACKET  = 12,
    CLOSE_BRACKET = 13,
    BODY_START    = 14,
    BODY_END      = 15,
    STATEMENT     = 16,
    IF            = 17,
    WHILE         = 18,
    RET           = 19,
    PARAM_LINKER  = 20,
    NEW_VAR       = 21,
    NEW_FUNC_DEF  = 22,
    NEW_FUNC_DECL = 23,
    CALL          = 24,
    HLT           = 25,
};

//———————————————————————————————————————————————————————————————————//

struct lang_ctx_t;
struct node_t;

struct operator_t
{
    operator_code_t code;
    const char*     name;
    size_t          len;
    const char*     asm_name;
    int             n_children;
    lang_status_t   (*to_ir_func) (lang_ctx_t* ctx, node_t* cur_node);
    lang_status_t   (*src_func)   (lang_ctx_t* ctx, node_t* cur_node);
};

//———————————————————————————————————————————————————————————————————//

lang_status_t binary_operation_to_ir  (lang_ctx_t* ctx, node_t* node);
lang_status_t div_to_ir               (lang_ctx_t* ctx, node_t* node);
lang_status_t assignment_to_ir        (lang_ctx_t* ctx, node_t* node);
lang_status_t statement_to_ir         (lang_ctx_t* ctx, node_t* node);
lang_status_t if_to_ir                (lang_ctx_t* ctx, node_t* node);
lang_status_t while_to_ir             (lang_ctx_t* ctx, node_t* node);
lang_status_t new_var_to_ir           (lang_ctx_t* ctx, node_t* node);
lang_status_t new_func_to_ir          (lang_ctx_t* ctx, node_t* node);
lang_status_t func_decl_to_ir         (lang_ctx_t* ctx, node_t* node);
lang_status_t return_to_ir            (lang_ctx_t* ctx, node_t* node);
lang_status_t call_to_ir              (lang_ctx_t* ctx, node_t* node);
lang_status_t exit_to_ir              (lang_ctx_t* ctx, node_t* node);
lang_status_t var_to_ir               (lang_ctx_t* ctx, node_t* node);
lang_status_t node_to_ir              (lang_ctx_t* ctx, node_t* node);
lang_status_t sqrt_to_ir              (lang_ctx_t* ctx, node_t* node);

//———————————————————————————————————————————————————————————————————//

#define STR_AND_LEN(str) str, sizeof(str)

const operator_t OperatorsTable[] =
{ // code           name and len              asm_name    n_children    to_ir_func                src_func
    {UNDEFINED,     nullptr, 0,               nullptr,       0,          nullptr                 , nullptr},
    {ADD,           STR_AND_LEN("+"),         "add",         2,          &binary_operation_to_ir , nullptr},
    {SUB,           STR_AND_LEN("-"),         "sub",         2,          &binary_operation_to_ir , nullptr},
    {MUL,           STR_AND_LEN("*"),         "mul",         2,          &binary_operation_to_ir , nullptr},
    {DIV,           STR_AND_LEN("/"),         "div",         2,          &div_to_ir              , nullptr},
    {COS,           STR_AND_LEN("cos"),       "cos",         1,          nullptr                 , nullptr},
    {SIN,           STR_AND_LEN("sin"),       "sin",         1,          nullptr                 , nullptr},
    {SQRT,          STR_AND_LEN("sqrt"),      "sqrt",        1,          &sqrt_to_ir             , nullptr},
    {POW,           STR_AND_LEN("^"),         "pow",         1,          nullptr                 , nullptr},
    {BIGGER,        STR_AND_LEN(">"),         nullptr,       2,          &assignment_to_ir       , nullptr},
    {SMALLER,       STR_AND_LEN("<"),         nullptr,       2,          &assignment_to_ir       , nullptr},
    {ASSIGNMENT,    STR_AND_LEN("="),         nullptr,       2,          &assignment_to_ir       , nullptr},
    {OPEN_BRACKET,  STR_AND_LEN("("),         nullptr,       1,          nullptr                 , nullptr},
    {CLOSE_BRACKET, STR_AND_LEN(")"),         nullptr,       1,          nullptr                 , nullptr},
    {BODY_START,    STR_AND_LEN("{"),         nullptr,       1,          nullptr                 , nullptr},
    {BODY_END,      STR_AND_LEN("}"),         nullptr,       1,          nullptr                 , nullptr},
    {STATEMENT,     STR_AND_LEN("~"),         nullptr,       2,          &statement_to_ir        , nullptr},
    {IF,            STR_AND_LEN("if"),        nullptr,       2,          &if_to_ir               , nullptr},
    {WHILE,         STR_AND_LEN("while"),     nullptr,       2,          &while_to_ir            , nullptr},
    {RET,           STR_AND_LEN("return"),    nullptr,       0,          &return_to_ir           , nullptr},
    {PARAM_LINKER,  STR_AND_LEN(":"),         nullptr,       2,          nullptr                 , nullptr},
    {NEW_VAR,       STR_AND_LEN("var"),       nullptr,       2,          &new_var_to_ir          , nullptr},
    {NEW_FUNC_DEF,  STR_AND_LEN("func"),      nullptr,       2,          &new_func_to_ir         , nullptr},
    {NEW_FUNC_DECL, STR_AND_LEN("fdecl"),     nullptr,       2,          &func_decl_to_ir        , nullptr},
    {CALL,          STR_AND_LEN("call"),      "call",        0,          &call_to_ir             , nullptr},
    {HLT,           STR_AND_LEN("exit"),      nullptr,       0,          &exit_to_ir             , nullptr}
};

#undef STR_AND_LEN

const int nOperators = sizeof(OperatorsTable) / sizeof(operator_t);

//———————————————————————————————————————————————————————————————————//

enum identifier_type_t
{
    UNKNOWN     = 0,
    VAR         = 1,
    FUNC_DEF    = 2,
    FUNC_DECL   = 3,
};

//———————————————————————————————————————————————————————————————————//

struct identifier_t
{
    identifier_type_t type;
    char*             name;
    size_t            len;
    int               n_params;
    bool              is_inited;
    int               addr;
    bool              is_global;
    bool              is_stdlib; 
};

//———————————————————————————————————————————————————————————————————//

struct name_t
{
    char* name;
    size_t len;
};

//———————————————————————————————————————————————————————————————————//

typedef int number_t;

//———————————————————————————————————————————————————————————————————//

union value_t
{
    operator_code_t   operator_code;
    size_t            id_index;
    number_t          number;
    char*             string;
};

//———————————————————————————————————————————————————————————————————//

struct node_t
{
    value_type_t value_type;
    value_t      value;
    size_t       line_number;
    node_t*      left;
    node_t*      right;
    node_t*      parent;
};

//———————————————————————————————————————————————————————————————————//

struct stack_t
{
    size_t  size;
    size_t  top;
    size_t* data;
};

//———————————————————————————————————————————————————————————————————//

struct name_table_t
{
    size_t        n_names;
    name_t*       names;
    size_t        n_ids;
    identifier_t* ids;
};

//———————————————————————————————————————————————————————————————————//

struct node_allocator_t;

//——————————————————————————————————————————————————————————————————————————————

struct lang_ctx_t
{
    FILE*             input_file;
    FILE*             output_file;
    size_t            input_size;

    node_allocator_t* node_allocator;
    name_table_t      name_table;
    node_t*           tree;
    node_t**          nodes;
    size_t            n_nodes;
    char*             code;

    // for frontend
    stack_t           id_stack;
    stack_t           id_counter_stack;
    size_t            cur_line;
    size_t            pos;

    // for backend
    ap_ctx_t          ap_ctx;

    int               level;
    size_t            n_labels;
    size_t            n_globals;
    size_t            n_locals;
    bool              emitting_global_init;

    ir_node_t*        ir_list;
    buffer_t          ir_buf;
    buffer_t          bin_buf;

    label_table_t     label_table;
    fixup_table_t     fixups;
    fixup_table_t     global_data_fixups;
    fixup_table_t     string_fixups;
    
    lib_calls_table_t lib_calls_table;
    stdlib_data_t     stdlib_data;
};

//———————————————————————————————————————————————————————————————————//

lang_status_t lang_ctx_ctor (lang_ctx_t* ctx,
                             int argc,
                             const char* argv[],
                             const char* default_input,
                             const char* default_output);

lang_status_t lang_ctx_dtor (lang_ctx_t* ctx);

//———————————————————————————————————————————————————————————————————//

#endif // LANG_H__
