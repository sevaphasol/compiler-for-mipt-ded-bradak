#include "lang.h"
#include "custom_assert.h"

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t fold_constants(node_t* tree);
static lang_status_t try_fold_node(node_t* node);
static lang_status_t calculate_node(node_t* node);

// static lang_status_t solve_trivial_arithmetic(node_t* node);
// static lang_status_t solve_add(node_t* node);
// static lang_status_t solve_sub(node_t* node);
// static lang_status_t solve_mul(node_t* node);
// static lang_status_t solve_div(node_t* node);
// static lang_status_t solve_sqrt(node_t* node);

static bool is_number(node_t* node);
static bool is_math_op(node_t* node);

//——————————————————————————————————————————————————————————————————————————————

lang_status_t optimize_ast(lang_ctx_t* ctx)
{
    ASSERT(ctx);
    return fold_constants(ctx->tree);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t fold_constants(node_t* tree)
{
    return try_fold_node(tree);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t try_fold_node(node_t* node)
{
    ASSERT(node);

    if (node->left)  try_fold_node(node->left);
    if (node->right) try_fold_node(node->right);

    if (is_math_op(node) && is_number(node->left)
                         && is_number(node->right)) {
        calculate_node(node);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t calculate_node(node_t* node)
{
    ASSERT(node);

    if (node->value_type != OPERATOR) {
        return LANG_NOT_OPERATOR_STATUS;
    }

    ASSERT(node->left);

    number_t value, rval = 0;
    number_t lval = node->left->value.number;

    if (node->right) {
        rval = node->right->value.number;
    }

    switch (node->value.operator_code) {
        case ADD:  value = lval + rval; break;
        case SUB:  value = lval - rval; break;
        case MUL:  value = lval * rval; break;
        case DIV:  value = lval / rval; break;
        default:   return LANG_SUCCESS;
    }

    node->value_type   = NUMBER;
    node->value.number = value;
    node->left         = nullptr;
    node->right        = nullptr;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

bool is_number(node_t* node)
{
    return node && node->value_type == NUMBER;
}

//——————————————————————————————————————————————————————————————————————————————

bool is_math_op(node_t* node)
{
    ASSERT(node);

    operator_code_t opc = node->value.operator_code;

    return (node->value_type == OPERATOR) && (opc == ADD  ||
                                              opc == SUB  ||
                                              opc == MUL  ||
                                              opc == DIV  ||
                                              opc == SQRT ||
                                              opc == POW  ||
                                              opc == SIN  ||
                                              opc == COS);
}

//——————————————————————————————————————————————————————————————————————————————
//
// lang_status_t solve_trivial_arithmetic(node_t* node)
// {
//     ASSERT(node);
//
//     if (!is_math_op(node)) {
//         return LANG_ERROR;
//     }
//
//     operator_code_t opc = node->value.operator_code;
//
//     switch(opc) {
//         case ADD:
//             solve_add(node);
//             break;
//         case SUB:
//             solve_sub(node);
//             break;
//         case MUL:
//             solve_mul(node);
//             break;
//         case DIV:
//             solve_div(node);
//             break;
//         case SQRT:
//             solve_sqrt(node);
//             break;
//         default:
//             return LANG_ERROR;
//     }
//
//     return LANG_SUCCESS;
// }
//
// //——————————————————————————————————————————————————————————————————————————————
//
// lang_status_t solve_add(node_t* node)
// {
//     ASSERT(node);
//
//     bool left_is_zero = is_number(node->left) && node->left->value.number == 0;
//
//     if (left_is_zero) {
//         *node = *node->right;
//         return LANG_SUCCESS;
//     }
//
//     bool right_is_zero = is_number(node->right) && node->right->value.number == 0;
//
//     if (right_is_zero) {
//         *node = *node->left;
//         return LANG_SUCCESS;
//     }
//
//     return LANG_SUCCESS;
// }
//
// lang_status_t solve_sub(node_t* node)
// {
//     ASSERT(node);
//
//     return LANG_SUCCESS;
// }
//
// lang_status_t solve_mul(node_t* node)
// {
//     ASSERT(node);
//
//     return LANG_SUCCESS;
// }
//
// lang_status_t solve_div(node_t* node)
// {
//     ASSERT(node);
//
//     return LANG_SUCCESS;
// }
//
// lang_status_t solve_sqrt(node_t* node)
// {
//     ASSERT(node);
//
//     return LANG_SUCCESS;
// }
//
// //——————————————————————————————————————————————————————————————————————————————
//
// lang_status_t solve_if_child_is_zero(node_t* node, node_t* child1, node_)
//
// //——————————————————————————————————————————————————————————————————————————————
