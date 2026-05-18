#ifndef _READ_NAME_TABLE_UTILS_H__
#define _READ_NAME_TABLE_UTILS_H__

//——————————————————————————————————————————————————————————————————————————————

#include "lang.h"

#define N_NODES_INIT 1024

//——————————————————————————————————————————————————————————————————————————————

lang_status_t read_tree               (lang_ctx_t* ctx,
                                       node_t**    node);

lang_status_t put_node_value          (lang_ctx_t* ctx,
                                       int         type,
                                       value_t*    node_value);

lang_status_t read_name_table         (lang_ctx_t* ctx);

lang_status_t read_input_ctx          (lang_ctx_t* ctx);

lang_status_t name_table_output       (lang_ctx_t* ctx);

lang_status_t tree_output             (lang_ctx_t* ctx, node_t* node);

//——————————————————————————————————————————————————————————————————————————————

#endif // _READ_NAME_TABLE_UTILS_H__
