//——————————————————————————————————————————————————————————————————————————————

#include "custom_assert.h"
#include "elf_program_header_builders.h"
#include "elf_builder_defines.h"

//——————————————————————————————————————————————————————————————————————————————

void build_exe_phdr(Elf64_Phdr* phdr, size_t code_size)
{
    ASSERT(phdr);

    /* Executable segment must be loadable into the memory. */
    phdr->p_type   = PT_LOAD;

    /* p_offset = PAGE_SIZE = 0x1000 means that segment starts at 0x1000.
       Because 1) p_offset must be larger than 0 for obvious reasons;
               2) p_offset % p_align = p_vaddr % p_align.
       More details about p_vaddr anf p_align below.
       So we place code at PAGE_SIZE == 0x1000,
       after ELF header, Program Header Table and padding. */
    phdr->p_offset = PAGE_SIZE;

    /* p_vaddr = DEFAULT_ENTRY_ADDRESS = 0x400000 means that loaded segment
       will have 0x400000 address in virtual memory.
       This value is default for static linked executable
       in Linux x86-64. */
    phdr->p_vaddr  = DEFAULT_ENTRY_ADDRESS;

    /* p_paddr is a physical address in memory of loaded segment.
       In x86-64 there is no need for it, so it setted to zero. */
    phdr->p_paddr  = 0;

    /* p_filesz - the size of the segment it occupies in the ELF file */
    phdr->p_filesz = code_size;

    /* p_memsz - the size of the loaded segment it occupies in the memory */
    phdr->p_memsz  = code_size;

    /* Stdlib code is appended together with its writable data. */
    phdr->p_flags  = PF_R | PF_W | PF_X;

    /* PAGE_SIZE = 0x1000 — default alignment in x86-64. */
    phdr->p_align  = PAGE_SIZE;
}

//——————————————————————————————————————————————————————————————————————————————
