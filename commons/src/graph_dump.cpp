#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "graph_dump.h"
#include "node_allocator.h"
#include "custom_assert.h"
#include "ir.h"

//——————————————————————————————————————————————————————————————————————————————

static tree_dump_status_t dot_file_init (FILE*       dot_file);

static tree_dump_status_t make_edges    (lang_ctx_t*     ctx,
                                         node_t*         node,
                                         FILE*           file);

static tree_dump_status_t make_elem     (lang_ctx_t*     ctx,
                                         node_t*         node,
                                         FILE*           file);

static tree_dump_status_t create_png    (const char* dot_file_name,
                                         const char* png_file_name);

//——————————————————————————————————————————————————————————————————————————————

void make_dot_elem_str(FILE*       file,
                       void*       elem_number,
                       const char* label,
                       const char* str)
{
    ASSERT(file);
    ASSERT(label);
    ASSERT(str);

    fprintf(file, "elem%p["
                  "shape=\"Mrecord\", "
                  "label= \"{%s | val = %s}\""
                  "];\n",
                  elem_number,
                  label,
                  str);
}

//——————————————————————————————————————————————————————————————————————————————

void make_dot_elem_number(FILE*       file,
                          void*       elem_number,
                          const char* label,
                          int64_t     number)
{
    ASSERT(file);
    ASSERT(label);

    fprintf(file, "elem%p["
                  "shape=\"Mrecord\", "
                  "label= \"{%s | val = %ld}\""
                  "];\n",
                  elem_number,
                  label,
                  number);
}

//——————————————————————————————————————————————————————————————————————————————

void make_dot_elems_connection(FILE* file, void* number1, void* number2)
{
    ASSERT(file);

   fprintf(file, "elem%p->elem%p;", number1, number2);
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_dot_ir_opd(FILE* file, ir_opd_t* opd)
{
    ASSERT(file);
    ASSERT(opd);

    switch(opd->type) {
        case IR_OPD_NAN: {
            return TREE_DUMP_EMPTY;
        }
        case IR_OPD_REGISTER: {
            make_dot_elem_str   (file, opd, "REGISTER",
                                 RegNames[opd->value.reg]);
            break;
        }
        case IR_OPD_MEMORY: {
            make_dot_elem_number(file, opd, "MEMORY",
                                 opd->value.offset);
            break;
        }
        case IR_OPD_IMMEDIATE: {
            make_dot_elem_number(file, opd, "IMMEDIATE",
                                 opd->value.imm);
            break;
        }
        case IR_OPD_GLOBAL_LABEL: {
            make_dot_elem_str   (file, opd, "GLOBAL LABEL",
                                 opd->value.global_label_name);
            break;
        }
        case IR_OPD_LOCAL_LABEL: {
            make_dot_elem_number(file, opd, "LOCAL LABEL",
                                 opd->value.local_label_number);
            break;
        }
        default: {
            return TREE_DUMP_EMPTY;
        }
    }

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_ir_elem(lang_ctx_t* ctx, ir_instr_t* instr, FILE* file)
{
    ASSERT(ctx);
    ASSERT(instr);
    ASSERT(file);

    make_dot_elem_str(file, instr, "OPCODE", IrOpAsmNamesTable[instr->opc]);

    if (make_dot_ir_opd(file, &instr->opd1) == TREE_DUMP_SUCCESS) {
        make_dot_elems_connection(file, &instr->opc, &instr->opd1);
    }

    if (make_dot_ir_opd(file, &instr->opd2) == TREE_DUMP_SUCCESS) {
        make_dot_elems_connection(file, &instr->opc, &instr->opd2);
    }

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t ir_buffer_graph_dump(lang_ctx_t* ctx, ir_instr_t* buffer)
{
    ASSERT(ctx);
    ASSERT(buffer);

    static size_t n_dumps = 0;

    char dot_file_name[FileNameBufSize] = {};
    snprintf(dot_file_name, FileNameBufSize,
             LOGS_DIR "/" DOTS_DIR "/" "ir_%03ld.dot",
             n_dumps);

    FILE* dot_file = fopen(dot_file_name, "w");
    VERIFY(!dot_file, return TREE_DUMP_FILE_OPEN_ERROR);

    dot_file_init(dot_file);

    size_t buffer_size = ctx->ir_buf.size / sizeof(ir_instr_t);

    ir_instr_t* ir_buf = (ir_instr_t*) ctx->ir_buf.data;

    for (size_t i = 0; i < buffer_size; i++) {
        make_ir_elem(ctx, &ir_buf[i], dot_file);
    }

    fputs("}\n", dot_file);
    fclose(dot_file);

    char png_file_name[FileNameBufSize] = {};
    snprintf(png_file_name, FileNameBufSize,
             LOGS_DIR "/" IMGS_DIR "/" "ir_%03ld.png",
             n_dumps);

    create_png(dot_file_name, png_file_name);

    n_dumps++;

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t graph_dump(lang_ctx_t* ctx,
                              node_t*     node,
                              dump_mode_t mode)
{
    ASSERT(ctx);

    static size_t n_dumps = 0;

    char dot_file_name[FileNameBufSize] = {};
    snprintf(dot_file_name, FileNameBufSize,
             LOGS_DIR "/" DOTS_DIR "/" "ast_%03ld.dot",
             n_dumps);

    FILE* dot_file = fopen(dot_file_name, "w");
    VERIFY(!dot_file, return TREE_DUMP_FILE_OPEN_ERROR);

    dot_file_init(dot_file);

    if (mode == TREE) {
        make_edges(ctx, node, dot_file);
    } else if (mode == ARR) {
        for (int node_ind = 0; node_ind < ctx->n_nodes; node_ind++) {
            make_elem(ctx, ctx->nodes[node_ind], dot_file);
        }
    }

    fputs("}\n", dot_file);
    fclose(dot_file);

    char png_file_name[FileNameBufSize] = {};
    snprintf(png_file_name, FileNameBufSize,
             LOGS_DIR "/" IMGS_DIR "/" "ast_%03ld.png",
             n_dumps);

    create_png(dot_file_name, png_file_name);

    n_dumps++;

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t dot_file_init(FILE* dot_file)
{
    ASSERT(dot_file);

    fprintf(dot_file, "digraph G{\n"
                      "nodesep=1;\n"
                      "ranksep=0.5;\n"
                      "rankdir=HR;\n"
                      "node[style=filled, color=\"%s\", fillcolor=\"%s\","
                      "fontcolor=\"%s\", fontsize=14];\n"
                      "edge[color=\"%s\", fontsize=12, penwidth=1, "
                      "fontcolor = \"%s\"];\n"
                      "bgcolor=\"%s\";\n",
                      NodeBorderColor, NodeBackGroundColor, NodeFontColor,
                      EdgeColor, EdgeFontColor,
                      BackGroundColor);

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_dot_ast_id(FILE*       file,
                                   void*       elem_number,
                                   const char* type,
                                   const char* label,
                                   const char* name,
                                   int64_t     number)
{
    fprintf(file, "elem%p["
                   "shape=\"Mrecord\", "
                   "label= \"{%s | type = %s | name = %s | operator_code = %ld}\""
                   "];\n",
                   elem_number,
                   type,
                   label,
                   name,
                   number);

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_dot_ast_operator(FILE*       file,
                                         void*       elem_number,
                                         const char* label,
                                         const char* name,
                                         int64_t     number)
{
    fprintf(file, "elem%p["
                   "shape=\"Mrecord\", "
                   "label= \"{%s | name = %s | operator_code = %ld}\""
                   "];\n",
                   elem_number,
                   label,
                   name,
                   number);

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_elem(lang_ctx_t* ctx, node_t* node, FILE* file)
{
    ASSERT(ctx);
    ASSERT(node);
    ASSERT(file);

    switch(node->value_type) {
        case NUMBER: {
            make_dot_elem_number(file, node, "NUMBER", node->value.number);
            break;
        }
        case IDENTIFIER: {
            const char* id_type = "UNKNOWN";
            identifier_t node_id = ctx->name_table.ids[node->value.id_index];

            if (node_id.type == VAR) {
                id_type = "VAR";
            }
            else if (node_id.type == FUNC_DEF) {
                id_type = "FUNC_DEF";
            }

            make_dot_ast_id(file, node,
                           "IDENTIFIER",
                           id_type,
                           ctx->name_table.ids[node->value.id_index].name,
                           node->value.id_index);
            break;
        }
        case OPERATOR: {
            const char* name = OperatorsTable[node->value.operator_code].name;

            make_dot_ast_operator(file, node, "OPERATOR", name, node->value.operator_code);
            break;
        }
        default: {
            break;
        }
    }

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t make_edges(lang_ctx_t* ctx, node_t* node, FILE* file)
{
    ASSERT(node);
    ASSERT(file);

    make_elem(ctx, node, file);

    if (node->left) {
        make_dot_elems_connection(file, node, node->left);
        make_edges(ctx, node->left, file);
    }

    if (node->right) {
        make_dot_elems_connection(file, node, node->right);
        make_edges(ctx, node->right, file);
    }

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

tree_dump_status_t create_png(const char* dot_file_name,
                              const char* png_file_name)
{
    ASSERT(dot_file_name);
    ASSERT(png_file_name);

    char command[SysCommandBufSize] = {};
    snprintf(command, SysCommandBufSize, "touch %s; dot %s -Tpng -o %s",
             png_file_name, dot_file_name, png_file_name);

    VERIFY(system(command),
           return TREE_DUMP_SYSTEM_COMMAND_ERROR);

    return TREE_DUMP_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
