//——————————————————————————————————————————————————————————————————————————————

#include "elf_builder.h"
#include "custom_assert.h"
#include "elf_builder_defines.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

//——————————————————————————————————————————————————————————————————————————————

static lang_status_t write_elf_hdr     (buffer_t* elf);
static lang_status_t write_elf_phdr    (buffer_t* elf, size_t code_size);
static lang_status_t write_elf_padding (buffer_t* elf);
static lang_status_t write_elf_code    (buffer_t* elf,
                                        const uint8_t*  code,
                                        size_t    code_size);

static lang_status_t write_elf_shstrtab(buffer_t* elf,
                                        buffer_t* shstrtab,
                                        uint32_t* text_name_offset,
                                        uint32_t* shstrtab_name_offset,
                                        size_t*   shstrtab_offset);

static lang_status_t write_elf_shdr    (buffer_t* elf,
                                        buffer_t* shstrtab,
                                        size_t    code_size,
                                        uint32_t  text_name_offset,
                                        uint32_t  shstrtab_name_offset,
                                        size_t    shstrtab_offset);

static lang_status_t write_elf_upd_hdr (buffer_t* elf,
                                        size_t    shoff);

//——————————————————————————————————————————————————————————————————————————————

/* Main goal of this function is to build:
   Start of ELF
   ├── [ELF Header]
   ├── [Program Header Table]
   │             └── [Executable segment program header]
   ├── [Padding до 0x1000]
   ├── [Executable Code]
   ├── [.shstrtab]
   |             └── "\0.text\0.shstrtab"
   └── [Section Header Table]
                 ├── [.text section]
                 └── [.shstrtab section]
   */

#define N_PHDRS 1
#define N_SHDRS 2

lang_status_t build_elf(buffer_t* elf, const uint8_t* code, size_t code_size)
{
    ASSERT(elf);
    ASSERT(code);

    write_elf_hdr    (elf);
    write_elf_phdr   (elf, code_size);
    write_elf_padding(elf);
    write_elf_code   (elf, code, code_size);

    buffer_t shstrtab = {};

    uint32_t text_name_offset     = 0;
    uint32_t shstrtab_name_offset = 0;
    size_t   shstrtab_offset      = 0;

    write_elf_shstrtab(elf,
                       &shstrtab,
                       &text_name_offset,
                       &shstrtab_name_offset,
                       &shstrtab_offset);

    size_t shoff = elf->size;

    write_elf_shdr(elf,
                   &shstrtab,
                   code_size,
                   text_name_offset,
                   shstrtab_name_offset,
                   shstrtab_offset);

    write_elf_upd_hdr (elf, shoff);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t create_elf_file(const char* file_name, const uint8_t* code, size_t code_size)
{
    ASSERT(file_name);
    ASSERT(code);

    buffer_t elf = {};
    buf_ctor(&elf, 10000);

    build_elf(&elf, code, code_size);

    FILE* fp = fopen(file_name, "wb");
    VERIFY(!fp, return LANG_ERROR);

    fwrite(elf.data, sizeof(uint8_t), elf.size, fp);

    fclose(fp);
    buf_dtor(&elf);

    if (chmod(file_name, RWX_RX_RX) == -1) {
        fprintf(stderr, "chmod error\n");
        return LANG_ERROR;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_hdr(buffer_t* elf)
{
    ASSERT(elf);

    Elf64_Ehdr elf_hdr = {};

    return buf_write(elf, &elf_hdr, sizeof(elf_hdr));
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_phdr(buffer_t* elf, size_t code_size)
{
    ASSERT(elf);

    Elf64_Phdr exe_phdr = {};
    build_exe_phdr(&exe_phdr, code_size);

    return buf_write(elf, &exe_phdr, sizeof(exe_phdr));
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_padding (buffer_t* elf)
{
    ASSERT(elf);

    size_t padding_size = PAGE_SIZE - sizeof(Elf64_Ehdr) - sizeof(Elf64_Phdr);
    uint8_t* padding = (uint8_t*) calloc(padding_size, sizeof(uint8_t));
    buf_write(elf, padding, padding_size);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_code(buffer_t* elf, const uint8_t* code, size_t code_size)
{
    return buf_write(elf, code, code_size);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_shstrtab(buffer_t* elf,
                                 buffer_t* shstrtab,
                                 uint32_t* text_name_offset,
                                 uint32_t* shstrtab_name_offset,
                                 size_t*   shstrtab_offset)
{
    ASSERT(elf);
    ASSERT(text_name_offset);
    ASSERT(shstrtab_name_offset);
    ASSERT(shstrtab_offset);

    shstrtab_init(shstrtab, strlen("0.text0.shstrtab0"));

    add_section_name(shstrtab, ".text",     text_name_offset);
    add_section_name(shstrtab, ".shstrtab", shstrtab_name_offset);

    *shstrtab_offset = elf->size;
    buf_write(elf, shstrtab->data, shstrtab->size);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_shdr(buffer_t* elf,
                             buffer_t* shstrtab,
                             size_t    code_size,
                             uint32_t  text_name_offset,
                             uint32_t  shstrtab_name_offset,
                             size_t    shstrtab_offset)
{
    ASSERT(elf);

    Elf64_Shdr text_shdr = {};
    build_text_shdr(&text_shdr, code_size, text_name_offset);
    buf_write(elf, &text_shdr, sizeof(text_shdr));

    Elf64_Shdr shstrtab_shdr = {};
    build_shstrtab_shdr(&shstrtab_shdr, shstrtab->size,
                        shstrtab_name_offset, shstrtab_offset);
    buf_write(elf, &shstrtab_shdr, sizeof(shstrtab_shdr));

    return buf_dtor(shstrtab);
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t write_elf_upd_hdr (buffer_t* elf, size_t shoff)
{
    ASSERT(elf);

    Elf64_Ehdr elf_hdr = {};

    build_elf_hdr(&elf_hdr, N_PHDRS, N_SHDRS, shoff, 1);

    return buf_rewrite(elf, &elf_hdr, sizeof(elf_hdr), 0);
}

//——————————————————————————————————————————————————————————————————————————————
