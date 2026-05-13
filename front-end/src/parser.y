%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Reuse DSL macros and context (same as your old parser) */
#define _DSL_DEFINE_
#include "dsl.h"
#undef _DSL_DEFINE_

#include "lang.h"
#include "node_allocator.h"
#include "custom_assert.h"

/* Global context – defined in main */
extern lang_ctx_t* ctx;

/* Semantic helpers (same as old syntax_analysis.c) */
static lang_status_t push_new_id_counter(lang_ctx_t* ctx);
static lang_status_t pop_locales(lang_ctx_t* ctx);
static lang_status_t stack_push(lang_ctx_t* ctx, size_t val);
static lang_status_t check_var(lang_ctx_t* ctx, size_t* ind, int mode);
static lang_status_t add_new_id(lang_ctx_t* ctx, identifier_type_t type, node_t* node, bool is_global);

/* AST helpers */
static size_t add_identifier(const char* name);
static size_t get_identifier_index(const char* name);
static node_t* make_binary_op(operator_code_t op, node_t* left, node_t* right);
static node_t* make_unary_op(operator_code_t op, node_t* operand);
static size_t count_nodes(node_t* node);
static node_t* reverse_statement_list(node_t* head);   // NEW

/* Modes for check_var */
#define ON_REDECLARATION 0
#define ON_INITED        1

/* Are we parsing a global declaration? */
static int is_global_context = 1;

/* Forward declarations (C++ compatible) */
int yylex(void);
void yyerror(const char* s);

%}

%locations
%define parse.error verbose

%union {
    int num;
    char* str;
    node_t* node;
}

/* Tokens */
%token TK_TILDE TK_COLON
%token TK_FUNC TK_VAR TK_IF TK_ELSE TK_WHILE TK_RETURN TK_PRINT TK_SCAN TK_CALL TK_SQRT
%token TK_ASSIGN TK_PLUS TK_MINUS TK_STAR TK_SLASH
%token TK_L_ROUND TK_R_ROUND TK_L_CURLY TK_R_CURLY
%token <num> TK_NUMBER
%token <str> TK_IDENTIFIER
%token TK_ERROR

/* Non‑terminal types */
%type <node> program top_level_list top_level_decl function_def var_decl
%type <node> param_list param param_rest
%type <node> body statement_list statement
%type <node> assignment if_stmt while_stmt return_stmt
%type <node> print_stmt scan_stmt call_stmt
%type <node> expression additive_expr multiplicative_expr unary_expr primary_expr
%type <node> argument_list

%%

program
    : /* Push global scope */
      { push_new_id_counter(ctx); is_global_context = 1; }
      top_level_list
      {
          node_t* reversed = reverse_statement_list($2);
          ctx->tree = reversed;
          ctx->nodes[0] = reversed;
          ctx->n_nodes = count_nodes(reversed);
          pop_locales(ctx);   /* pop global scope */
      }
    ;

top_level_list
    : /* empty */       { $$ = NULL; }
    | top_level_decl    { $$ = _OPERATOR(STATEMENT); $$->left = $1; $$->right = NULL; }
    | top_level_list top_level_decl
        {
            node_t* stmt = _OPERATOR(STATEMENT);
            stmt->left = $2;             /* new declaration on left */
            stmt->right = $1;            /* previous list on right */
            $$ = stmt;
        }
    ;

top_level_decl
    : TK_TILDE function_def   { $$ = $2; is_global_context = 1; }
    | TK_TILDE var_decl       { $$ = $2; is_global_context = 1; }
    ;

/* ------------------------------------------------------------------
   FUNCTION DEFINITIONS
   ------------------------------------------------------------------ */
function_def
    : TK_FUNC TK_IDENTIFIER
      {
          size_t idx = add_identifier($2);
          node_t* func_id = _IDENTIFIER(idx);
          if (check_var(ctx, &idx, ON_REDECLARATION) != LANG_SUCCESS) {
              fprintf(stderr, "Redeclaration of function '%s'\n", $2);
              YYERROR;
          }
          add_new_id(ctx, FUNC, func_id, true);
          push_new_id_counter(ctx);          /* scope for parameters */
          $<node>$ = func_id;                /* store for later use */
      }
      TK_L_ROUND param_list TK_R_ROUND
      {
          /* ---- Count parameters BEFORE parsing the body ---- */
          int n_params = 0;
          node_t* p = $5;                   /* param_list */
          while (p && p->value_type == OPERATOR && p->value.operator_code == PARAM_LINKER) {
              n_params++;
              p = p->right;
          }
          node_t* func_id = $<node>3;       /* retrieve stored func_id */
          size_t func_idx = func_id->value.id_index;
          ctx->name_table.ids[func_idx].n_params = n_params;
      }
      body
      {
          node_t* func_id   = $<node>3;
          node_t* func_node = _OPERATOR(NEW_FUNC);
          func_node->left        = func_id;
          func_node->left->left  = $5;      /* param_list */
          func_node->left->right = $8;      /* body (shifted by the two mid‑rule actions) */
          pop_locales(ctx);                 /* pop parameter scope */
          $$ = func_node;
      }
    ;

param_list
    : /* empty */                    { $$ = NULL; }
    | TK_COLON param param_rest
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $2;       /* the NEW_VAR node */
            linker->right = $3;      /* rest of the chain */
            $$ = linker;
        }
    ;

param_rest
    : /* empty */                    { $$ = NULL; }
    | TK_COLON param param_rest
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $2;
            linker->right = $3;
            $$ = linker;
        }
    ;

param
    : TK_VAR TK_IDENTIFIER
        {
            /* Parameter variable – no initialisation */
            node_t* var_decl = _OPERATOR(NEW_VAR);
            node_t* var_id   = _IDENTIFIER(add_identifier($2));
            size_t idx = var_id->value.id_index;
            if (check_var(ctx, &idx, ON_REDECLARATION) != LANG_SUCCESS) {
                fprintf(stderr, "Redeclaration of parameter '%s'\n", $2);
                YYERROR;
            }
            add_new_id(ctx, VAR, var_id, false);
            var_decl->left = var_id;   /* no assignment */
            $$ = var_decl;
        }
    ;

/* ------------------------------------------------------------------
   VARIABLE DECLARATIONS (global or local)
   ------------------------------------------------------------------ */
var_decl
    : TK_VAR TK_IDENTIFIER TK_ASSIGN expression
        {
            node_t* var_node = _OPERATOR(NEW_VAR);
            node_t* assign   = _OPERATOR(ASSIGNMENT);
            node_t* id_node  = _IDENTIFIER(add_identifier($2));
            size_t idx = id_node->value.id_index;
            if (check_var(ctx, &idx, ON_REDECLARATION) != LANG_SUCCESS) {
                fprintf(stderr, "Redeclaration of variable '%s'\n", $2);
                YYERROR;
            }
            add_new_id(ctx, VAR, id_node, is_global_context);
            assign->left = id_node;
            assign->right = $4;
            var_node->left = assign;
            $$ = var_node;
        }
    ;

/* ------------------------------------------------------------------
   BODY (scope for statements)
   ------------------------------------------------------------------ */
body
    : TK_L_CURLY
      { push_new_id_counter(ctx); }
      statement_list TK_R_CURLY
      {
          $$ = reverse_statement_list($3);   /* REVERSE to forward order */
          pop_locales(ctx);
      }
    ;

statement_list
    : /* empty */       { $$ = NULL; }
    | statement         { $$ = _OPERATOR(STATEMENT); $$->left = $1; $$->right = NULL; }
    | statement_list statement
        {
            node_t* stmt = _OPERATOR(STATEMENT);
            stmt->left = $2;          /* new statement on left */
            stmt->right = $1;         /* previous list on right */
            $$ = stmt;
        }
    ;

statement
    : TK_TILDE var_decl      { is_global_context = 0; $$ = $2; is_global_context = 1; }
    | TK_TILDE assignment    { $$ = $2; }
    | TK_TILDE if_stmt       { $$ = $2; }
    | TK_TILDE while_stmt    { $$ = $2; }
    | TK_TILDE return_stmt   { $$ = $2; }
    | TK_TILDE print_stmt    { $$ = $2; }
    | TK_TILDE scan_stmt     { $$ = $2; }
    | TK_TILDE call_stmt     { $$ = $2; }
    | TK_TILDE expression    { $$ = $2; }
    ;

/* ------------------------------------------------------------------
   ASSIGNMENT
   ------------------------------------------------------------------ */
assignment
    : TK_IDENTIFIER TK_ASSIGN expression
        {
            node_t* assign = _OPERATOR(ASSIGNMENT);
            node_t* id_node = _IDENTIFIER(get_identifier_index($1));
            size_t idx = id_node->value.id_index;
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Use of undeclared variable '%s'\n", $1);
                YYERROR;
            }
            assign->left = id_node;
            assign->right = $3;
            $$ = assign;
        }
    ;

if_stmt
    : TK_IF TK_L_ROUND expression TK_R_ROUND body
        {
            node_t* if_node = _OPERATOR(IF);
            if_node->left = $3;
            if_node->right = $5;
            $$ = if_node;
        }
    ;

while_stmt
    : TK_WHILE TK_L_ROUND expression TK_R_ROUND body
        {
            node_t* while_node = _OPERATOR(WHILE);
            while_node->left = $3;
            while_node->right = $5;
            $$ = while_node;
        }
    ;

return_stmt
    : TK_RETURN expression
        {
            node_t* ret = _OPERATOR(RET);
            ret->left = $2;
            $$ = ret;
        }
    | TK_RETURN
        {
            node_t* ret = _OPERATOR(RET);
            ret->left = NULL;
            $$ = ret;
        }
    ;

print_stmt
    : TK_PRINT TK_L_ROUND TK_COLON expression TK_R_ROUND
        {
            node_t* print = _OPERATOR(OUT);
            print->left = _OPERATOR(PARAM_LINKER);
            print->left->left = $4;
            print->left->right = NULL;
            $$ = print;
        }
    ;

scan_stmt
    : TK_SCAN TK_L_ROUND TK_COLON TK_IDENTIFIER TK_R_ROUND
        {
            node_t* scan = _OPERATOR(IN);
            node_t* id = _IDENTIFIER(get_identifier_index($4));
            size_t idx = id->value.id_index;
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Variable '%s' not declared\n", $4);
                YYERROR;
            }
            scan->left = _OPERATOR(PARAM_LINKER);
            scan->left->left = id;
            scan->left->right = NULL;
            $$ = scan;
        }
    ;

call_stmt
    : TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            node_t* call = _OPERATOR(CALL);
            node_t* id = _IDENTIFIER(get_identifier_index($2));
            size_t idx = id->value.id_index;
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Function '%s' not declared\n", $2);
                YYERROR;
            }
            int expected = ctx->name_table.ids[idx].n_params;
            int got = 0;
            node_t* arg = $4;
            while (arg && arg->value_type == OPERATOR && arg->value.operator_code == PARAM_LINKER) {
                got++;
                arg = arg->right;
            }
            if (got != expected) {
                fprintf(stderr, "Function '%s' expects %d arguments, got %d\n",
                        ctx->name_table.ids[idx].name, expected, got);
                YYERROR;
            }
            call->left = id;
            call->left->left = $4;
            $$ = call;
        }
    ;

argument_list
    : /* empty */   { $$ = NULL; }
    | TK_COLON expression
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $2;
            linker->right = NULL;
            $$ = linker;
        }
    | argument_list TK_COLON expression
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $3;
            linker->right = $1;
            $$ = linker;
        }
    ;

/* ------------------------------------------------------------------
   EXPRESSIONS
   ------------------------------------------------------------------ */
expression
    : additive_expr { $$ = $1; }
    ;

additive_expr
    : multiplicative_expr                      { $$ = $1; }
    | additive_expr TK_PLUS multiplicative_expr   { $$ = make_binary_op(ADD, $1, $3); }
    | additive_expr TK_MINUS multiplicative_expr  { $$ = make_binary_op(SUB, $1, $3); }
    ;

multiplicative_expr
    : unary_expr                               { $$ = $1; }
    | multiplicative_expr TK_STAR unary_expr      { $$ = make_binary_op(MUL, $1, $3); }
    | multiplicative_expr TK_SLASH unary_expr     { $$ = make_binary_op(DIV, $1, $3); }
    ;

unary_expr
    : primary_expr                             { $$ = $1; }
    | TK_MINUS primary_expr                    { $$ = make_unary_op(SUB, $2); }
    | TK_SQRT TK_L_ROUND TK_COLON primary_expr TK_R_ROUND { $$ = make_unary_op(SQRT, $4); }
    ;

primary_expr
    : TK_NUMBER   { $$ = _NUMBER($1); }
    | TK_IDENTIFIER
        {
            size_t idx = get_identifier_index($1);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Use of undeclared variable '%s'\n", $1);
                YYERROR;
            }
            $$ = _IDENTIFIER(idx);
            free($1);
        }
    | TK_L_ROUND expression TK_R_ROUND { $$ = $2; }
    | TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            /* Inline call as expression – same logic as call_stmt */
            node_t* call = _OPERATOR(CALL);
            node_t* id = _IDENTIFIER(get_identifier_index($2));
            size_t idx = id->value.id_index;
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Function '%s' not declared\n", $2);
                YYERROR;
            }
            int expected = ctx->name_table.ids[idx].n_params;
            int got = 0;
            node_t* arg = $4;
            while (arg && arg->value_type == OPERATOR && arg->value.operator_code == PARAM_LINKER) {
                got++;
                arg = arg->right;
            }
            if (got != expected) {
                fprintf(stderr, "Function '%s' expects %d arguments, got %d\n",
                        ctx->name_table.ids[idx].name, expected, got);
                YYERROR;
            }
            call->left = id;
            call->left->left = $4;
            $$ = call;
        }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d, column %d: %s\n",
            yylloc.first_line, yylloc.first_column, s);
}

/* ---------- Semantic helper implementations ---------- */

static size_t add_identifier(const char* name) {
    for (size_t i = 0; i < ctx->name_table.n_names; i++) {
        if (strcmp(ctx->name_table.names[i].name, name) == 0)
            return i;
    }
    size_t idx = ctx->name_table.n_names;
    ctx->name_table.names[idx].name = strdup(name);
    ctx->name_table.names[idx].len = strlen(name);
    ctx->name_table.n_names++;
    return idx;
}

static size_t get_identifier_index(const char* name) {
    for (size_t i = 0; i < ctx->name_table.n_names; i++) {
        if (strcmp(ctx->name_table.names[i].name, name) == 0)
            return i;
    }
    return add_identifier(name);
}

static node_t* make_binary_op(operator_code_t op, node_t* left, node_t* right) {
    node_t* node = _OPERATOR(op);
    node->left = left;
    node->right = right;
    return node;
}

static node_t* make_unary_op(operator_code_t op, node_t* operand) {
    node_t* node = _OPERATOR(op);
    node->left = operand;
    return node;
}

static size_t count_nodes(node_t* node) {
    if (!node) return 0;
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}

/* ---- List reversal (new) ---- */
static node_t* reverse_statement_list(node_t* head) {
    node_t* prev = NULL;
    node_t* curr = head;
    while (curr) {
        node_t* next = curr->right;   /* save next */
        curr->right = prev;           /* reverse link */
        prev = curr;
        curr = next;
    }
    return prev;
}

/* ----- Stack and symbol‑table helpers (exact copies from your old syntax_analysis.c) ----- */

static lang_status_t stack_push(lang_ctx_t* ctx, size_t val) {
    if (ctx->id_stack.top >= ctx->id_stack.size) return LANG_ID_STACK_OVERFLOW_ERROR;
    ctx->id_stack.data[ctx->id_stack.top++] = val;
    ctx->id_counter_stack.data[ctx->id_counter_stack.top - 1]++;
    return LANG_SUCCESS;
}

static lang_status_t push_new_id_counter(lang_ctx_t* ctx) {
    if (ctx->id_counter_stack.top >= ctx->id_counter_stack.size)
        return LANG_ID_COUNTER_STACK_OVERFLOW_ERROR;
    ctx->id_counter_stack.data[ctx->id_counter_stack.top++] = 0;
    return LANG_SUCCESS;
}

static lang_status_t pop_locales(lang_ctx_t* ctx) {
    if (ctx->id_counter_stack.top == 0) return LANG_POP_LOCALES_ERROR;
    ctx->id_stack.top -= ctx->id_counter_stack.data[--ctx->id_counter_stack.top];
    return LANG_SUCCESS;
}

static lang_status_t check_var(lang_ctx_t* ctx, size_t* ind, int mode) {
    stack_t* id_stack = &ctx->id_stack;
    name_t*  names    = ctx->name_table.names;
    identifier_t* ids = ctx->name_table.ids;

    for (int i = (int)id_stack->top - 1; i >= 0; i--) {
        if (strcmp(names[*ind].name, ids[id_stack->data[i]].name) == 0) {
            if (mode == ON_REDECLARATION) return LANG_REDECLARATION_ERROR;
            /* mode ON_INITED: found, update index to the actual id */
            *ind = id_stack->data[i];
            return LANG_SUCCESS;
        }
    }
    if (mode == ON_INITED) return LANG_NOT_INIT_ERROR;
    return LANG_SUCCESS;  /* not found is fine for redeclaration check */
}

static lang_status_t add_new_id(lang_ctx_t* ctx, identifier_type_t type, node_t* node, bool is_global) {
    size_t name_index = node->value.id_index;
    const char* name = ctx->name_table.names[name_index].name;
    size_t len = ctx->name_table.names[name_index].len;

    size_t new_id = ctx->name_table.n_ids;
    ctx->name_table.ids[new_id].type      = type;
    ctx->name_table.ids[new_id].name      = (char*)name;   /* cast to match struct field */
    ctx->name_table.ids[new_id].len       = len;
    ctx->name_table.ids[new_id].n_params  = 0;
    ctx->name_table.ids[new_id].is_inited = true;
    ctx->name_table.ids[new_id].addr      = 0;
    ctx->name_table.ids[new_id].is_global = is_global;
    ctx->name_table.ids[new_id].is_stdlib = 0;
    node->value.id_index = new_id;
    ctx->name_table.n_ids++;
    return stack_push(ctx, new_id);
}