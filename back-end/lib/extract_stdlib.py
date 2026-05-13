#!/usr/bin/env python3
import sys
import struct
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

def main(elf_path, combined_out):
    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)

        text_section = None 
        for section in elf.iter_sections():
            if section.name == '.text':
                text_section = section
                break
        if not text_section:
            print("No .text section found", file=sys.stderr)
            sys.exit(1)

        symtab = None
        for section in elf.iter_sections():
            if isinstance(section, SymbolTableSection):
                symtab = section
                break
        if not symtab:
            print("No symbol table found", file=sys.stderr)
            sys.exit(1)

        offsets = []
        for symbol in symtab.iter_symbols():
            if symbol['st_info']['type'] == 'STT_FUNC' and symbol.name:
                offset = symbol['st_value'] - text_section['sh_addr']
                offsets.append((symbol.name, offset))

        if not offsets:
            print("No functions found", file=sys.stderr)
            sys.exit(1)

        with open(combined_out, 'wb') as out:
            out.write(b'STDL')                     # magic
            out.write(struct.pack('<I', 1))        # version
            out.write(struct.pack('<I', len(offsets)))

            # Write offset table (name + 4-byte offset)
            for name, offset in offsets:
                out.write(name.encode('utf-8') + b'\0')
                out.write(struct.pack('<I', offset))

            # Append raw .text
            out.write(text_section.data())

    print(f"Created {combined_out} with {len(offsets)} functions")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: extract_stdlib.py <elf> <combined_out>", file=sys.stderr)
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])