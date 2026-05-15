cmake_minimum_required(VERSION 3.16)
project(random_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_language(ASM)
enable_language(ASM_NASM)

set(ASM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/asm_out)

set(EXTRACT_OBJ_DIR ${CMAKE_BINARY_DIR}/stdlib_obj)

add_library(random_client STATIC
    global_src/std/src/io.c
    global_src/std/src/wrappers.s
    global_src/std/src/io_old.s
)

# BE CAREFULL WITH THIS!!!!:
set_source_files_properties (global_src/std/src/io_old.s PROPERTIES LANGUAGE ASM_NASM)
# flags to simplify compiled asm file for SPL compiler compability
set_source_files_properties(global_src/std/src/io.c global_src/std/src/wrappers.s PROPERTIES
    COMPILE_OPTIONS "-save-temps=obj;-masm=intel;-fno-asynchronous-unwind-tables;-ffreestanding;-nostdlib;-mno-red-zone;-fno-builtin;-fno-stack-protector;-fno-ident;-ffunction-sections;-fdata-sections;-g0"
)

target_include_directories(random_client PRIVATE 
    global_src/client/inc
    global_src/std/inc
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/stdlib.bin
    DEPENDS random_client
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTRACT_OBJ_DIR}
    
    # Extract all object files from the static library
    COMMAND cd ${EXTRACT_OBJ_DIR} && ar x ${CMAKE_BINARY_DIR}/librandom_client.a
    
    # Link all .o files into a fully resolved ELF using stdlib.ld
    COMMAND ld -static -T ${CMAKE_CURRENT_SOURCE_DIR}/linker/stdlib.ld -o ${CMAKE_BINARY_DIR}/stdlib.elf ${EXTRACT_OBJ_DIR}/*.o
    
    COMMENT "Building fully resolved stdlib.bin from static library"
)

add_custom_command(TARGET random_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ASM_OUTPUT_DIR}
    COMMAND find ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/random_client.dir -name "*.s" -exec cp {} ${ASM_OUTPUT_DIR} "\\;"
    COMMENT "Copying generated assembly files"
)

add_custom_target(stdlib_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/stdlib.bin)