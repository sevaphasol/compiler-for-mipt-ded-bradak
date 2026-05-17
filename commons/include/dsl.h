#ifdef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————

//------------------------------------------------------------------------------

#define _ID(node)     ctx->name_table.ids[_ID_IND(node)]
#define _ID_IND(node) node->value.id_index
#define _CURRENT_NODE ctx->nodes[ctx->pos]
#define _CURRENT_ID   _ID(_CURRENT_NODE)
#define _CURRENT_OP   OperatorsTable[_CURRENT_NODE->value.operator_code]
#define _OP(_node_)   OperatorsTable[_node_->value.operator_code]

//------------------------------------------------------------------------------

#define VAR_SIZE 8

//------------------------------------------------------------------------------

#define _ON_REDECLARATION 0
#define _ON_INITED        1

//==============================================================================

#define _SYNTAX_ERROR(...)                                            \
                                                                      \
    fprintf(stderr, _RED("syntax error")                              \
                    " in line "                                       \
                    _YELLOW("%ld") ": ",                              \
                    _CURRENT_NODE->line_number);                      \
                                                                      \
    fprintf(stderr, ##__VA_ARGS__);                                   \
                                                                      \
    fputc('\n', stderr);                                              \
                                                                      \
    return LANG_SYNTAX_ERROR;                                         \

//==============================================================================

#define _NOT_INIT_ERROR                                               \
    _SYNTAX_ERROR(_TURQUOISE("%s")                                    \
                             " not initialized.",                     \
                             _CURRENT_ID.name);                       \

//==============================================================================

#define _EXPECTED(str)                                                \
    _SYNTAX_ERROR("Expected " _TURQUOISE("%s") ".", str);             \

//==============================================================================

#define _CHECK_TYPE(__type__)                                         \
                                                                      \
    if (_CURRENT_NODE->value_type != __type__)                        \
    {                                                                 \
        if (_CURRENT_NODE->value_type == IDENTIFIER)                  \
        {                                                             \
            _SYNTAX_ERROR(_RED("unexpected ")                         \
                          _PURPLE("%s") ".\n",                        \
                          _CURRENT_ID.name);                          \
        }                                                             \
        fprintf(stderr, "\n\n%d\n\n", __LINE__);                      \
        _SYNTAX_ERROR("Expected type " _PURPLE("%s") ". "             \
                      "Got type " _PURPLE("%s") " instead.",          \
                      type_name(__type__),                            \
                      type_name(_CURRENT_NODE->value_type));          \
    }                                                                 \

//==============================================================================

#define _INVALID_PARAMS_MESSAGE(__node_func__)                        \
                                                                      \
    _SYNTAX_ERROR("function "                                         \
                  _PURPLE("%s")                                       \
                  " gets "                                            \
                  _YELLOW("%d") " params "                            \
                  "but given %d",                                     \
                  _ID(__node_func__).name,                            \
                  _ID(__node_func__).n_params,                        \
                  n_params);                                          \

//==============================================================================

#define _CHECK_OPERATOR(__code__)                                     \
                                                                      \
    VERIFY(ctx->pos >= ctx->n_nodes,                                  \
           return LANG_NODES_OVERFLOW_ERROR;)                         \
                                                                      \
    _CHECK_TYPE(OPERATOR);                                            \
                                                                      \
    if (_CURRENT_NODE->value.operator_code != __code__)               \
    {                                                                 \
        _SYNTAX_ERROR("Expected " _PURPLE("\"%s\"") ". "              \
                      "Got " _PURPLE("\"%s\"") " instead.",           \
                      OperatorsTable[__code__].name,                  \
                      _CURRENT_OP.name);                              \
    }                                                                 \

//==============================================================================

#define _REDECLARATION_MESSAGE(__node__)                              \
                                                                      \
    _SYNTAX_ERROR("Redeclaration of "                                 \
                    _TURQUOISE("%s") ".",                             \
                    _ID(__node__).name)                               \

//==============================================================================

#define _CHECK_REDECLARATION(__node__)                                \
                                                                      \
    if (_ID(__node__).is_inited)                                      \
    {                                                                 \
        _SYNTAX_ERROR("Redeclaration of "                             \
                      _TURQUOISE("%s") ".",                           \
                      _ID(__node__).name)                             \
    }                                                                 \

//==============================================================================

#define _NODE_IS_OPERATOR(__node__, __operator_code__)                \
                                                                      \
    (__node__->value_type          == OPERATOR &&                     \
     __node__->value.operator_code == __operator_code__)              \

//==============================================================================

#define _IS_OPERATOR(__operator_code__)                               \
    _NODE_IS_OPERATOR(_CURRENT_NODE, __operator_code__)               \

//==============================================================================

#define _NODE_IS_TYPE(__node__, __type__)                             \
    (__node__->value_type == __type__)                                \

//==============================================================================

#define _IS_TYPE(__type__)                                            \
    _NODE_IS_TYPE(_CURRENT_NODE, __type__)                            \

//==============================================================================

#define _NODE_IS_ID_TYPE(__node__, __id_type__)                       \
    (_NODE_IS_TYPE(__node__, IDENTIFIER) &&                           \
     _ID(__node__).type == __id_type__)                               \

//==============================================================================

#define _IS_ID_TYPE(__id_type__)                                      \
    _NODE_IS_ID_TYPE(_CURRENT_NODE, __id_type__)                      \

//==============================================================================

#define _NEXT_POS                                                     \
                                                                      \
    ctx->pos++;                                                       \
    VERIFY(ctx->pos > ctx->n_nodes,                                   \
           return LANG_NODES_OVERFLOW_ERROR;)                         \

//==============================================================================

#define ASM(...)                                                      \
                                                                      \
    fprintf(ctx->output_file, "%*s", 8*ctx->level, "");               \
    fprintf(ctx->output_file, ##__VA_ARGS__);                         \

//==============================================================================

#define TAB_ASM(...)                                                  \
                                                                      \
    fprintf(ctx->output_file, "%*s", 8, "");                          \
    fprintf(ctx->output_file, ##__VA_ARGS__);                         \

//==============================================================================

#define RAW_ASM(...)                                                  \
                                                                      \
    fprintf(ctx->output_file, ##__VA_ARGS__);                         \

//==============================================================================

#define _POISON            node_ctor(ctx->node_allocator,             \
                                     POISON,                          \
                                     {},                              \
                                     -1,                              \
                                     nullptr, nullptr)                \

#define _OPERATOR(_code)   node_ctor(ctx->node_allocator,             \
                                     OPERATOR,                        \
                                     {.operator_code = (_code)},      \
                                     ctx->cur_line,                   \
                                     nullptr, nullptr)                \

#define _IDENTIFIER(_code) node_ctor(ctx->node_allocator,             \
                                     IDENTIFIER,                      \
                                     {.id_index = (_code)},           \
                                     ctx->cur_line,                   \
                                     nullptr, nullptr)                \

#define _NUMBER(_numb)     node_ctor(ctx->node_allocator,             \
                                     NUMBER,                          \
                                     {.number = (_numb)},             \
                                     ctx->cur_line,                   \
                                     nullptr, nullptr)                \

#define _STRING(_str)      node_ctor(ctx->node_allocator,             \
                                     STRING,                          \
                                     {.string = (_str)},              \
                                     ctx->cur_line,                   \
                                     nullptr, nullptr)                \

//——————————————————————————————————————————————————————————————————————————————

#endif
#undef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————

#ifdef _DSL_UNDEF_

//——————————————————————————————————————————————————————————————————————————————

#undef _RED
#undef _GREEN
#undef _YELLOW
#undef _BLUE
#undef _PURPLE
#undef _TURQUOISE
#undef _POISON
#undef _OPERATOR
#undef _IDENTIFIER
#undef _NUMBER
#undef _ID
#undef _CURRENT_NODE
#undef _CURRENT_ID
#undef _CURRENT_OP
#undef _PRINT
#undef _SYNTAX_ERROR
#undef _NOT_INIT_ERROR
#undef _EXPECTED
#undef _CHECK_TYPE
#undef _INVALID_PARAMS_MESSAGE
#undef _CHECK_OPERATOR
#undef _CHECK_REDECLARATION
#undef _REDECLARATION_MESSAGE
#undef _NODE_IS_OPERATOR
#undef _NODE_IS_TYPE
#undef _NODE_IS_ID_TYPE
#undef _IS_OPERATOR
#undef _IS_TYPE
#undef _IS_ID_TYPE
#undef _ON_REDECLARATION
#undef _ON_INITED

//——————————————————————————————————————————————————————————————————————————————

#endif
#undef _DSL_UNDEF_

//——————————————————————————————————————————————————————————————————————————————
