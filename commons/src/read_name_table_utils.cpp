#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "lang.h"
#include "custom_assert.h"
#include "graph_dump.h"
#include "argv_parser.h"
#include "read_name_table_utils.h"
#include "node_allocator.h"
#include "io_interaction.h"

//——————————————————————————————————————————————————————————————————————————————

static void ast_set_parents(node_t* node, node_t* parent) {
    if (!node) return;

    node->parent = parent;

    ast_set_parents(node->left,  node);
    ast_set_parents(node->right, node);
}

lang_status_t read_input_ctx(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    VERIFY(read_name_table(ctx),
           return LANG_ERROR);

    VERIFY(read_tree(ctx, &ctx->tree),
           return LANG_ERROR);
    
    ast_set_parents(ctx->tree, NULL);

    VERIFY(graph_dump(ctx, ctx->tree, GRAPH_DUMP_MODE_TREE),
           return LANG_ERROR);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t read_name_table(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    int n_chars = 0;

    size_t n_ids = 0;
    sscanf(ctx->code, "%ld%n", &n_ids, &n_chars);
    ctx->code += n_chars;
    ctx->name_table.n_ids = n_ids;

    ctx->name_table.ids = (identifier_t*) calloc(n_ids, sizeof(identifier_t));
    VERIFY(!ctx->name_table.ids,
           return LANG_STD_ALLOCATE_ERROR);

    int  len = 0;
    char buf[MaxStrLength] = {};
    int type = 0;
    int n_params = 0;
    int is_global;
    int nchars = 0;

    for (int i = 0; i < n_ids; i++) {
        sscanf(ctx->code, " { %d %s %d %d %d } %n",
               &len, buf, &type, &is_global, &n_params, &nchars);

        ctx->code += nchars;

        ctx->name_table.ids[i].type      = (identifier_type_t) type;
        ctx->name_table.ids[i].n_params  = n_params;
        ctx->name_table.ids[i].name      = strdup(buf);
        ctx->name_table.ids[i].is_global = is_global;
        ctx->name_table.ids[i].addr      = -1;
    }

    sscanf(ctx->code, " %ld%n", &ctx->n_nodes, &nchars);
    ctx->code += nchars;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t read_tree(lang_ctx_t* ctx, node_t** node)
{
    ASSERT(ctx);
    ASSERT(node);

    while (isspace(*ctx->code)) {
        ctx->code++;
    }

    if (*ctx->code == '_') {
        ctx->code++;
        *node = nullptr;

        return LANG_SUCCESS;
    }

    int nchars = 0;
    int type   = 0;

    sscanf(ctx->code, " { %d%n", &type, &nchars);
    ctx->code += nchars;

    value_t node_value = {};

    if(put_node_value(ctx, type, &node_value)) {
        return LANG_PUT_NODE_VALUE_ERROR;
    }

    *node = node_ctor(ctx->node_allocator,
                      (value_type_t) type,
                      node_value,
                      0,
                      nullptr,
                      nullptr);

    if (read_tree(ctx, &(*node)->left)) {
        return LANG_READ_LEFT_NODE_ERROR;
    }

    if (read_tree(ctx, &(*node)->right)) {
        return LANG_READ_RIGHT_NODE_ERROR;
    }

    while (isspace(*ctx->code)) {ctx->code++;}

    VERIFY(*(ctx)->code != '}',
           return LANG_INCORRECT_INPUT_SYNTAX_ERROR);

    ctx->code++;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t put_integer_node_value(lang_ctx_t* ctx, number_t* val)
{
	int nchars = 0;
	sscanf(ctx->code, " %" SCNd64 "%n", val, &nchars);
	ctx->code += nchars;
	return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t put_string_node_value(lang_ctx_t* ctx, char** string)
{
	assert(string);

	size_t len = 0;
	int nchars = 0;
	sscanf(ctx->code, " %ld%n", &len, &nchars);
	ctx->code += nchars;

	while (isspace(*ctx->code)) {
		ctx->code++;
	}

	*string = (char*) calloc(len + 1, sizeof(char));
	VERIFY(!*string, return LANG_STD_ALLOCATE_ERROR);
	memcpy(*string, ctx->code, len);
	(*string)[len] = '\0';
	ctx->code += len;
	return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t put_node_value(lang_ctx_t* ctx, int type, value_t* node_value)
{
    ASSERT(ctx);
    ASSERT(node_value);

    switch(type) {
        case OPERATOR: {
			number_t val = 0;
			put_integer_node_value(ctx, &val );
			node_value->operator_code = (operator_code_t)val;
            break;
        }
        case IDENTIFIER: {
			number_t val = 0;
			put_integer_node_value(ctx,&val );
			node_value->id_index = (size_t)val;
            break;
        }
        case NUMBER: {
			number_t val = 0;
			put_integer_node_value(ctx,&val );
			node_value->number = val;
            break;
        }
        case STRING: {
			put_string_node_value(ctx, &node_value->string);
            break;
        }
        default: {
            fprintf(stderr, "Unknown type: %d\n", type);

            return LANG_UNKNOWN_TYPE_ERROR;
        }
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t print_node_value(FILE* fp, node_t* node)
{
    ASSERT(fp);
    ASSERT(node);

    switch (node->value_type) {
        case OPERATOR: {
            fprintf(fp, "%d ", node->value.operator_code);
            break;
        }
        case IDENTIFIER: {
            fprintf(fp, "%ld ", node->value.id_index);
            break;
        }
        case NUMBER: {
            fprintf(fp, "%" PRId64 " ", node->value.number);
            break;
        }
        case STRING: {
            fprintf(fp, "%ld %s ", strlen(node->value.string), node->value.string);
            break;
        }
        default: {
            return LANG_PRINT_NODE_VALUE_ERROR;
            break;
        }
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t name_table_output(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    fprintf(ctx->output_file, "%ld\n", ctx->name_table.n_ids);

    for (int i = 0; i < ctx->name_table.n_ids; i++)
    {
        fprintf(ctx->output_file,
               "{ %-3ld %-10s %-1d %-1d %-2d }\n",
               ctx->name_table.ids[i].len,
               ctx->name_table.ids[i].name,
               ctx->name_table.ids[i].type,
               ctx->name_table.ids[i].is_global ? 1 : 0,
               ctx->name_table.ids[i].n_params);
    }

    fputc('\n', ctx->output_file);
    fprintf(ctx->output_file, "%ld\n", ctx->n_nodes);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t tree_output(lang_ctx_t* ctx, node_t* node)
{
    ASSERT(ctx);
    ASSERT(node);

    fprintf(ctx->output_file, "{%d ", node->value_type);

    VERIFY(print_node_value(ctx->output_file, node),
           return LANG_TREE_OUTPUT_ERROR);

    node->left  ? tree_output(ctx, node->left)  : fputs(" _ ", ctx->output_file);
    node->right ? tree_output(ctx, node->right) : fputs(" _ ", ctx->output_file);

    fputs("} ",  ctx->output_file);

    return LANG_SUCCESS;
}
