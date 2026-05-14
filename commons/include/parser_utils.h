#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "lang.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t add_identifier(const char* name);
size_t get_identifier_index(const char* name);

node_t* make_binary_op(operator_code_t op, node_t* left, node_t* right);
node_t* make_unary_op(operator_code_t op, node_t* operand);
node_t* make_statement(node_t* left, node_t* right);
node_t* make_linker(node_t* operand);

size_t count_nodes(node_t* node);
node_t* reverse_statement_list(node_t* head);
node_t* reverse_param_list(node_t* head);

#ifdef __cplusplus
}
#endif

#endif // PARSER_UTILS_H