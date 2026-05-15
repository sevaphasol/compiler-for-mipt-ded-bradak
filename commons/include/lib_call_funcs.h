#ifndef _LIB_CALL_FUNCS_H__
#define _LIB_CALL_FUNCS_H__

//——————————————————————————————————————————————————————————————————————————————

#include "lang.h"
#include "buffer.h"
#include "lib_calls_table.h"

#include <stddef.h>

//——————————————————————————————————————————————————————————————————————————————

lang_status_t lib_calls_table_ctor(lib_calls_table_t*      table,
                                   size_t                  init_capacity);
lang_status_t lib_calls_table_dtor(lib_calls_table_t*      table);
lang_status_t add_lib_call_request(lib_calls_table_t*      table,
                                   const char*             name,
                                   size_t                  addr);
lang_status_t solve_lib_call_requests(lang_ctx_t* ctx);

//——————————————————————————————————————————————————————————————————————————————


lang_status_t stdlib_data_ctor(stdlib_data_t *data);
lang_status_t stdlib_data_dtor(stdlib_data_t *data);

lang_status_t stdlib_data_load(stdlib_data_t *data, const char *path);
lang_status_t stdlib_data_append_and_free(stdlib_data_t *data, buffer_t *bin_buf);

uint32_t      stdlib_data_get_offset(const stdlib_data_t *data, const char *name);

#endif // _LIB_CALL_FUNCS_H__
