#include "custom_assert.h"
#include "buffer.h"
#include "lib_call_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//——————————————————————————————————————————————————————————————————————————————

lang_status_t lib_calls_table_ctor(lib_calls_table_t* table,
                                   size_t             init_capacity)
{
    ASSERT(table);

    table->requests = (lib_call_request_t*) calloc(init_capacity,
                                                   sizeof(lib_call_request_t));

    if (!table->requests) {
        return LANG_ERROR;
    }

    table->capacity = init_capacity;
    table->size = 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t lib_calls_table_dtor(lib_calls_table_t* table)
{
    ASSERT(table);

    if (!table->requests) {
        return LANG_ERROR;
    }

    free(table->requests);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t add_lib_call_request(lib_calls_table_t*      table,
                                   const char*             name,
                                   size_t                  addr)
{
    ASSERT(table);
    ASSERT(name);

    if (table->size >= table->capacity) {
        table->requests = (lib_call_request_t*) realloc(table->requests,
                                                        table->capacity *
                                                        2 * sizeof(lib_call_request_t));
        if (!table->requests) {
            return LANG_ERROR;
        }

        table->capacity *= 2;
    }

    table->requests[table->size++] = {
        .name = name,
        .addr = addr
    };

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

// ----------------------------------------------------------------------
// Combined stdlib.bin format:
//   magic     : 4 bytes "STDL"
//   version   : 4 bytes (little-endian, currently 1)
//   num_funcs : 4 bytes
//   for each function:
//     name     : null-terminated string
//     offset   : 4 bytes (little-endian, offset from start of .text)
//   raw .text : remaining bytes
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------------

static lang_status_t read_stdlib_header(FILE *fp, uint32_t *out_num_funcs) {
    char magic[4];
    if (fread(magic, 1, 4, fp) != 4 || memcmp(magic, "STDL", 4) != 0)
        return LANG_ERROR;
    uint32_t version;
    if (fread(&version, 4, 1, fp) != 1 || version != 1)
        return LANG_ERROR;
    uint32_t num_funcs;
    if (fread(&num_funcs, 4, 1, fp) != 1)
        return LANG_ERROR;
    *out_num_funcs = num_funcs;
    return LANG_SUCCESS;
}

static lang_status_t read_stdlib_entries(FILE *fp, uint32_t num_funcs, stdlib_entry_t **out_entries) {
    stdlib_entry_t *entries = (stdlib_entry_t*)calloc(num_funcs, sizeof(stdlib_entry_t));
    if (!entries) return LANG_STD_ALLOCATE_ERROR;
    for (uint32_t i = 0; i < num_funcs; ++i) {
        char name_buf[256];
        size_t pos = 0;
        int c;
        while ((c = fgetc(fp)) != EOF && c != '\0' && pos < sizeof(name_buf)-1)
            name_buf[pos++] = (char)c;
        if (c != '\0') {
            for (uint32_t j = 0; j < i; ++j) free(entries[j].name);
            free(entries);
            return LANG_ERROR;
        }
        name_buf[pos] = '\0';
        entries[i].name = strdup(name_buf);
        if (fread(&entries[i].offset, 4, 1, fp) != 1) {
            free(entries[i].name);
            for (uint32_t j = 0; j < i; ++j) free(entries[j].name);
            free(entries);
            return LANG_FREAD_ERROR;
        }
    }
    *out_entries = entries;
    return LANG_SUCCESS;
}

static lang_status_t read_stdlib_text(FILE *fp, uint8_t **out_text, size_t *out_size) {
    long text_start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    size_t text_size = (size_t)(file_size - text_start);
    uint8_t *text = (uint8_t*)malloc(text_size);
    if (!text) return LANG_STD_ALLOCATE_ERROR;
    fseek(fp, text_start, SEEK_SET);
    if (fread(text, 1, text_size, fp) != text_size) {
        free(text);
        return LANG_FREAD_ERROR;
    }
    *out_text = text;
    *out_size = text_size;
    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

// ----------------------------------------------------------------------
// Public functions
// ----------------------------------------------------------------------

lang_status_t stdlib_data_ctor(stdlib_data_t *data) {
    if (!data) return LANG_ERROR;
    memset(data, 0, sizeof(stdlib_data_t));
    return LANG_SUCCESS;
}

lang_status_t stdlib_data_dtor(stdlib_data_t *data) {
    if (!data) return LANG_ERROR;
    for (uint32_t i = 0; i < data->num_funcs; ++i)
        free(data->entries[i].name);
    free(data->entries);
    free(data->text_data);
    data->entries = NULL;
    data->num_funcs = 0;
    data->text_data = NULL;
    data->text_size = 0;
    data->base_offset = 0;
    return LANG_SUCCESS;
}

lang_status_t stdlib_data_load(stdlib_data_t *data, const char *path) {
    if (!data) return LANG_ERROR;

    if (data->entries) {
        stdlib_data_dtor(data); 
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "stdlib_data_load: cannot open %s\n", path);
        return LANG_FILE_OPEN_ERROR;
    }

    uint32_t num_funcs = 0;
    if (read_stdlib_header(fp, &num_funcs) != LANG_SUCCESS) {
        fclose(fp);
        return LANG_ERROR;
    }

    stdlib_entry_t *entries = NULL;
    if (read_stdlib_entries(fp, num_funcs, &entries) != LANG_SUCCESS) {
        fclose(fp);
        return LANG_ERROR;
    }

    uint8_t *text = NULL;
    size_t text_size = 0;
    if (read_stdlib_text(fp, &text, &text_size) != LANG_SUCCESS) {
        for (uint32_t i = 0; i < num_funcs; ++i) free(entries[i].name);
        free(entries);
        fclose(fp);
        return LANG_ERROR;
    }
    fclose(fp);

    data->entries = entries;
    data->num_funcs = num_funcs;
    data->text_data = text;
    data->text_size = text_size;
    data->base_offset = 0;
    return LANG_SUCCESS;
}

lang_status_t stdlib_data_append_and_free(stdlib_data_t *data, buffer_t *bin_buf) {
    if (!data || !bin_buf) return LANG_ERROR;
    if (!data->text_data) return LANG_SUCCESS; 

    data->base_offset = bin_buf->size;
    buf_write(bin_buf, data->text_data, data->text_size);

    free(data->text_data);
    data->text_data = NULL;
    data->text_size = 0;

    // printf("stdlib: appended .text (%zu bytes) at offset 0x%zx\n",
    //        data->text_size, data->base_offset);
    return LANG_SUCCESS;
}

uint32_t stdlib_data_get_offset(const stdlib_data_t *data, const char *name) {
    if (!data || !name) return (uint32_t)-1;
    for (uint32_t i = 0; i < data->num_funcs; ++i) {
        if (strcmp(data->entries[i].name, name) == 0)
            return data->entries[i].offset;
    }
    return (uint32_t)-1;
}

lang_status_t solve_lib_call_requests(lang_ctx_t *ctx) {
    ASSERT(ctx);

    if (ctx->stdlib_data.num_funcs == 0) {
        VERIFY(stdlib_data_load(&ctx->stdlib_data, "./back-end/lib/stdlib.bin"),
               return LANG_ERROR);
        VERIFY(stdlib_data_append_and_free(&ctx->stdlib_data, &ctx->bin_buf),
               return LANG_ERROR);
    }

    for (size_t i = 0; i < ctx->lib_calls_table.size; ++i) {
        lib_call_request_t *req = &ctx->lib_calls_table.requests[i];
        const char *func_name = req->name;
        uint32_t offset = stdlib_data_get_offset(&ctx->stdlib_data, func_name);
        if (offset == (uint32_t)-1) {
            fprintf(stderr, "ERROR: stdlib function '%s' not found\n", func_name);
            return LANG_ERROR;
        }

        uint64_t target_addr = ctx->stdlib_data.base_offset + offset;
        uint32_t patch_addr = (uint32_t)req->addr;   // position of the 4‑byte placeholder
        int32_t rel = (int32_t)(target_addr - (patch_addr + 4));

        memcpy(ctx->bin_buf.data + patch_addr, &rel, 4);

        printf("Patched %s call at 0x%x -> target 0x%lx (rel = %d)\n",
               func_name, patch_addr, target_addr, rel);
    }

    return LANG_SUCCESS;
}