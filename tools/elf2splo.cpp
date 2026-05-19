#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
    uint32_t offset;
} symbol_t;

typedef struct {
    const char* name;
    uint32_t offset;
    uint32_t rel_base;
} reloc_t;

static uint64_t align_up_u64(uint64_t value, uint64_t align)
{
    if (align <= 1) return value;
    return (value + align - 1) & ~(align - 1);
}

static int starts_with(const char* str, const char* prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static int write_u32(FILE* fp, uint32_t value)
{
    return fwrite(&value, sizeof(value), 1, fp) == 1 ? 0 : -1;
}

static uint8_t* read_file(const char* path, size_t* out_size)
{
    FILE* fp = fopen(path, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t* data = (uint8_t*) malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return NULL;
    }

    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    *out_size = (size_t)size;
    return data;
}

static const Elf64_Shdr* find_section_by_name(const uint8_t* data,
                                              const Elf64_Ehdr* ehdr,
                                              const char* name)
{
    const Elf64_Shdr* shdrs = (const Elf64_Shdr*)(data + ehdr->e_shoff);
    const Elf64_Shdr* shstr = &shdrs[ehdr->e_shstrndx];
    const char* names = (const char*)(data + shstr->sh_offset);

    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        const char* cur_name = names + shdrs[i].sh_name;
        if (strcmp(cur_name, name) == 0) {
            return &shdrs[i];
        }
    }

    return NULL;
}

static uint16_t find_section_index_by_name(const uint8_t* data,
                                           const Elf64_Ehdr* ehdr,
                                           const char* name)
{
    const Elf64_Shdr* shdrs = (const Elf64_Shdr*)(data + ehdr->e_shoff);
    const Elf64_Shdr* shstr = &shdrs[ehdr->e_shstrndx];
    const char* names = (const char*)(data + shstr->sh_offset);

    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        const char* cur_name = names + shdrs[i].sh_name;
        if (strcmp(cur_name, name) == 0) {
            return i;
        }
    }

    return SHN_UNDEF;
}

static const Elf64_Shdr* find_symtab(const uint8_t* data, const Elf64_Ehdr* ehdr)
{
    (void)data;
    const Elf64_Shdr* shdrs = (const Elf64_Shdr*)(data + ehdr->e_shoff);
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            return &shdrs[i];
        }
    }
    return NULL;
}

static int write_splobj(const char* path,
                        const symbol_t* symbols,
                        size_t n_symbols,
                        const reloc_t* relocs,
                        size_t n_relocs,
                        const uint8_t* code,
                        size_t code_size)
{
    FILE* fp = fopen(path, "wb");
    if (!fp) return -1;

    if (fwrite("SPLO", 1, 4, fp) != 4 ||
        write_u32(fp, (uint32_t)n_symbols) != 0 ||
        write_u32(fp, (uint32_t)n_relocs) != 0 ||
        write_u32(fp, (uint32_t)code_size) != 0) {
        fclose(fp);
        return -1;
    }

    for (size_t i = 0; i < n_symbols; ++i) {
        size_t name_len = strlen(symbols[i].name) + 1;
        if (fwrite(symbols[i].name, 1, name_len, fp) != name_len ||
            write_u32(fp, symbols[i].offset) != 0) {
            fclose(fp);
            return -1;
        }
    }

    for (size_t i = 0; i < n_relocs; ++i) {
        size_t name_len = strlen(relocs[i].name) + 1;
        if (fwrite(relocs[i].name, 1, name_len, fp) != name_len ||
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

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.elf> <output.splobj>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t file_size = 0;
    uint8_t* data = read_file(argv[1], &file_size);
    if (!data || file_size < sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "spl_elf2splo: cannot read %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)data;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS64 ||
        ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "spl_elf2splo: unsupported ELF format\n");
        free(data);
        return EXIT_FAILURE;
    }

    const Elf64_Shdr* symtab = find_symtab(data, ehdr);
    if (!find_section_by_name(data, ehdr, ".text") || !symtab) {
        fprintf(stderr, "spl_elf2splo: .text or .symtab not found\n");
        free(data);
        return EXIT_FAILURE;
    }

    const Elf64_Shdr* shdrs = (const Elf64_Shdr*)(data + ehdr->e_shoff);
    const Elf64_Shdr* strtab = &shdrs[symtab->sh_link];
    const char* strings = (const char*)(data + strtab->sh_offset);
    const Elf64_Sym* syms = (const Elf64_Sym*)(data + symtab->sh_offset);
    size_t n_elf_symbols = symtab->sh_size / sizeof(Elf64_Sym);

    const Elf64_Shdr* shstr = &shdrs[ehdr->e_shstrndx];
    const char* section_names = (const char*)(data + shstr->sh_offset);
    uint32_t* section_bases = (uint32_t*) calloc(ehdr->e_shnum, sizeof(uint32_t));
    uint8_t* section_used = (uint8_t*) calloc(ehdr->e_shnum, sizeof(uint8_t));
    if (!section_bases || !section_used) {
        free(section_bases);
        free(section_used);
        free(data);
        return EXIT_FAILURE;
    }

    uint64_t code_size64 = 0;
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        const char* name = section_names + shdrs[i].sh_name;
        int is_text = shdrs[i].sh_type == SHT_PROGBITS &&
                      (shdrs[i].sh_flags & SHF_ALLOC) &&
                      (shdrs[i].sh_flags & SHF_EXECINSTR) &&
                      starts_with(name, ".text");
        if (!is_text) {
            continue;
        }

        code_size64 = align_up_u64(code_size64, shdrs[i].sh_addralign);
        section_bases[i] = (uint32_t)code_size64;
        section_used[i] = 1;
        code_size64 += shdrs[i].sh_size;
    }

    uint8_t* code = (uint8_t*) calloc((size_t)code_size64, sizeof(uint8_t));
    if (!code && code_size64) {
        free(section_bases);
        free(section_used);
        free(data);
        return EXIT_FAILURE;
    }

    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        if (section_used[i]) {
            memcpy(code + section_bases[i], data + shdrs[i].sh_offset, shdrs[i].sh_size);
        }
    }

    symbol_t* symbols = (symbol_t*) calloc(n_elf_symbols, sizeof(symbol_t));
    if (!symbols) {
        free(code);
        free(section_bases);
        free(section_used);
        free(data);
        return EXIT_FAILURE;
    }

    size_t n_symbols = 0;
    for (size_t i = 0; i < n_elf_symbols; ++i) {
        unsigned type = ELF64_ST_TYPE(syms[i].st_info);
        if ((type != STT_FUNC && type != STT_NOTYPE) || syms[i].st_name == 0) {
            continue;
        }

        if (syms[i].st_shndx < ehdr->e_shnum && section_used[syms[i].st_shndx]) {
            const Elf64_Shdr* sym_section = &shdrs[syms[i].st_shndx];
            symbols[n_symbols++] = (symbol_t) {
                .name = strings + syms[i].st_name,
                .offset = section_bases[syms[i].st_shndx] +
                          (uint32_t)(syms[i].st_value - sym_section->sh_addr),
            };
        }
    }

    size_t max_relocs = 0;
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        if (shdrs[i].sh_type == SHT_RELA &&
            shdrs[i].sh_info < ehdr->e_shnum &&
            section_used[shdrs[i].sh_info]) {
            max_relocs += shdrs[i].sh_size / sizeof(Elf64_Rela);
        }
    }

    reloc_t* relocs = (reloc_t*) calloc(max_relocs, sizeof(reloc_t));
    if (!relocs && max_relocs) {
        free(code);
        free(section_bases);
        free(section_used);
        free(symbols);
        free(data);
        return EXIT_FAILURE;
    }

    size_t n_relocs = 0;
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        if (shdrs[i].sh_type != SHT_RELA ||
            shdrs[i].sh_info >= ehdr->e_shnum ||
            !section_used[shdrs[i].sh_info]) {
            continue;
        }

        uint32_t target_section_base = section_bases[shdrs[i].sh_info];
        const Elf64_Rela* rela = (const Elf64_Rela*)(data + shdrs[i].sh_offset);
        size_t n_rela = shdrs[i].sh_size / sizeof(Elf64_Rela);
        for (size_t j = 0; j < n_rela; ++j) {
            uint32_t sym_idx = (uint32_t)ELF64_R_SYM(rela[j].r_info);
            uint32_t type = (uint32_t)ELF64_R_TYPE(rela[j].r_info);
            if (sym_idx >= n_elf_symbols) {
                fprintf(stderr, "spl_elf2splo: bad relocation symbol index\n");
                free(relocs);
                free(symbols);
                free(data);
                return EXIT_FAILURE;
            }

            if (type != R_X86_64_PC32 && type != R_X86_64_PLT32) {
                fprintf(stderr, "spl_elf2splo: unsupported .text relocation type %u\n", type);
                free(relocs);
                free(code);
                free(section_bases);
                free(section_used);
                free(symbols);
                free(data);
                return EXIT_FAILURE;
            }

            uint32_t patch_offset = target_section_base + (uint32_t)rela[j].r_offset;
            uint32_t rel_base = (uint32_t)((int64_t)patch_offset - rela[j].r_addend);
            if (ELF64_ST_TYPE(syms[sym_idx].st_info) == STT_SECTION &&
                syms[sym_idx].st_shndx < ehdr->e_shnum &&
                section_used[syms[sym_idx].st_shndx]) {
                uint32_t target = section_bases[syms[sym_idx].st_shndx] +
                                  (uint32_t)syms[sym_idx].st_value;
                int32_t rel = (int32_t)target - (int32_t)rel_base;
                memcpy(code + patch_offset, &rel, sizeof(rel));
                continue;
            }

            const char* name = strings + syms[sym_idx].st_name;
            if (!name[0]) {
                fprintf(stderr, "spl_elf2splo: unsupported anonymous relocation symbol\n");
                free(relocs);
                free(code);
                free(section_bases);
                free(section_used);
                free(symbols);
                free(data);
                return EXIT_FAILURE;
            }

            relocs[n_relocs++] = (reloc_t) {
                .name = name,
                .offset = patch_offset,
                .rel_base = rel_base,
            };
        }
    }

    int rc = write_splobj(argv[2], symbols, n_symbols, relocs, n_relocs, code, (size_t)code_size64);
    if (rc != 0) {
        fprintf(stderr, "spl_elf2splo: cannot write %s\n", argv[2]);
    }

    free(relocs);
    free(symbols);
    free(code);
    free(section_bases);
    free(section_used);
    free(data);
    return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
