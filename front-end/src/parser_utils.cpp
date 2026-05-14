#include "parser_utils.h"
#include "node_allocator.h"
#define _DSL_DEFINE_
#include "dsl.h"            
#undef _DSL_DEFINE_
#include <string.h>
#include <stdlib.h>
#include <stdio.h>             

extern lang_ctx_t* ctx;

size_t add_identifier(const char* name) {
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

size_t get_identifier_index(const char* name) {
    for (size_t i = 0; i < ctx->name_table.n_names; i++) {
        if (strcmp(ctx->name_table.names[i].name, name) == 0)
            return i;
    }
    /* auto-insert if not found (safe for forward references) */
    return add_identifier(name);
}

node_t* make_binary_op(operator_code_t op, node_t* left, node_t* right) {
    node_t* n = _OPERATOR(op);
    n->left = left;
    n->right = right;
    return n;
}

node_t* make_unary_op(operator_code_t op, node_t* operand) {
    node_t* n = _OPERATOR(op);
    n->left = operand;
    n->right = NULL;
    return n;
}

node_t* make_statement(node_t* left, node_t* right) {
    node_t* n = _OPERATOR(STATEMENT);
    n->left = left;
    n->right = right;
    return n;
}

node_t* make_linker(node_t* operand) {
    node_t* n = _OPERATOR(PARAM_LINKER);
    n->left = operand;
    n->right = NULL;
    return n;
}

size_t count_nodes(node_t* node) {
    if (!node) return 0;
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}

node_t* reverse_statement_list(node_t* head) {
    node_t* prev = NULL;
    node_t* curr = head;
    while (curr) {
        node_t* next = curr->right;
        curr->right = prev;
        prev = curr;
        curr = next;
    }
    return prev;
}

node_t* reverse_param_list(node_t* head) {
    node_t* prev = NULL;
    node_t* curr = head;
    while (curr) {
        node_t* next = curr->right;
        curr->right = prev;
        prev = curr;
        curr = next;
    }
    return prev;
}
