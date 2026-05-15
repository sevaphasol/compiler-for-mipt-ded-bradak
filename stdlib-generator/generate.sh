#!/bin/bash
set -e

cd stdlib
# Configure the build directory
cmake -B build

# Build the project (this triggers the POST_BUILD command)
cmake --build build

echo "------------------------------------------------"
echo "Build successful."
echo "Assembly files collected in: build/asm_out/"
ls build/asm_out/


python3 extract_stdlib.py build/stdlib.elf stdlib.bin

cd -

# rm ./../back-end/lib/stdlib.bin
mv stdlib/stdlib.bin ./../back-end/lib/stdlib.bin