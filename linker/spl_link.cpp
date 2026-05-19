#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    char* name;
    uint32_t offset;
} symbol_t;

typedef struct {
    char* name;
    uint32_t offset;
    uint32_t rel_base;
} reloc_t;

typedef struct {
    const char* path;
    symbol_t* symbols;
    size_t n_symbols;
    reloc_t* relocs;
    size_t n_relocs;
    uint8_t* code;
    size_t code_size;
    size_t base;
} obj_t;

typedef struct {
    char* name;
    size_t addr;
    const char* path;
} global_symbol_t;

static int read_u32(FILE* fp, uint32_t* value)
{
    return fread(value, sizeof(*value), 1, fp) == 1 ? 0 : -1;
}

static int write_u32(FILE* fp, uint32_t value)
{
    return fwrite(&value, sizeof(value), 1, fp) == 1 ? 0 : -1;
}

static char* read_cstr(FILE* fp)
{
    size_t cap = 32;
    size_t size = 0;
    char* str = (char*) calloc(cap, sizeof(char));
    if (!str) return NULL;

    for (;;) {
        int ch = fgetc(fp);
        if (ch == EOF) {
            free(str);
            return NULL;
        }

        if (size + 1 >= cap) {
            cap *= 2;
            char* new_str = (char*) realloc(str, cap);
            if (!new_str) {
                free(str);
                return NULL;
            }
            str = new_str;
        }

        str[size++] = (char) ch;
        if (ch == '\0') {
            return str;
        }
    }
}

static void free_obj(obj_t* obj)
{
    if (!obj) return;

    for (size_t i = 0; i < obj->n_symbols; ++i) free(obj->symbols[i].name);
    for (size_t i = 0; i < obj->n_relocs; ++i) free(obj->relocs[i].name);
    free(obj->symbols);
    free(obj->relocs);
    free(obj->code);
}

static int read_splobj(const char* path, obj_t* obj)
{
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "spl_link: cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }

    char magic[4] = {};
    uint32_t n_symbols = 0;
    uint32_t n_relocs = 0;
    uint32_t code_size = 0;

    if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic) ||
        memcmp(magic, "SPLO", 4) != 0 ||
        read_u32(fp, &n_symbols) != 0 ||
        read_u32(fp, &n_relocs) != 0 ||
        read_u32(fp, &code_size) != 0) {
        fprintf(stderr, "spl_link: %s is not a SPLO object\n", path);
        fclose(fp);
        return -1;
    }

    obj->path = path;
    obj->n_symbols = n_symbols;
    obj->n_relocs = n_relocs;
    obj->code_size = code_size;
    obj->symbols = (symbol_t*) calloc(n_symbols, sizeof(symbol_t));
    obj->relocs = (reloc_t*) calloc(n_relocs, sizeof(reloc_t));
    obj->code = (uint8_t*) malloc(code_size);

    if ((!obj->symbols && n_symbols) || (!obj->relocs && n_relocs) || (!obj->code && code_size)) {
        fclose(fp);
        return -1;
    }

    for (uint32_t i = 0; i < n_symbols; ++i) {
        obj->symbols[i].name = read_cstr(fp);
        if (!obj->symbols[i].name || read_u32(fp, &obj->symbols[i].offset) != 0) {
            fclose(fp);
            return -1;
        }
    }

    for (uint32_t i = 0; i < n_relocs; ++i) {
        obj->relocs[i].name = read_cstr(fp);
        if (!obj->relocs[i].name ||
            read_u32(fp, &obj->relocs[i].offset) != 0 ||
            read_u32(fp, &obj->relocs[i].rel_base) != 0) {
            fclose(fp);
            return -1;
        }
    }

    if (fread(obj->code, 1, code_size, fp) != code_size) {
        fprintf(stderr, "spl_link: %s has truncated code section\n", path);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

static ssize_t find_symbol(global_symbol_t* symbols, size_t n_symbols, const char* name)
{
    for (size_t i = 0; i < n_symbols; ++i) {
        if (strcmp(symbols[i].name, name) == 0) return (ssize_t) i;
    }
    return -1;
}

static int build_global_symbols(obj_t* objs,
                                size_t n_objs,
                                global_symbol_t** out_symbols,
                                size_t* out_n_symbols,
                                size_t* out_code_size,
                                int allow_duplicates)
{
    size_t total_symbols = 0;
    size_t code_size = 0;
    for (size_t i = 0; i < n_objs; ++i) {
        objs[i].base = code_size;
        code_size += objs[i].code_size;
        total_symbols += objs[i].n_symbols;
    }

    global_symbol_t* symbols = (global_symbol_t*) calloc(total_symbols, sizeof(global_symbol_t));
    if (!symbols && total_symbols) return -1;

    size_t n_symbols = 0;
    for (size_t i = 0; i < n_objs; ++i) {
        for (size_t j = 0; j < objs[i].n_symbols; ++j) {
            symbol_t* sym = &objs[i].symbols[j];
            if (find_symbol(symbols, n_symbols, sym->name) >= 0) {
                if (strcmp(sym->name, "__global_init") == 0) {
                    symbols[n_symbols++] = (global_symbol_t) {
                        .name = sym->name,
                        .addr = objs[i].base + sym->offset,
                        .path = objs[i].path,
                    };
                    continue;
                }

                if (allow_duplicates) {
                    fprintf(stderr,
                            "spl_link: warning: duplicate symbol '%s' from %s, keeping first\n",
                            sym->name,
                            objs[i].path);
                    continue;
                } else {
                    fprintf(stderr, "spl_link: duplicate symbol '%s' from %s\n", sym->name, objs[i].path);
                    free(symbols);
                    return -1;
                }
            }
            symbols[n_symbols++] = (global_symbol_t) {
                .name = sym->name,
                .addr = objs[i].base + sym->offset,
                .path = objs[i].path,
            };
        }
    }

    *out_symbols = symbols;
    *out_n_symbols = n_symbols;
    *out_code_size = code_size;
    return 0;
}

static int write_cstr(FILE* fp, const char* str)
{
    size_t len = strlen(str) + 1;
    return fwrite(str, 1, len, fp) == len ? 0 : -1;
}

static int write_splobj(const char* path,
                        const global_symbol_t* symbols,
                        size_t n_symbols,
                        const reloc_t* relocs,
                        size_t n_relocs,
                        const uint8_t* code,
                        size_t code_size)
{
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "spl_link: cannot create %s: %s\n", path, strerror(errno));
        return -1;
    }

    if (fwrite("SPLO", 1, 4, fp) != 4 ||
        write_u32(fp, (uint32_t)n_symbols) != 0 ||
        write_u32(fp, (uint32_t)n_relocs) != 0 ||
        write_u32(fp, (uint32_t)code_size) != 0) {
        fclose(fp);
        return -1;
    }

    for (size_t i = 0; i < n_symbols; ++i) {
        if (write_cstr(fp, symbols[i].name) != 0 ||
            write_u32(fp, (uint32_t)symbols[i].addr) != 0) {
            fclose(fp);
            return -1;
        }
    }

    for (size_t i = 0; i < n_relocs; ++i) {
        if (write_cstr(fp, relocs[i].name) != 0 ||
            write_u32(fp, relocs[i].offset) != 0 ||
            write_u32(fp, relocs[i].rel_base) != 0) {
            fclose(fp);
            return -1;
        }
    }

    if (fwrite(code, 1, code_size, fp) != code_size) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

static int link_relocatable(obj_t* objs, size_t n_objs, const char* out_path)
{
    global_symbol_t* symbols = NULL;
    size_t n_symbols = 0;
    size_t code_size = 0;
    if (build_global_symbols(objs, n_objs, &symbols, &n_symbols, &code_size, 1) != 0) {
        return -1;
    }

    uint8_t* code = (uint8_t*) calloc(code_size, sizeof(uint8_t));
    if (!code && code_size) {
        free(symbols);
        return -1;
    }

    size_t max_relocs = 0;
    for (size_t i = 0; i < n_objs; ++i) {
        memcpy(code + objs[i].base, objs[i].code, objs[i].code_size);
        max_relocs += objs[i].n_relocs;
    }

    reloc_t* out_relocs = (reloc_t*) calloc(max_relocs, sizeof(reloc_t));
    if (!out_relocs && max_relocs) {
        free(code);
        free(symbols);
        return -1;
    }

    size_t n_out_relocs = 0;
    for (size_t i = 0; i < n_objs; ++i) {
        for (size_t j = 0; j < objs[i].n_relocs; ++j) {
            reloc_t* reloc = &objs[i].relocs[j];
            ssize_t sym_idx = find_symbol(symbols, n_symbols, reloc->name);
            size_t patch_offset = objs[i].base + reloc->offset;
            size_t rel_base = objs[i].base + reloc->rel_base;

            if (sym_idx >= 0) {
                int32_t rel = (int32_t)symbols[sym_idx].addr - (int32_t)rel_base;
                memcpy(code + patch_offset, &rel, sizeof(rel));
            } else {
                out_relocs[n_out_relocs++] = (reloc_t) {
                    .name = reloc->name,
                    .offset = (uint32_t)patch_offset,
                    .rel_base = (uint32_t)rel_base,
                };
            }
        }
    }

    int rc = write_splobj(out_path, symbols, n_symbols, out_relocs, n_out_relocs, code, code_size);

    free(out_relocs);
    free(code);
    free(symbols);
    return rc;
}

static void usage(const char* argv0)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s -r -o <out.splobj> <input1.splobj> <input2.splobj> ...\n",
            argv0);
}

int main(int argc, char** argv)
{
    const char* out_path = NULL;
    int first_input = 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
        } else {
            first_input = i;
            break;
        }
    }

    if (!out_path || first_input >= argc) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    size_t n_objs = (size_t)(argc - first_input);
    obj_t* objs = (obj_t*) calloc(n_objs, sizeof(obj_t));
    if (!objs) return EXIT_FAILURE;

    for (size_t i = 0; i < n_objs; ++i) {
        if (read_splobj(argv[first_input + (int)i], &objs[i]) != 0) {
            return EXIT_FAILURE;
        }
    }

    int rc = link_relocatable(objs, n_objs, out_path);
    for (size_t i = 0; i < n_objs; ++i) free_obj(&objs[i]);
    free(objs);

    return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
