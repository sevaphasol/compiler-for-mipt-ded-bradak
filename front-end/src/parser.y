%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _DSL_DEFINE_
#include "dsl.h"
#undef _DSL_DEFINE_

#include "lang.h"
#include "node_allocator.h"

extern lang_ctx_t* ctx;
extern int yylex(void);

void yyerror(const char* s);

static size_t add_identifier(const char* name);
static size_t get_identifier_index(const char* name);
static node_t* make_binary_op(operator_code_t op, node_t* left, node_t* right);
static node_t* make_unary_op(operator_code_t op, node_t* operand);
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

%type <node> program top_level_list top_level_decl function_def var_decl
%type <node> param_list param
%type <node> body statement_list statement
%type <node> assignment if_stmt while_stmt return_stmt
%type <node> print_stmt scan_stmt call_stmt
%type <node> expression additive_expr multiplicative_expr unary_expr primary_expr
%type <node> argument_list

%%

program
    : top_level_list
        { 
            ctx->tree = $1; 
            ctx->nodes[0] = $1;   /* Some functions expect nodes[0] */
            ctx->n_nodes = 1;
        }
    ;

top_level_list
    : /* empty */
        { $$ = NULL; }
    | top_level_decl
        { 
            /* Wrap single declaration in a STATEMENT node */
            node_t* stmt = _OPERATOR(STATEMENT);
            stmt->left = $1;
            stmt->right = NULL;
            $$ = stmt;
        }
    | top_level_list top_level_decl
        {
            node_t* stmt = _OPERATOR(STATEMENT);
            stmt->left = $1;
            stmt->right = $2;
            $$ = stmt;
        }
    ;

top_level_decl
    : TK_TILDE function_def
        { $$ = $2; }
    | TK_TILDE var_decl
        { $$ = $2; }
    ;

function_def
    : TK_FUNC TK_IDENTIFIER TK_L_ROUND param_list TK_R_ROUND body
        {
            node_t* func_node = _OPERATOR(NEW_FUNC);
            node_t* func_id   = _IDENTIFIER(add_identifier($2));
            free($2);
            func_node->left = func_id;
            func_node->left->left = $4;
            func_node->left->right = $6;
            $$ = func_node;
        }
    ;

param_list
    : /* empty */
        { $$ = NULL; }
    | TK_COLON TK_VAR TK_IDENTIFIER
        {
            node_t* var_decl = _OPERATOR(NEW_VAR);
            node_t* var_id   = _IDENTIFIER(add_identifier($3));
            free($3);
            var_decl->left = var_id;
            $$ = var_decl;
        }
    | param_list TK_COLON TK_VAR TK_IDENTIFIER
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $1;
            node_t* var_decl = _OPERATOR(NEW_VAR);
            node_t* var_id   = _IDENTIFIER(add_identifier($4));
            free($4);
            var_decl->left = var_id;
            linker->right = var_decl;
            $$ = linker;
        }
    ;

body
    : TK_L_CURLY statement_list TK_R_CURLY
        { $$ = $2; }
    ;

statement_list
    : /* empty */
        { $$ = NULL; }
    | statement_list statement
        {
            if ($1 == NULL) $$ = $2;
            else {
                node_t* seq = _OPERATOR(STATEMENT);
                seq->left = $1;
                seq->right = $2;
                $$ = seq;
            }
        }
    ;

statement
    : TK_TILDE var_decl      { $$ = $2; }
    | TK_TILDE assignment    { $$ = $2; }
    | TK_TILDE if_stmt       { $$ = $2; }
    | TK_TILDE while_stmt    { $$ = $2; }
    | TK_TILDE return_stmt   { $$ = $2; }
    | TK_TILDE print_stmt    { $$ = $2; }
    | TK_TILDE scan_stmt     { $$ = $2; }
    | TK_TILDE call_stmt     { $$ = $2; }
    | TK_TILDE expression    { $$ = $2; }
    ;

var_decl
    : TK_VAR TK_IDENTIFIER TK_ASSIGN expression
        {
            node_t* var_node = _OPERATOR(NEW_VAR);
            node_t* assign   = _OPERATOR(ASSIGNMENT);
            node_t* id_node  = _IDENTIFIER(add_identifier($2));
            free($2);
            assign->left = id_node;
            assign->right = $4;
            var_node->left = assign;
            $$ = var_node;
        }
    ;

assignment
    : TK_IDENTIFIER TK_ASSIGN expression
        {
            node_t* assign = _OPERATOR(ASSIGNMENT);
            node_t* id_node = _IDENTIFIER(get_identifier_index($1));
            free($1);
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
    ;

print_stmt
    : TK_PRINT TK_L_ROUND TK_COLON expression TK_R_ROUND
        {
            node_t* print = _OPERATOR(OUT);
            print->left = $4;
            $$ = print;
        }
    ;

scan_stmt
    : TK_SCAN TK_L_ROUND TK_COLON TK_IDENTIFIER TK_R_ROUND
        {
            node_t* scan = _OPERATOR(IN);
            node_t* id = _IDENTIFIER(get_identifier_index($4));
            free($4);
            scan->left = id;
            $$ = scan;
        }
    ;

call_stmt
    : TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            node_t* call = _OPERATOR(CALL);
            node_t* id = _IDENTIFIER(get_identifier_index($2));
            free($2);
            call->left = id;
            call->left->left = $4;
            $$ = call;
        }
    ;

argument_list
    : /* empty */
        { $$ = NULL; }
    | TK_COLON expression
        { $$ = $2; }
    | argument_list TK_COLON expression
        {
            node_t* linker = _OPERATOR(PARAM_LINKER);
            linker->left = $1;
            linker->right = $3;
            $$ = linker;
        }
    ;

expression
    : additive_expr
        { $$ = $1; }
    ;

additive_expr
    : multiplicative_expr
        { $$ = $1; }
    | additive_expr TK_PLUS multiplicative_expr
        { $$ = make_binary_op(ADD, $1, $3); }
    | additive_expr TK_MINUS multiplicative_expr
        { $$ = make_binary_op(SUB, $1, $3); }
    ;

multiplicative_expr
    : unary_expr
        { $$ = $1; }
    | multiplicative_expr TK_STAR unary_expr
        { $$ = make_binary_op(MUL, $1, $3); }
    | multiplicative_expr TK_SLASH unary_expr
        { $$ = make_binary_op(DIV, $1, $3); }
    ;

unary_expr
    : primary_expr
        { $$ = $1; }
    | TK_MINUS primary_expr
        { $$ = make_unary_op(SUB, $2); }
    | TK_SQRT TK_L_ROUND TK_COLON primary_expr TK_R_ROUND
        { $$ = make_unary_op(SQRT, $4); }
    ;

primary_expr
    : TK_NUMBER
        { $$ = _NUMBER($1); }
    | TK_IDENTIFIER
        { $$ = _IDENTIFIER(get_identifier_index($1)); free($1); }
    | TK_L_ROUND expression TK_R_ROUND
        { $$ = $2; }
    | TK_CALL TK_IDENTIFIER TK_L_ROUND argument_list TK_R_ROUND
        {
            node_t* call = _OPERATOR(CALL);
            node_t* id = _IDENTIFIER(get_identifier_index($2));
            free($2);
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