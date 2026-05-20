#ifndef _FIXUP_TABLE_H__
#define _FIXUP_TABLE_H__

//——————————————————————————————————————————————————————————————————————————————

#include <stddef.h>
#include <stdint.h>
#include "lang_status.h"
#include "buffer.h"

//——————————————————————————————————————————————————————————————————————————————

typedef enum {
    FIXUP_TYPE_JMP,
    FIXUP_TYPE_JE,
    FIXUP_TYPE_JNE,
    FIXUP_TYPE_CALL,
} fixup_type_t;

//——————————————————————————————————————————————————————————————————————————————

union label_value_t {
    char* global_name;
    size_t local_number;
};

//——————————————————————————————————————————————————————————————————————————————

struct label_t {
    size_t address;
    label_value_t value;
    bool is_global;
};

//——————————————————————————————————————————————————————————————————————————————

struct label_table_t {
    label_t* labels;
    size_t   size;
    size_t   capacity;
};

//——————————————————————————————————————————————————————————————————————————————

struct fixup_entry_t {
    uint32_t offset;
    uint32_t rel_base;
    label_t  label;
};

//——————————————————————————————————————————————————————————————————————————————

struct fixup_table_t {
    fixup_entry_t* entries;
    size_t         size;
    size_t         capacity;
};

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_ctor(label_table_t* table, size_t init_capacity);
lang_status_t label_table_dtor(label_table_t* table);
lang_status_t label_table_add_global(label_table_t* table, const char* name, size_t address);
lang_status_t label_table_add_local(label_table_t* table, size_t number, size_t address);
lang_status_t label_table_find_global(label_table_t* table, const char* name, uint32_t* offset);
lang_status_t label_table_find_local(label_table_t* table, size_t number, uint32_t* offset);

lang_status_t fixup_table_ctor(fixup_table_t* table, size_t init_capacity);
lang_status_t fixup_table_dtor(fixup_table_t* fixups);
lang_status_t add_fixup(fixup_table_t* table,
                        const char*    global_name,
                        size_t         local_number,
                        uint32_t       offset,
                        uint32_t       rel_base);
lang_status_t fixup_table_apply(fixup_table_t* table, label_table_t* label_table, buffer_t* code_buf);
lang_status_t fixup_table_apply_and_collect_external(fixup_table_t* table,
                                                     label_table_t* label_table,
                                                     buffer_t*      code_buf,
                                                     fixup_table_t* external_fixups);

//——————————————————————————————————————————————————————————————————————————————

#endif // _FIXUP_TABLE_H__
