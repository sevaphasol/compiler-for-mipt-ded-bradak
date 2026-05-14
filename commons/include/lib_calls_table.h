#ifndef _LIB_CALLS_TABLE_H__
#define _LIB_CALLS_TABLE_H__

//——————————————————————————————————————————————————————————————————————————————

#include <stddef.h>

//——————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————

struct lib_call_request_t {
    const char             *name;
    size_t                  addr;
};

//——————————————————————————————————————————————————————————————————————————————

struct lib_calls_table_t {
    lib_call_request_t* requests;
    size_t              capacity;
    size_t              size;

    size_t              in_addr;
    size_t              out_addr;
};

//——————————————————————————————————————————————————————————————————————————————

struct stdlib_entry_t {
    char               *name;
    uint32_t            offset;
};

struct stdlib_data_t {
    stdlib_entry_t     *entries;
    uint32_t            num_funcs;
    uint8_t            *text_data;
    size_t              text_size;
    size_t              base_offset; // where the .text was appended in bin_buf
};

//——————————————————————————————————————————————————————————————————————————————

#endif // _LIB_CALLS_TABLE_H__
