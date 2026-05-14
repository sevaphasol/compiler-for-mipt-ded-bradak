%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _DSL_DEFINE_
#include "dsl.h"
#undef _DSL_DEFINE_

#include "lang.h"
#include "node_allocator.h"
#include "parser_utils.h"

extern lang_ctx_t* ctx;
extern int yylineno;

int yylex(void);
void yyerror(const char* s);

#define ON_REDECLARATION 0
#define ON_INITED        1

static int is_global_context = 1;

/* Semantic helpers */
static lang_status_t push_new_id_counter(lang_ctx_t* ctx);
static lang_status_t pop_locales(lang_ctx_t* ctx);
static lang_status_t check_var(lang_ctx_t* ctx, size_t* ind, int mode);
static lang_status_t add_new_id(lang_ctx_t* ctx, identifier_type_t type, node_t* node, bool is_global);
static int find_existing_id(lang_ctx_t* ctx, size_t name_idx);
%}

%locations
%define parse.error verbose

%union {
    int num;
    char* str;
    node_t* node;
}

%token TK_TILDE TK_COLON TK_FDECL
%token TK_FUNC TK_VAR TK_IF TK_ELSE TK_WHILE TK_RETURN TK_CALL TK_SQRT
%token TK_ASSIGN TK_PLUS TK_MINUS TK_STAR TK_SLASH
%token TK_L_ROUND TK_R_ROUND TK_L_CURLY TK_R_CURLY
%token <num> TK_NUMBER
%token <str> TK_IDENTIFIER
%token TK_ERROR

%type <node> program top_level_list top_level_decl function_def function_decl var_decl 
%type <node> param_list param param_rest
%type <node> body statement_list statement
%type <node> assignment if_stmt while_stmt return_stmt
%type <node> call_stmt
%type <node> expression additive_expr multiplicative_expr unary_expr primary_expr
%type <node> argument_list

%%

program
    : { push_new_id_counter(ctx); is_global_context = 1; }
      top_level_list
      {
          node_t* root = ($2) ? reverse_statement_list($2) : NULL;
          ctx->tree = root;
          ctx->nodes[0] = root;
          ctx->n_nodes = count_nodes(root);
          pop_locales(ctx);
      }
    ;

top_level_list
    : /* empty */       { $$ = NULL; }
    | top_level_list top_level_decl
        {
            $$ = make_statement($2, $1);
        }
    ;

top_level_decl
    : TK_TILDE { is_global_context = 1; } function_def   { $$ = $3; }
    | TK_TILDE { is_global_context = 1; } function_decl  { $$ = $3; }
    | TK_TILDE { is_global_context = 1; } var_decl       { $$ = $3; }
    ;

function_decl
    : TK_FDECL TK_IDENTIFIER
      {
          size_t name_idx = add_identifier($2);

          int existing = find_existing_id(ctx, name_idx);
          if (existing >= 0) {
              fprintf(stderr, "Error: '%s' already declared or defined\n", $2);
              YYERROR;
          }

          node_t* func_id = _IDENTIFIER(name_idx);
          add_new_id(ctx, FUNC_DECL, func_id, true);
          push_new_id_counter(ctx);
          $<node>$ = func_id;
          free($2);
      }
      TK_L_ROUND param_list TK_R_ROUND
      {
          int n_params = 0;
          node_t* p = $5;
          while (p && p->value_type == OPERATOR && p->value.operator_code == PARAM_LINKER) {
              n_params++;
              p = p->right;
          }

          node_t* func_id = $<node>3;
          ctx->name_table.ids[func_id->value.id_index].n_params = n_params;
          func_id->left = $5;           

          pop_locales(ctx);             // parameters go out of scope

          node_t* decl_node = _OPERATOR(NEW_FUNC_DECL);
          decl_node->left = func_id;
          decl_node->right = NULL;      // no body
          $$ = decl_node;
      }
    ;

function_def
    : TK_FUNC TK_IDENTIFIER
      {
          size_t name_idx = add_identifier($2);

          int existing = find_existing_id(ctx, name_idx);
          if (existing >= 0) {
              identifier_t* id = &ctx->name_table.ids[existing];
              if (id->type == FUNC_DEF) {
                  fprintf(stderr, "Error: Redefinition of function '%s'\n", $2);
              } else if (id->type == NEW_FUNC_DECL) {
                  fprintf(stderr, "Error: Function '%s' was already declared; definition not allowed\n", $2);
              } else {
                  fprintf(stderr, "Error: '%s' is already used as a variable\n", $2);
              }
              YYERROR;
          }

          node_t* func_id = _IDENTIFIER(name_idx);
          add_new_id(ctx, FUNC_DEF, func_id, true);
          push_new_id_counter(ctx);
          $<node>$ = func_id;
          free($2);
      }
      TK_L_ROUND param_list TK_R_ROUND
      {
          int n_params = 0;
          node_t* p = $5;
          while (p && p->value_type == OPERATOR && p->value.operator_code == PARAM_LINKER) {
              n_params++;
              p = p->right;
          }
          node_t* func_id = $<node>3;
          ctx->name_table.ids[func_id->value.id_index].n_params = n_params;
          func_id->left = $5;
      }
      body
      {
          node_t* func_id   = $<node>3;
          node_t* func_node = _OPERATOR(NEW_FUNC_DEF);
          func_node->left   = func_id;
          func_id->right    = $8;
          pop_locales(ctx);
          $$ = func_node;
      }
    ;

param_list
    : /* empty */ { $$ = NULL; }
    | TK_COLON param param_rest
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $2;
            linker->right = $3;
            $$ = linker;
        }
    ;

param_rest
    : /* empty */ { $$ = NULL; }
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
            size_t idx = add_identifier($2);
            if (check_var(ctx, &idx, ON_REDECLARATION) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Redeclaration of parameter '%s'\n", $2);
                YYERROR;
            }
            node_t* var_id = _IDENTIFIER(idx);
            add_new_id(ctx, VAR, var_id, false);
            node_t* var_decl = _OPERATOR(NEW_VAR);
            var_decl->left = var_id;
            $$ = var_decl;
            free($2);
        }
    ;

var_decl
    : TK_VAR TK_IDENTIFIER TK_ASSIGN expression
        {
            size_t idx = add_identifier($2);
            if (check_var(ctx, &idx, ON_REDECLARATION) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Redeclaration of variable '%s'\n", $2);
                YYERROR;
            }
            node_t* id_node = _IDENTIFIER(idx);
            add_new_id(ctx, VAR, id_node, is_global_context);
            
            node_t* assign = _OPERATOR(ASSIGNMENT);
            assign->left = id_node;
            assign->right = $4;
            
            node_t* var_node = _OPERATOR(NEW_VAR);
            var_node->left = assign;
            $$ = var_node;
            free($2);
        }
    ;

body
    : TK_L_CURLY { push_new_id_counter(ctx); }
      statement_list TK_R_CURLY
      {
          $$ = ($3) ? reverse_statement_list($3) : NULL;
          pop_locales(ctx);
      }
    ;

statement_list
    : /* empty */       { $$ = NULL; }
    | statement_list statement
        {
            $$ = make_statement($2, $1);
        }
    ;

statement
    : TK_TILDE { is_global_context = 0; } var_decl      { $$ = $3; }
    | TK_TILDE assignment                               { $$ = $2; }
    | TK_TILDE if_stmt                                  { $$ = $2; }
    | TK_TILDE while_stmt                               { $$ = $2; }
    | TK_TILDE return_stmt                              { $$ = $2; }
    | TK_TILDE call_stmt                                { $$ = $2; }
    | TK_TILDE expression                               { $$ = $2; }
    ;

assignment
    : TK_IDENTIFIER TK_ASSIGN expression
        {
            size_t idx = get_identifier_index($1);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Undeclared variable '%s'\n", $1);
                YYERROR;
            }
            node_t* id_node = _IDENTIFIER(idx); 
            node_t* assign = _OPERATOR(ASSIGNMENT);
            assign->left = id_node;
            assign->right = $3;
            $$ = assign;
            free($1);
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
    : TK_RETURN expression  { $$ = _OPERATOR(RET); $$->left = $2; }
    | TK_RETURN             { $$ = _OPERATOR(RET); $$->left = NULL; }
    ;

/* print_stmt
    : TK_PRINT TK_L_ROUND TK_COLON expression TK_R_ROUND
        {
            node_t* print = _OPERATOR(OUT);
            print->left = make_linker($4);
            $$ = print;
        }
    ;

scan_stmt
    : TK_SCAN TK_L_ROUND TK_COLON TK_IDENTIFIER TK_R_ROUND
        {
            size_t idx = get_identifier_index($4);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Variable '%s' not declared\n", $4);
                YYERROR;
            }
            node_t* scan = _OPERATOR(IN);
            scan->left = make_linker(_IDENTIFIER(idx));
            $$ = scan;
            free($4);
        }
    ; */

call_stmt
    : TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            size_t idx = get_identifier_index($2);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Function '%s' not declared\n", $2);
                YYERROR;
            }
            node_t* id = _IDENTIFIER(idx);
            // Symmetry: Arguments go into IDENTIFIER->left
            id->left = ($4) ? reverse_statement_list($4) : NULL;
            node_t* call = _OPERATOR(CALL);
            call->left = id;
            $$ = call;
            free($2);
        }
    ;

argument_list
    : /* empty */ { $$ = NULL; }
    | argument_list TK_COLON expression
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $3;
            linker->right = $1;
            $$ = linker;
        }
    | TK_COLON expression
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $2;
            linker->right = NULL;
            $$ = linker;
        }
    ;

expression
    : additive_expr { $$ = $1; }
    ;

additive_expr
    : multiplicative_expr                         { $$ = $1; }
    | additive_expr TK_PLUS multiplicative_expr   { $$ = make_binary_op(ADD, $1, $3); }
    | additive_expr TK_MINUS multiplicative_expr  { $$ = make_binary_op(SUB, $1, $3); }
    ;

multiplicative_expr
    : unary_expr                                 { $$ = $1; }
    | multiplicative_expr TK_STAR unary_expr      { $$ = make_binary_op(MUL, $1, $3); }
    | multiplicative_expr TK_SLASH unary_expr     { $$ = make_binary_op(DIV, $1, $3); }
    ;

unary_expr
    : primary_expr                                { $$ = $1; }
    | TK_MINUS primary_expr                       { $$ = make_unary_op(SUB, $2); }
    | TK_SQRT TK_L_ROUND TK_COLON expression TK_R_ROUND 
        { 
            node_t* sqrt_node = _OPERATOR(SQRT);
            sqrt_node->left = make_linker($4);
            $$ = sqrt_node;
        }

primary_expr
    : TK_NUMBER   { $$ = _NUMBER($1); }
    | TK_IDENTIFIER
        {
            size_t idx = get_identifier_index($1);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Undeclared variable '%s'\n", $1);
                YYERROR;
            }
            $$ = _IDENTIFIER(idx);
            free($1);
        }
    | TK_L_ROUND expression TK_R_ROUND { $$ = $2; }
    | TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            size_t idx = get_identifier_index($2);
            if (check_var(ctx, &idx, ON_INITED) != LANG_SUCCESS) {
                fprintf(stderr, "Error: Function '%s' not declared\n", $2);
                YYERROR;
            }
            node_t* id = _IDENTIFIER(idx);
            id->left = ($4) ? reverse_statement_list($4) : NULL;
            node_t* call = _OPERATOR(CALL);
            call->left = id;
            $$ = call;
            free($2);
        }
    ;
%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}

/* ---------- Semantic helper implementations ---------- */

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
    node->value.id_index = new_id;   /* replace name index with the new id index */
    ctx->name_table.n_ids++;
    return stack_push(ctx, new_id);
}

static int find_existing_id(lang_ctx_t* ctx, size_t name_idx) {
    stack_t* id_stack = &ctx->id_stack;
    name_t*  names    = ctx->name_table.names;
    identifier_t* ids = ctx->name_table.ids;

    for (int i = (int)id_stack->top - 1; i >= 0; i--) {
        size_t id_idx = id_stack->data[i];
        if (strcmp(names[name_idx].name, ids[id_idx].name) == 0) {
            return (int)id_idx;
        }
    }
    return -1;
}

