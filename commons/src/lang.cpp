#include "lang.h"
#include "custom_assert.h"
#include "node_allocator.h"
#include "io_interaction.h"

//———————————————————————————————————————————————————————————————————//

lang_status_t lang_ctx_ctor(lang_ctx_t* ctx,
                            int argc,
                            const char* argv[],
                            const char* default_input,
                            const char* default_output)
{
    ASSERT(ctx);
    ASSERT(argv);

    //---------------------------------------------------------------//

    VERIFY(open_files(&ctx->input_file,
                      &ctx->output_file,
                      argc,
                      argv,
                      default_input,
                      default_output),
           return LANG_OPEN_FILES_ERROR);

    //---------------------------------------------------------------//

    VERIFY(read_in_buf(ctx->input_file,
                       &ctx->input_size,
                       &ctx->code),
           return LANG_READ_CODE_ERROR);

    //---------------------------------------------------------------//

    ctx->nodes = (node_t**) calloc(ctx->input_size + 1, sizeof(node_t*));

    VERIFY(!ctx->nodes,
           return LANG_STD_ALLOCATE_ERROR);

    ctx->n_nodes = 0;

    ctx->pos = 0;

    //---------------------------------------------------------------//

    ctx->name_table.ids = (identifier_t*) calloc(ctx->input_size,
                                                   sizeof(identifier_t));

    ctx->name_table.names = (name_t*) calloc(ctx->input_size,
                                                   sizeof(identifier_t));

    ctx->id_stack.data = (size_t*) calloc(ctx->input_size,
                                                     sizeof(identifier_t));

    ctx->id_stack.size = ctx->input_size;

    ctx->id_counter_stack.data = (size_t*) calloc(ctx->input_size,
                                                     sizeof(identifier_t));

    ctx->id_counter_stack.size = ctx->input_size;

    // VERIFY(!ctx->name_table.ids,
        //    return LANG_STD_ALLOCATE_ERROR);

    ctx->name_table.n_names = 0;
    ctx->n_labels = 0;
    ctx->global_data_size = 0;
    ctx->cur_stack_frame_size = 0;
    ctx->cur_func_epilogue_label = 0;
    ctx->emitting_global_init = false;
    ctx->level = 0;
    //---------------------------------------------------------------//

    VERIFY(node_allocator_ctor(ctx->node_allocator,
                               nAllocatedNodes),
           return LANG_NODE_ALLOCATOR_CTOR_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//———————————————————————————————————————————————————————————————————//

lang_status_t lang_ctx_dtor(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    //----------------------------------------------------------------//

    if (ctx->input_file)  fclose(ctx->input_file);
    if (ctx->output_file) fclose(ctx->output_file);

    //----------------------------------------------------------------//

    if (ctx->nodes) free(ctx->nodes);

    //----------------------------------------------------------------//

//     for (int id_ind = 0; id_ind < ctx->name_table.n_names; id_ind++)
//     {
//         char* str = ctx->name_table.ids[id_ind].name;
//
//         if (str) free(str);
//     }

    //----------------------------------------------------------------//

    // if (ctx->name_table.table) free(ctx->name_table.table);

    //----------------------------------------------------------------//

    VERIFY(node_allocator_dtor(ctx->node_allocator),
           return LANG_NODE_ALLOCATOR_DTOR_ERROR);

    //---------------------------------------------------------------//

    return LANG_SUCCESS;
}

//———————————————————————————————————————————————————————————————————//
