cmake_minimum_required(VERSION 3.16)

add_library(spl_libstd STATIC
    source/api/libstd.cpp
    source/api/cdecl_wrapper.s
    source/backend/libstd_backend.cpp
)

target_include_directories(spl_libstd PUBLIC include)

target_compile_options(spl_libstd PRIVATE
    -ffreestanding
    -fno-stack-protector
    -mno-red-zone
    -fno-builtin
    -fno-exceptions
    -fno-rtti
)

set(LIBSTD_ELF ${SPL_OBJECT_DIR}/libstd.elf)
set(LIBSTD_SPLOBJ ${SPL_OBJECT_DIR}/libstd.splobj)

add_custom_command(
    OUTPUT ${LIBSTD_ELF}
    DEPENDS spl_libstd ${SPL_LINKER_SCRIPT}
    COMMAND ${CMAKE_CXX_COMPILER} -nostdlib -static -Wl,-r -T ${SPL_LINKER_SCRIPT}
        -Wl,--whole-archive $<TARGET_FILE:spl_libstd>
        -Wl,--no-whole-archive
        -o ${LIBSTD_ELF}
    COMMENT "Linking libstd.elf"
)

add_custom_command(
    OUTPUT ${LIBSTD_SPLOBJ}
    DEPENDS ${LIBSTD_ELF} elf2splo
    COMMAND $<TARGET_FILE:elf2splo> ${LIBSTD_ELF} ${LIBSTD_SPLOBJ}
    COMMENT "Converting libstd.elf to libstd.splobj"
)

add_custom_target(libstd_splobj ALL DEPENDS ${LIBSTD_SPLOBJ})
set_property(GLOBAL PROPERTY LIBSTD_SPLOBJ_PATH ${LIBSTD_SPLOBJ})
