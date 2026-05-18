#include "lang_status.h"
#include "fixup_table.h"
#include "custom_assert.h"
#include <stdlib.h>
#include <string.h>

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_ctor(label_table_t* table, size_t init_capacity)
{
    ASSERT(table);

    table->labels = (label_t*) calloc(init_capacity, sizeof(label_t));

    if (!table->labels) {
        return LANG_STD_ALLOCATE_ERROR;
    }

    table->capacity = init_capacity;
    table->size     = 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_dtor(label_table_t* table)
{
    ASSERT(table);

    for (size_t i = 0; i < table->size; ++i) {
        if (table->labels[i].is_global &&
            table->labels[i].value.global_name) {
            free(table->labels[i].value.global_name); // for strdup
        }
    }

    free(table->labels);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_expand(label_table_t* table)
{
    ASSERT(table);

    size_t new_capacity = table->capacity * 2;
    label_t* new_labels = (label_t*) realloc(table->labels, new_capacity * sizeof(label_t));

    if (!new_labels) {
        return LANG_STD_ALLOCATE_ERROR;
    }

    table->labels = new_labels;
    table->capacity = new_capacity;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_add_global(label_table_t* table, const char* name, size_t address)
{
    ASSERT(table);
    ASSERT(name);

    if (table->size >= table->capacity) {
        label_table_expand(table);
    }

    label_value_t value = {.global_name = strdup(name)};

    table->labels[table->size++] = {
        .address   = address,
        .value     = value,
        .is_global = 1
    };

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_add_local(label_table_t* table, size_t number, size_t address)
{
    ASSERT(table);

    if (table->size >= table->capacity) {
        label_table_expand(table);
    }

    label_value_t value = {.local_number = number};

    table->labels[table->size++] = {
        .address   = address,
        .value     = value,
        .is_global = 0
    };

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_find_global(label_table_t* table, const char* name, uint32_t* addr)
{
    ASSERT(table);
    ASSERT(name);

    for (size_t i = 0; i < table->size; ++i) { //TODO hashmap
        if (table->labels[i].is_global &&
            strcmp(table->labels[i].value.global_name, name) == 0) {
            *addr = (uint32_t)table->labels[i].address;
            return LANG_SUCCESS;
        }
    }

    return LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t label_table_find_local(label_table_t* table, size_t number, uint32_t* addr)
{
    ASSERT(table);
    ASSERT(addr);

    size_t table_size = table->size;

    for (size_t i = 0; i < table_size; ++i) {
        if (!table->labels[i].is_global &&
            table->labels[i].value.local_number == number) {
            *addr = (uint32_t)table->labels[i].address;
            return LANG_SUCCESS;
        }
    }

    return LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————
