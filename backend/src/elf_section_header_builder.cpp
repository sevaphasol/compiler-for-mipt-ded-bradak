//——————————————————————————————————————————————————————————————————————————————

#include "custom_assert.h"
#include "elf_section_header_builders.h"
#include "elf_builder_defines.h"

#include <string.h>

//——————————————————————————————————————————————————————————————————————————————

lang_status_t shstrtab_init(buffer_t* tab, size_t init_capacity)
{
    ASSERT(tab);

    buf_ctor(tab, init_capacity);
    tab->size = 1; // First symbol of shstrtab must be '\0'

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t add_section_name(buffer_t* tab, const char* name, uint32_t* offset)
{
    ASSERT(tab);
    ASSERT(name);
    ASSERT(offset);

    *offset = (uint32_t) tab->size;
    buf_write(tab, name, strlen(name) + 1); // +1 for including '\0'

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

void build_text_shdr(Elf64_Shdr* shdr, size_t code_size, uint32_t name_offset)
{
    ASSERT(shdr);

    /* sh_name is an offset to the section name in .shstrtab. */
    shdr->sh_name      = name_offset;

    /* sh_type describes what kind of data this section contains.
       SHT_PROGBITS means that section holds
       information defined by the program*/
    shdr->sh_type      = SHT_PROGBITS;

    /* sh_flags defines attributes of the section.
       SHF_EXECINSTR — executable instructions.
       SHF_ALLOC     — this section occupies memory during execution. */
    shdr->sh_flags     = SHF_EXECINSTR | SHF_ALLOC;

    /* sh_addr is the virtual address at which the first byte
       of this section resides in memory. For .text
       sh_addr = DEFAULT_ENTRY_ADDRESS (see segment_builders.cpp). */
    shdr->sh_addr      = DEFAULT_ENTRY_ADDRESS;

    /* sh_offset is the byte offset from the beginning of the file to
       the first byte in section. We place .text at PAGE_SIZE == 0x1000,
       after ELF header, Program Header Table and padding. */
    shdr->sh_offset    = PAGE_SIZE;

    /* sh_size is the size of section in bytes.
       Equals to machine code size passed as parameter. */
    shdr->sh_size      = code_size;

    /* sh_link is used for section-specific interpretation.
       For SHT_PROGBITS sections this field doesn't have meaning
       and set to SHN_UNDEF. */
    shdr->sh_link      = SHN_UNDEF;

    /* sh_info is used for section-specific information.
       Also unused for SHT_PROGBITS, set to zero. */
    shdr->sh_info      = 0;

    /* sh_addralign is the required alignment of the section. */
    shdr->sh_addralign = PAGE_SIZE;

    /* sh_entsize is needed for sections that contain fixed-size entries.
       For .text it is not needed sp is is setted to zero. */
    shdr->sh_entsize   = 0;
}

//——————————————————————————————————————————————————————————————————————————————

void build_shstrtab_shdr(Elf64_Shdr* shdr, size_t shstrtab_size, uint32_t name_offset, size_t table_offset)
{
    ASSERT(shdr);

    /* sh_name is an offset to the section name in .shstrtab. */
    shdr->sh_name      = name_offset;

    /* SHT_STRTAB means that this section is a string table. */
    shdr->sh_type      = SHT_STRTAB;

    /* Flags are not needed for shstrtab. */
    shdr->sh_flags     = 0;

    /* This section is not loaded into memory, so sh_addr setted to zero. */
    shdr->sh_addr      = 0;

    /* sh_offset is the byte offset from the beginning of the file
       to the first byte in section. */
    shdr->sh_offset    = table_offset;

    /* sh_size is the size of section in bytes. */
    shdr->sh_size      = shstrtab_size;

    /* sh_link is used for section-specific interpretation.
       For SHT_STRTAB it has no meaning and set to SHN_UNDEF. */
    shdr->sh_link      = SHN_UNDEF;

    /* sh_info is used for section-specific information.
       Also unused for SHT_STRTAB, set to zero. */
    shdr->sh_info      = 0;

    /* sh_addralign is the required alignment of the section.
       String tables do not require special alignment, 1-byte is fine. */
    shdr->sh_addralign = 1;

    /* sh_entsize is needed for sections that contain fixed-size entries.
       For string tables it is not needed sp is is setted to zero. */
    shdr->sh_entsize   = 0;
}

//——————————————————————————————————————————————————————————————————————————————
