#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lang.h"
#include "custom_assert.h"
#include "graph_dump.h"
#include "node_allocator.h"
#include "io_interaction.h"
#include "read_name_table_utils.h"
#include "color_print.h"
#include "optimize_func.h"

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t middle_end_lang_ctx_ctor(lang_ctx_t* ctx,
                                              int         argc,
                                              char*       argv[]);

static lang_status_t middle_end_lang_ctx_dtor(lang_ctx_t* ctx);

static lang_status_t write_opt_ast_in_output(lang_ctx_t* ctx);

extern lang_status_t optimize_ast(lang_ctx_t* ctx);

//——————————————————————————————————————————————————————————————————————————————

int main(int argc, char* argv[])
{
    lang_ctx_t ctx = {};

    node_allocator_t node_allocator;
    ctx.node_allocator = &node_allocator;

    VERIFY(middle_end_lang_ctx_ctor(&ctx, argc, argv),
           return EXIT_FAILURE);

    VERIFY(read_input_ctx(&ctx),
           middle_end_lang_ctx_dtor(&ctx);
           return EXIT_FAILURE);

    VERIFY(graph_dump(&ctx, ctx.tree, TREE),
           middle_end_lang_ctx_dtor(&ctx);
           return EXIT_FAILURE);

    VERIFY(optimize(&ctx),
           middle_end_lang_ctx_dtor(&ctx);
           return EXIT_FAILURE);

    VERIFY(graph_dump(&ctx, ctx.tree, TREE),
           middle_end_lang_ctx_dtor(&ctx);
           return EXIT_FAILURE);

    VERIFY(write_opt_ast_in_output(&ctx),
           middle_end_lang_ctx_dtor(&ctx);
           return EXIT_FAILURE);

    VERIFY(middle_end_lang_ctx_dtor(&ctx),
           return EXIT_FAILURE);

    fprintf(stderr, _PURPLE("middle-end: ") _GREEN("success\n"));

    return EXIT_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_opt_ast_in_output(lang_ctx_t* ctx)
{
    ctx->output_file = fopen(ctx->ap_ctx.output_file, "wb");
    VERIFY(!ctx->output_file,           return LANG_ERROR);

    VERIFY(name_table_output(ctx),      return LANG_ERROR);
    VERIFY(tree_output(ctx, ctx->tree), return LANG_ERROR);

    fclose(ctx->output_file);
    ctx->output_file = nullptr;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t optimize(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    VERIFY(optimize_ast(ctx), return LANG_ERROR);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t middle_end_lang_ctx_ctor(lang_ctx_t* ctx, int argc, char* argv[])
{
    ASSERT(ctx);
    ASSERT(argv);

    VERIFY(parse_argv(argc, argv, &ctx->ap_ctx),
           return LANG_OPEN_FILES_ERROR);

    ctx->input_file = fopen(ctx->ap_ctx.input_file, "rb");
    VERIFY(!ctx->input_file, return LANG_FILE_OPEN_ERROR);

    VERIFY(read_in_buf(ctx->input_file,
                       &ctx->input_size,
                       &ctx->code),
           return LANG_READ_CODE_ERROR);

    ctx->nodes = (node_t**) calloc(ctx->input_size + 1, sizeof(node_t*));
    VERIFY(!ctx->nodes, return LANG_STD_ALLOCATE_ERROR);

    ctx->n_nodes = 0;
    ctx->pos = 0;

    ctx->name_table.ids = (identifier_t*) calloc(ctx->input_size, sizeof(identifier_t));
    VERIFY(!ctx->name_table.ids, return LANG_STD_ALLOCATE_ERROR);

    ctx->name_table.names = (name_t*) calloc(ctx->input_size,  sizeof(identifier_t));
    VERIFY(!ctx->name_table.names, return LANG_STD_ALLOCATE_ERROR);

    ctx->name_table.n_names = 0;
    ctx->n_globals = 0;
    ctx->n_locals = 0;
    ctx->emitting_global_init = false;
    ctx->level = 0;

    VERIFY(node_allocator_ctor(ctx->node_allocator, N_NODES_INIT),
           return LANG_NODE_ALLOCATOR_CTOR_ERROR);

    return LANG_SUCCESS;

}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t middle_end_lang_ctx_dtor(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    if (ctx->input_file) {
        fclose(ctx->input_file);
        ctx->input_file = nullptr;
    }

    if (ctx->output_file) {
        fclose(ctx->output_file);
        ctx->output_file = nullptr;
    }

    if (ctx->nodes) {
        free(ctx->nodes);
        ctx->nodes = nullptr;
    }

    VERIFY(node_allocator_dtor(ctx->node_allocator),
           return LANG_NODE_ALLOCATOR_DTOR_ERROR);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
