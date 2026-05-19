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
#include "ir.h"
#include "fixup_table.h"
#include "elf_builder.h"
#include "buffer.h"
#include "read_name_table_utils.h"
#include "encode_utils.h"
#include "color_print.h"

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t backend_lang_ctx_ctor (lang_ctx_t* ctx,
                                            int         argc,
                                            char*       argv[]);

static lang_status_t backend_lang_ctx_dtor (lang_ctx_t* ctx);

static lang_status_t compile    (lang_ctx_t* ctx);
static lang_status_t link_splobj_to_elf(lang_ctx_t* ctx);
static lang_status_t append_entry_call(buffer_t* entry, uint32_t target_offset, uint32_t call_offset);
static lang_status_t make_ir    (lang_ctx_t* ctx);
static lang_status_t make_asm   (lang_ctx_t* ctx);
static lang_status_t make_binary(lang_ctx_t* ctx);
static lang_status_t fixup_global_data(lang_ctx_t* ctx);
static lang_status_t fixup_strings    (lang_ctx_t* ctx);
static lang_status_t fail_on_unresolved_symbols(lang_ctx_t* ctx);
static lang_status_t write_splobj     (lang_ctx_t* ctx, const char* file_name);

extern lang_status_t optimize_ir(lang_ctx_t* ctx);

//——————————————————————————————————————————————————————————————————————————————

int main(int argc, char* argv[])
{
    lang_ctx_t ctx = {};

    node_allocator_t node_allocator;
    ctx.node_allocator = &node_allocator;

    VERIFY(backend_lang_ctx_ctor(&ctx, argc, argv),
           return EXIT_FAILURE);

    if (ctx.ap_ctx.emit_obj && ctx.ap_ctx.link_obj) {
        fprintf(stderr, "backend: --emit-obj and --link-obj are mutually exclusive\n");
        backend_lang_ctx_dtor(&ctx);
        return EXIT_FAILURE;
    }

    if (ctx.ap_ctx.link_obj) {
        VERIFY(link_splobj_to_elf(&ctx),
               backend_lang_ctx_dtor(&ctx);
               return EXIT_FAILURE);
    } else {
        VERIFY(read_input_ctx(&ctx),
               backend_lang_ctx_dtor(&ctx);
               return EXIT_FAILURE);

        VERIFY(compile(&ctx),
               backend_lang_ctx_dtor(&ctx);
               return EXIT_FAILURE);
    }

    VERIFY(backend_lang_ctx_dtor(&ctx),
           return EXIT_FAILURE);

    fprintf(stderr, _PURPLE("backend:   ") _GREEN("success\n"));

    return EXIT_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t compile(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    VERIFY(make_ir(ctx),     return LANG_ERROR);
    VERIFY(optimize_ir(ctx), return LANG_ERROR);

    if (ctx->ap_ctx.dump_source) {
        VERIFY(make_asm(ctx), return LANG_ERROR);
    }

    VERIFY(make_binary(ctx), return LANG_ERROR);

    if (!ctx->ap_ctx.emit_obj) {
        VERIFY(create_elf_file(ctx->ap_ctx.output_file,
                               ctx->bin_buf.data,
                               ctx->bin_buf.size),
               return LANG_ERROR);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t make_ir(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    buf_ctor(&ctx->ir_buf, IR_BUFFER_INIT_CAPACITY);
    build_ir(ctx);
    ir_buffer_graph_dump(ctx, (ir_instr_t*) &ctx->ir_buf.data);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t make_asm(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    ctx->output_file = fopen(ctx->ap_ctx.source_name, "wb");
    VERIFY(!ctx->output_file, return LANG_FILE_OPEN_ERROR);

    ir_to_asm(ctx);

    fclose(ctx->output_file);
    ctx->output_file = nullptr;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t make_binary(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    buf_ctor(&ctx->bin_buf, BIN_BUF_INIT_CAPACITY);

    label_table_ctor    (&ctx->label_table,     TABLE_INIT_CAPACITY);
    fixup_table_ctor    (&ctx->fixups,          TABLE_INIT_CAPACITY);
    fixup_table_ctor    (&ctx->external_fixups, TABLE_INIT_CAPACITY);
    fixup_table_ctor    (&ctx->global_data_fixups, TABLE_INIT_CAPACITY);
    fixup_table_ctor    (&ctx->string_fixups,   TABLE_INIT_CAPACITY);

    ir_to_binary(ctx);

    buf_dtor(&ctx->ir_buf);

    fixup_global_data(ctx);
    fixup_strings(ctx);

    if (ctx->ap_ctx.emit_obj) {
        VERIFY(write_splobj(ctx, ctx->ap_ctx.output_file),
               return LANG_ERROR);
    } else {
        VERIFY(fail_on_unresolved_symbols(ctx),
               return LANG_ERROR);
    }

    label_table_dtor(&ctx->label_table);
    fixup_table_dtor(&ctx->fixups);
    fixup_table_dtor(&ctx->global_data_fixups);
    fixup_table_dtor(&ctx->string_fixups);
    fixup_table_dtor(&ctx->external_fixups);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t write_u32(FILE* fp, uint32_t value)
{
    return fwrite(&value, sizeof(value), 1, fp) == 1 ? LANG_SUCCESS : LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————

struct spl_symbol_view_t
{
    const char* name;
    uint32_t    offset;
};

struct spl_reloc_view_t
{
    const char* name;
    uint32_t    offset;
    uint32_t    rel_base;
};

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t read_spl_u32(const uint8_t** cur, const uint8_t* end, uint32_t* value)
{
    ASSERT(cur);
    ASSERT(*cur);
    ASSERT(value);

    if ((size_t) (end - *cur) < sizeof(*value)) {
        return LANG_ERROR;
    }

    memcpy(value, *cur, sizeof(*value));
    *cur += sizeof(*value);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t read_spl_cstr(const uint8_t** cur, const uint8_t* end, const char** str)
{
    ASSERT(cur);
    ASSERT(*cur);
    ASSERT(str);

    const uint8_t* begin = *cur;
    while (*cur < end && **cur != '\0') {
        (*cur)++;
    }

    if (*cur == end) {
        return LANG_ERROR;
    }

    *str = (const char*) begin;
    (*cur)++;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static spl_symbol_view_t* find_spl_symbol(spl_symbol_view_t* symbols,
                                          size_t             n_symbols,
                                          const char*        name)
{
    ASSERT(symbols || n_symbols == 0);
    ASSERT(name);

    for (size_t i = 0; i < n_symbols; i++) {
        if (strcmp(symbols[i].name, name) == 0) {
            return &symbols[i];
        }
    }

    return nullptr;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t link_splobj_to_elf(lang_ctx_t* ctx)
{
    ASSERT(ctx);
    ASSERT(ctx->code);

    const uint8_t* cur = (const uint8_t*) ctx->code;
    const uint8_t* end = cur + ctx->input_size;

    if ((size_t) (end - cur) < 4 || memcmp(cur, "SPLO", 4) != 0) {
        fprintf(stderr, "backend: %s is not a SPLO object\n", ctx->ap_ctx.input_file);
        return LANG_ERROR;
    }
    cur += 4;

    uint32_t n_symbols = 0;
    uint32_t n_relocs = 0;
    uint32_t code_size = 0;

    VERIFY(read_spl_u32(&cur, end, &n_symbols), return LANG_ERROR);
    VERIFY(read_spl_u32(&cur, end, &n_relocs),  return LANG_ERROR);
    VERIFY(read_spl_u32(&cur, end, &code_size), return LANG_ERROR);

    spl_symbol_view_t* symbols = (spl_symbol_view_t*) calloc(n_symbols, sizeof(spl_symbol_view_t));
    spl_reloc_view_t*  relocs  = (spl_reloc_view_t*)  calloc(n_relocs,  sizeof(spl_reloc_view_t));
    VERIFY((!symbols && n_symbols) || (!relocs && n_relocs),
           free(symbols);
           free(relocs);
           return LANG_STD_ALLOCATE_ERROR);

    for (uint32_t i = 0; i < n_symbols; i++) {
        VERIFY(read_spl_cstr(&cur, end, &symbols[i].name), free(symbols); free(relocs); return LANG_ERROR);
        VERIFY(read_spl_u32(&cur, end, &symbols[i].offset), free(symbols); free(relocs); return LANG_ERROR);
    }

    for (uint32_t i = 0; i < n_relocs; i++) {
        VERIFY(read_spl_cstr(&cur, end, &relocs[i].name), free(symbols); free(relocs); return LANG_ERROR);
        VERIFY(read_spl_u32(&cur, end, &relocs[i].offset), free(symbols); free(relocs); return LANG_ERROR);
        VERIFY(read_spl_u32(&cur, end, &relocs[i].rel_base), free(symbols); free(relocs); return LANG_ERROR);
    }

    if ((size_t) (end - cur) != code_size) {
        fprintf(stderr, "backend: %s has invalid SPLO code size\n", ctx->ap_ctx.input_file);
        free(symbols);
        free(relocs);
        return LANG_ERROR;
    }

    spl_symbol_view_t* main_symbol = find_spl_symbol(symbols, n_symbols, "main");
    if (!main_symbol) {
        fprintf(stderr, "Undefined symbol: main\n");
        free(symbols);
        free(relocs);
        return LANG_ERROR;
    }

    for (uint32_t i = 0; i < n_relocs; i++) {
        spl_reloc_view_t* reloc = &relocs[i];
        fprintf(stderr, "Undefined symbol: %s\n", reloc->name);
    }

    if (n_relocs > 0) {
        free(symbols);
        free(relocs);
        return LANG_ERROR;
    }

    uint32_t n_global_init_symbols = 0;
    for (uint32_t i = 0; i < n_symbols; i++) {
        if (strcmp(symbols[i].name, "__global_init") == 0) {
            n_global_init_symbols++;
        }
    }

    buffer_t final_code = {};
    buf_ctor(&final_code, code_size + 32);

    const uint32_t call_size = 5;
    const uint32_t mov_rdi_rax_size = 3;
    const uint32_t mov_rax_60_size = 7;
    const uint32_t syscall_size = 2;

    uint32_t entry_size = call_size + mov_rdi_rax_size + mov_rax_60_size + syscall_size;
    entry_size += n_global_init_symbols * call_size;

    for (uint32_t i = 0; i < n_symbols; i++) {
        if (strcmp(symbols[i].name, "__global_init") != 0) {
            continue;
        }

        VERIFY(append_entry_call(&final_code, entry_size + symbols[i].offset, (uint32_t) final_code.size),
               buf_dtor(&final_code);
               free(symbols);
               free(relocs);
               return LANG_ERROR);
    }

    VERIFY(append_entry_call(&final_code, entry_size + main_symbol->offset, (uint32_t) final_code.size),
           buf_dtor(&final_code);
           free(symbols);
           free(relocs);
           return LANG_ERROR);

    const uint8_t exit_stub[] = {
        0x48, 0x89, 0xc7,                         // mov rdi, rax
        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60
        0x0f, 0x05                                // syscall
    };

    buf_write(&final_code, exit_stub, sizeof(exit_stub));
    buf_write(&final_code, cur, code_size);

    lang_status_t status = create_elf_file(ctx->ap_ctx.output_file, final_code.data, final_code.size);

    buf_dtor(&final_code);

    free(symbols);
    free(relocs);

    return status;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t append_entry_call(buffer_t* entry, uint32_t target_offset, uint32_t call_offset)
{
    ASSERT(entry);

    const uint8_t call_opcode = 0xe8;
    const uint32_t rel_base = call_offset + 5;
    const int32_t rel = (int32_t) target_offset - (int32_t) rel_base;

    buf_write(entry, &call_opcode, sizeof(call_opcode));
    buf_write(entry, &rel, sizeof(rel));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t write_cstr(FILE* fp, const char* str)
{
    ASSERT(fp);
    ASSERT(str);

    size_t len = strlen(str) + 1;
    return fwrite(str, sizeof(char), len, fp) == len ? LANG_SUCCESS : LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t write_splobj(lang_ctx_t* ctx, const char* file_name)
{
    ASSERT(ctx);
    ASSERT(file_name);

    FILE* fp = fopen(file_name, "wb");
    VERIFY(!fp, return LANG_FILE_OPEN_ERROR);

    uint32_t n_symbols = 0;
    for (size_t i = 0; i < ctx->label_table.size; i++) {
        if (ctx->label_table.labels[i].is_global) {
            n_symbols++;
        }
    }

    uint32_t n_relocs = (uint32_t) ctx->external_fixups.size;
    uint32_t code_size = (uint32_t) ctx->bin_buf.size;

    VERIFY(fwrite("SPLO", sizeof(char), 4, fp) != 4, fclose(fp); return LANG_ERROR);
    VERIFY(write_u32(fp, n_symbols), fclose(fp); return LANG_ERROR);
    VERIFY(write_u32(fp, n_relocs), fclose(fp); return LANG_ERROR);
    VERIFY(write_u32(fp, code_size), fclose(fp); return LANG_ERROR);

    for (size_t i = 0; i < ctx->label_table.size; i++) {
        label_t* label = &ctx->label_table.labels[i];
        if (!label->is_global) {
            continue;
        }

        VERIFY(write_cstr(fp, label->value.global_name), fclose(fp); return LANG_ERROR);
        VERIFY(write_u32(fp, (uint32_t) label->address), fclose(fp); return LANG_ERROR);
    }

    for (size_t i = 0; i < ctx->external_fixups.size; i++) {
        fixup_entry_t* entry = &ctx->external_fixups.entries[i];
        VERIFY(write_cstr(fp, entry->label.value.global_name), fclose(fp); return LANG_ERROR);
        VERIFY(write_u32(fp, entry->offset), fclose(fp); return LANG_ERROR);
        VERIFY(write_u32(fp, entry->rel_base), fclose(fp); return LANG_ERROR);
    }

    VERIFY(fwrite(ctx->bin_buf.data, sizeof(uint8_t), ctx->bin_buf.size, fp) != ctx->bin_buf.size,
           fclose(fp);
           return LANG_ERROR);

    fclose(fp);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t fail_on_unresolved_symbols(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    if (ctx->external_fixups.size == 0) {
        return LANG_SUCCESS;
    }

    for (size_t i = 0; i < ctx->external_fixups.size; i++) {
        fixup_entry_t* entry = &ctx->external_fixups.entries[i];
        fprintf(stderr, "Undefined symbol: %s\n", entry->label.value.global_name);
    }

    return LANG_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t backend_lang_ctx_ctor(lang_ctx_t* ctx, int argc, char* argv[])
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

    if (ctx->input_size >= 4 && memcmp(ctx->code, "SPLO", 4) == 0) {
        ctx->ap_ctx.link_obj = true;
    }

    if (ctx->ap_ctx.link_obj) {
        VERIFY(node_allocator_ctor(ctx->node_allocator, N_NODES_INIT),
               return LANG_NODE_ALLOCATOR_CTOR_ERROR);

        return LANG_SUCCESS;
    }

    ctx->nodes = (node_t**) calloc(ctx->input_size + 1, sizeof(node_t*));
    VERIFY(!ctx->nodes,
           return LANG_STD_ALLOCATE_ERROR);
    ctx->n_nodes = 0;
    ctx->pos = 0;

    ctx->name_table.ids = (identifier_t*) calloc(ctx->input_size, sizeof(identifier_t));
    VERIFY(!ctx->name_table.ids, return LANG_STD_ALLOCATE_ERROR);

    ctx->name_table.names = (name_t*) calloc(ctx->input_size,  sizeof(identifier_t));
    VERIFY(!ctx->name_table.names, return LANG_STD_ALLOCATE_ERROR);

    ctx->name_table.n_names = 0;
    ctx->global_data_size = 0;
    ctx->cur_stack_frame_size = 0;
    ctx->emitting_global_init = false;
    ctx->level = 0;

    VERIFY(node_allocator_ctor(ctx->node_allocator, N_NODES_INIT),
           return LANG_NODE_ALLOCATOR_CTOR_ERROR);

    return LANG_SUCCESS;

}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t fixup_global_data(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    size_t global_data_base = ctx->bin_buf.size;
    if (ctx->global_data_size > 0) {
        uint8_t* zeros = (uint8_t*) calloc(ctx->global_data_size, sizeof(uint8_t));
        VERIFY(!zeros, return LANG_STD_ALLOCATE_ERROR);

        buf_write(&ctx->bin_buf, zeros, ctx->global_data_size);
        free(zeros);
    }

    for (size_t i = 0; i < ctx->global_data_fixups.size; i++) {
        fixup_entry_t* entry = &ctx->global_data_fixups.entries[i];
        uint32_t target_addr = (uint32_t) (global_data_base + entry->label.value.local_number);
        int32_t rel = (int32_t) target_addr - (int32_t) entry->rel_base;

        memcpy(ctx->bin_buf.data + entry->offset, &rel, sizeof(rel));
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t fixup_strings(lang_ctx_t* ctx)
{
    ASSERT(ctx);

    for (size_t i = 0; i < ctx->string_fixups.size; i++) {
        fixup_entry_t* entry = &ctx->string_fixups.entries[i];
        const char* str = entry->label.value.global_name;
        uint32_t target_addr = (uint32_t) ctx->bin_buf.size;
        int32_t rel = (int32_t) target_addr - (int32_t) entry->rel_base;

        memcpy(ctx->bin_buf.data + entry->offset, &rel, sizeof(rel));
        buf_write(&ctx->bin_buf, str, strlen(str) + 1);
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t backend_lang_ctx_dtor(lang_ctx_t* ctx)
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
