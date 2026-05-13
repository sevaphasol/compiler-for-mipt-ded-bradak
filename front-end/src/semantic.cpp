#include "lang.h"

#define _DSL_DEFINE_
#include "dsl.h"
#undef _DSL_DEFINE_

#include "custom_assert.h"
#include "color_print.h"
#include <string.h>

// ----------------------------------------------------------------------
// Forward declarations for recursive traversal
static lang_status_t semantic_program(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_statement_list(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_declaration(lang_ctx_t* ctx, node_t* node, bool is_global);
static lang_status_t semantic_func_declaration(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_var_declaration(lang_ctx_t* ctx, node_t* node, bool is_global);
static lang_status_t semantic_body(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_statement(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_expression(lang_ctx_t* ctx, node_t* node);
static lang_status_t semantic_identifier(lang_ctx_t* ctx, node_t* node);

// Stack management helpers
static lang_status_t stack_push(lang_ctx_t* ctx, size_t id_index);
static lang_status_t push_new_id_counter(lang_ctx_t* ctx);
static lang_status_t pop_locales(lang_ctx_t* ctx);
static lang_status_t check_var(lang_ctx_t* ctx, size_t* id_index, int mode);
static lang_status_t add_new_id(lang_ctx_t* ctx, identifier_type_t type, node_t* node, bool is_global);

enum { ON_REDECLARATION = 0, ON_INITED = 1 };

// ----------------------------------------------------------------------
lang_status_t semantic_analysis(lang_ctx_t* ctx) {
    ASSERT(ctx);
    ASSERT(ctx->tree);

    // Initialize stacks (already allocated in lang_ctx_ctor)
    ctx->id_stack.top = 0;
    ctx->id_counter_stack.top = 0;
    ctx->name_table.n_ids = 0;
    ctx->n_globals = 0;
    ctx->n_locals = 0;
    ctx->level = 0;

    // Push global scope
    VERIFY(push_new_id_counter(ctx), return LANG_ERROR);

    // Process the whole program
    VERIFY(semantic_program(ctx, ctx->tree), return LANG_ERROR);

    // Pop global scope
    VERIFY(pop_locales(ctx), return LANG_ERROR);

    return LANG_SUCCESS;
}

// ----------------------------------------------------------------------
static lang_status_t semantic_program(lang_ctx_t* ctx, node_t* node) {
    if (!node) return LANG_SUCCESS;
    if (node->value_type == OPERATOR && node->value.operator_code == STATEMENT) {
        // Process the rest of the list (right) first
        if (node->right) {
            VERIFY(semantic_program(ctx, node->right), return LANG_ERROR);
        }
        // Then process the current declaration (left)
        if (node->left) {
            VERIFY(semantic_declaration(ctx, node->left, true), return LANG_ERROR);
        }
    } else {
        // Single declaration (should not occur, but safe)
        VERIFY(semantic_declaration(ctx, node, true), return LANG_ERROR);
    }
    return LANG_SUCCESS;
}

// ----------------------------------------------------------------------
static lang_status_t semantic_declaration(lang_ctx_t* ctx, node_t* node, bool is_global) {
    if (node->value_type != OPERATOR) {
        fprintf(stderr, "Expected operator, got %d\n", node->value_type);
        return LANG_ERROR;
    }
    if (node->value.operator_code == NEW_FUNC) {
        return semantic_func_declaration(ctx, node);
    } else if (node->value.operator_code == NEW_VAR) {
        return semantic_var_declaration(ctx, node, is_global);
    } else {
        fprintf(stderr, "Unexpected top-level operator %s\n", OperatorsTable[node->value.operator_code].name);
        return LANG_ERROR;
    }
}

// ----------------------------------------------------------------------
static lang_status_t semantic_func_declaration(lang_ctx_t* ctx, node_t* node) {
    node_t* func_id = node->left;
    if (func_id->value_type != IDENTIFIER) {
        fprintf(stderr, "Function name is not an identifier\n");
        return LANG_ERROR;
    }

    // Check redeclaration
    size_t id_index = func_id->value.id_index;
    if (check_var(ctx, &id_index, ON_REDECLARATION) != LANG_SUCCESS) {
        fprintf(stderr, "Redeclaration of function '%s'\n", ctx->name_table.names[func_id->value.id_index].name);
        return LANG_REDECLARATION_ERROR;
    }

    // Add function to symbol table
    VERIFY(add_new_id(ctx, FUNC, func_id, true), return LANG_ERROR);

    // Count parameters
    int n_params = 0;
    node_t* param = func_id->left;
    while (param && param->value_type == OPERATOR && param->value.operator_code == PARAM_LINKER) {
        n_params++;
        param = param->right;
    }
    ctx->name_table.ids[id_index].n_params = n_params;

    // Push new scope for function body
    VERIFY(push_new_id_counter(ctx), return LANG_ERROR);

    // Process parameters (they become local variables)
    param = func_id->left;
    while (param && param->value_type == OPERATOR && param->value.operator_code == PARAM_LINKER) {
        node_t* var_decl = param->left;
        if (var_decl->value_type != OPERATOR || var_decl->value.operator_code != NEW_VAR) {
            fprintf(stderr, "Expected NEW_VAR in parameter list\n");
            return LANG_ERROR;
        }
        VERIFY(semantic_var_declaration(ctx, var_decl, false), return LANG_ERROR);
        param = param->right;
    }

    // Process body
    VERIFY(semantic_body(ctx, func_id->right), return LANG_ERROR);

    // Pop function scope
    VERIFY(pop_locales(ctx), return LANG_ERROR);

    return LANG_SUCCESS;
}

// ----------------------------------------------------------------------
static lang_status_t semantic_var_declaration(lang_ctx_t* ctx, node_t* node, bool is_global) {
    // node: NEW_VAR
    // Two possible structures:
    // 1) With initializer: node->left is ASSIGNMENT (left=ID, right=expr)
    // 2) Without initializer (parameter): node->left is directly IDENTIFIER
    node_t* var_id = NULL;
    node_t* init_expr = NULL;
    
    if (node->left->value_type == OPERATOR && node->left->value.operator_code == ASSIGNMENT) {
        node_t* assign = node->left;
        var_id = assign->left;
        init_expr = assign->right;
    } else if (node->left->value_type == IDENTIFIER) {
        var_id = node->left;
        init_expr = NULL;
    } else {
        fprintf(stderr, "Invalid NEW_VAR structure\n");
        return LANG_ERROR;
    }
    
    if (var_id->value_type != IDENTIFIER) {
        fprintf(stderr, "Variable name is not an identifier\n");
        return LANG_ERROR;
    }
    
    // Check redeclaration in current scope
    size_t id_index = var_id->value.id_index;
    if (check_var(ctx, &id_index, ON_REDECLARATION) != LANG_SUCCESS) {
        fprintf(stderr, "Redeclaration of variable '%s'\n", ctx->name_table.names[var_id->value.id_index].name);
        return LANG_REDECLARATION_ERROR;
    }
    
    // Add variable to symbol table
    VERIFY(add_new_id(ctx, VAR, var_id, is_global), return LANG_ERROR);
    
    // Assign stack offset for locals
    if (!is_global) {
        ctx->name_table.ids[id_index].addr = (int)(VAR_SIZE * (++ctx->n_locals));
    } else {
        ctx->name_table.ids[id_index].addr = 0;
        ctx->n_globals++;
    }
    
    // Process initializer expression if present
    if (init_expr) {
        VERIFY(semantic_expression(ctx, init_expr), return LANG_ERROR);
    }
    
    return LANG_SUCCESS;
}

// ----------------------------------------------------------------------
static lang_status_t semantic_body(lang_ctx_t* ctx, node_t* node) {
    return semantic_statement_list(ctx, node);
}


static lang_status_t semantic_statement_list(lang_ctx_t* ctx, node_t* node) {
    if (!node) return LANG_SUCCESS;
    if (node->value_type == OPERATOR && node->value.operator_code == STATEMENT) {
        if (node->right) {
            VERIFY(semantic_statement_list(ctx, node->right), return LANG_ERROR);
        }
        if (node->left) {
            VERIFY(semantic_statement(ctx, node->left), return LANG_ERROR);
        }
    } else {
        VERIFY(semantic_statement(ctx, node), return LANG_ERROR);
    }
    return LANG_SUCCESS;
}


static lang_status_t semantic_statement(lang_ctx_t* ctx, node_t* node) {
    if (!node) return LANG_SUCCESS;
    if (node->value_type == OPERATOR) {
        switch (node->value.operator_code) {
            case NEW_VAR:
                return semantic_var_declaration(ctx, node, false);
            case ASSIGNMENT:
                VERIFY(semantic_identifier(ctx, node->left), return LANG_ERROR);
                VERIFY(semantic_expression(ctx, node->right), return LANG_ERROR);
                break;
            case IF:
            case WHILE:
                VERIFY(semantic_expression(ctx, node->left), return LANG_ERROR);
                VERIFY(semantic_body(ctx, node->right), return LANG_ERROR);
                break;
            case RET:
                if (node->left)
                    VERIFY(semantic_expression(ctx, node->left), return LANG_ERROR);
                break;
            case IN:
                if (node->left && node->left->value_type == OPERATOR && node->left->value.operator_code == PARAM_LINKER) {
                    VERIFY(semantic_identifier(ctx, node->left->left), return LANG_ERROR);
                }
                break;
            case OUT:
                if (node->left && node->left->value_type == OPERATOR && node->left->value.operator_code == PARAM_LINKER) {
                    VERIFY(semantic_expression(ctx, node->left->left), return LANG_ERROR);
                }
                break;
            case CALL: {
                VERIFY(semantic_identifier(ctx, node->left), return LANG_ERROR);
                size_t func_id_idx = node->left->value.id_index;
                int expected = ctx->name_table.ids[func_id_idx].n_params;
                int got = 0;
                node_t* args = node->left->left;
                while (args && args->value_type == OPERATOR && args->value.operator_code == PARAM_LINKER) {
                    got++;
                    args = args->right;
                }
                if (got != expected) {
                    fprintf(stderr, "Function %s expects %d arguments, got %d\n",
                            ctx->name_table.ids[func_id_idx].name, expected, got);
                    return LANG_ERROR;
                }
                args = node->left->left;
                while (args && args->value_type == OPERATOR && args->value.operator_code == PARAM_LINKER) {
                    VERIFY(semantic_expression(ctx, args->left), return LANG_ERROR);
                    args = args->right;
                }
                break;
            }
            default:
                VERIFY(semantic_expression(ctx, node), return LANG_ERROR);
                break;
        }
    } else if (node->value_type == IDENTIFIER) {
        VERIFY(semantic_identifier(ctx, node), return LANG_ERROR);
    } else if (node->value_type == NUMBER) {
        // OK
    } else {
        fprintf(stderr, "Unknown statement type %d\n", node->value_type);
        return LANG_ERROR;
    }
    return LANG_SUCCESS;
}

static lang_status_t semantic_expression(lang_ctx_t* ctx, node_t* node) {
    if (!node) return LANG_SUCCESS;
    if (node->value_type == NUMBER) {
        return LANG_SUCCESS;
    }
    if (node->value_type == IDENTIFIER) {
        return semantic_identifier(ctx, node);
    }
    if (node->value_type == OPERATOR) {
        if (node->left) VERIFY(semantic_expression(ctx, node->left), return LANG_ERROR);
        if (node->right) VERIFY(semantic_expression(ctx, node->right), return LANG_ERROR);
        return LANG_SUCCESS;
    }
    fprintf(stderr, "Invalid expression node type %d\n", node->value_type);
    return LANG_ERROR;
}

static lang_status_t semantic_identifier(lang_ctx_t* ctx, node_t* node) {
    if (node->value_type != IDENTIFIER) {
        fprintf(stderr, "semantic_identifier called on non-identifier\n");
        return LANG_ERROR;
    }
    size_t id_index = node->value.id_index;
    if (check_var(ctx, &id_index, ON_INITED) != LANG_SUCCESS) {
        fprintf(stderr, "Variable '%s' used before initialization\n", ctx->name_table.names[node->value.id_index].name);
        return LANG_NOT_INIT_ERROR;
    }
    node->value.id_index = id_index;
    return LANG_SUCCESS;
}

// ----------------------------------------------------------------------
// Stack and symbol table helpers
static lang_status_t stack_push(lang_ctx_t* ctx, size_t id_index) {
    if (ctx->id_stack.top >= ctx->id_stack.size) return LANG_ID_STACK_OVERFLOW_ERROR;
    ctx->id_stack.data[ctx->id_stack.top++] = id_index;
    ctx->id_counter_stack.data[ctx->id_counter_stack.top - 1]++;
    return LANG_SUCCESS;
}

static lang_status_t push_new_id_counter(lang_ctx_t* ctx) {
    if (ctx->id_counter_stack.top >= ctx->id_counter_stack.size) return LANG_ID_COUNTER_STACK_OVERFLOW_ERROR;
    ctx->id_counter_stack.data[ctx->id_counter_stack.top++] = 0;
    return LANG_SUCCESS;
}

static lang_status_t pop_locales(lang_ctx_t* ctx) {
    if (ctx->id_counter_stack.top == 0) return LANG_POP_LOCALES_ERROR;
    ctx->id_stack.top -= ctx->id_counter_stack.data[--ctx->id_counter_stack.top];
    return LANG_SUCCESS;
}

static lang_status_t check_var(lang_ctx_t* ctx, size_t* id_index, int mode) {
    size_t idx = *id_index;
    const char* name = ctx->name_table.names[idx].name;
    // Search from innermost scope outward
    for (int i = (int)ctx->id_stack.top - 1; i >= 0; i--) {
        size_t candidate = ctx->id_stack.data[i];
        if (strcmp(ctx->name_table.ids[candidate].name, name) == 0) {
            if (mode == ON_REDECLARATION) return LANG_REDECLARATION_ERROR;
            *id_index = candidate;
            return LANG_SUCCESS;
        }
    }
    if (mode == ON_INITED) return LANG_NOT_INIT_ERROR;
    return LANG_SUCCESS;
}

static lang_status_t add_new_id(lang_ctx_t* ctx, identifier_type_t type, node_t* node, bool is_global) {
    size_t name_index = node->value.id_index;
    const char* name = ctx->name_table.names[name_index].name;
    size_t len = ctx->name_table.names[name_index].len;

    size_t new_id = ctx->name_table.n_ids;
    ctx->name_table.ids[new_id] = (identifier_t){
        .type = type,
        .name = (char*)name,
        .len = len,
        .n_params = 0,
        .is_inited = true,
        .addr = 0,
        .is_global = is_global,
        .is_stdlib = 0
    };
    node->value.id_index = new_id;
    ctx->name_table.n_ids++;
    VERIFY(stack_push(ctx, new_id), return LANG_ERROR);
    return LANG_SUCCESS;
}