#include <stdio.h>
#include <stdlib.h>
#include "lang.h"
#include "node_allocator.h"
#include "graph_dump.h"
#include "color_print.h"
#include "read_name_table_utils.h"

extern int yyparse(void);
extern FILE* yyin;

lang_ctx_t* ctx;

int main(int argc, const char* argv[])
{
    lang_ctx_t ctx_local = {};
    node_allocator_t node_allocator = {};
    ctx_local.node_allocator = &node_allocator;

    if (lang_ctx_ctor(&ctx_local, argc, argv, FrontendDefaultInput, FrontendDefaultOutput) != LANG_SUCCESS)
    {
        lang_ctx_dtor(&ctx_local);
        return EXIT_FAILURE;
    }

    rewind(ctx_local.input_file);

    ctx = &ctx_local;
    yyin = ctx_local.input_file;

    if (yyparse() != 0)
    {
        fprintf(stderr, "Parsing failed\n");
        lang_ctx_dtor(&ctx_local);
        return EXIT_FAILURE;
    }

    name_table_output(&ctx_local);
    tree_output(&ctx_local, ctx_local.tree);
    graph_dump(&ctx_local, ctx_local.tree, TREE);

    lang_ctx_dtor(&ctx_local);
    fprintf(stderr, _PURPLE("front-end:  ") _GREEN("success\n"));
    return EXIT_SUCCESS;
}