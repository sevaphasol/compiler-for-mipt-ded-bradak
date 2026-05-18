//——————————————————————————————————————————————————————————————————————————————

#include "custom_assert.h"
#include "elf_builder_defines.h"
#include "elf_header_builder.h"

//——————————————————————————————————————————————————————————————————————————————

void build_ident(unsigned char* e_ident)
{
    ASSERT(e_ident);

    e_ident[EI_MAG0]       = ELFMAG0;       // 0x7F
    e_ident[EI_MAG1]       = ELFMAG1;       // 'E'
    e_ident[EI_MAG2]       = ELFMAG2;       // 'L'
    e_ident[EI_MAG3]       = ELFMAG3;       // 'F'
    e_ident[EI_CLASS]      = ELFCLASS64;    // 64-bit architecture
    e_ident[EI_DATA]       = ELFDATA2LSB;   // Little-endian
    e_ident[EI_VERSION]    = EV_CURRENT;    // Current version
    e_ident[EI_OSABI]      = ELFOSABI_SYSV; // UNIX System V ABI
    e_ident[EI_ABIVERSION] = 0;             // Usually 0

    for (int pad_ind = EI_PAD; pad_ind < EI_NIDENT; pad_ind++) {
        e_ident[pad_ind] = 0; // Padding contains zeroes
    }
}

//==============================================================================

void build_elf_hdr(Elf64_Ehdr* hdr, uint16_t n_phdrs, uint16_t n_shdrs, Elf64_Off shoff, Elf64_Half shstrndx)
{
    ASSERT(hdr);

    build_ident(hdr->e_ident);

    hdr->e_type      = ET_EXEC;               // Executable
    hdr->e_machine   = EM_X86_64;             // For x86-64
    hdr->e_version   = EV_CURRENT;            // Current version
    hdr->e_entry     = DEFAULT_ENTRY_ADDRESS; // Because of 1KB padding it is a constant
    hdr->e_phoff     = sizeof(Elf64_Ehdr);    // Program header table after elf header
    hdr->e_flags     = 0;                     // x86-64 doesn't use flags
    hdr->e_ehsize    = sizeof(Elf64_Ehdr);    // Size of elf header
    hdr->e_phentsize = sizeof(Elf64_Phdr);    // Size of element in Program Header Table
    hdr->e_phnum     = n_phdrs;               // 1 segment: .text
    hdr->e_shoff     = shoff;                 // Offset of Section Header Table
    hdr->e_shentsize = sizeof(Elf64_Shdr);    // Size of element in Section Header Table
    hdr->e_shnum     = n_shdrs;               // 2 sections: .text and .shstrtab
    hdr->e_shstrndx  = shstrndx;              // Index of .shstrtab in Section Headers Table
}

//——————————————————————————————————————————————————————————————————————————————
