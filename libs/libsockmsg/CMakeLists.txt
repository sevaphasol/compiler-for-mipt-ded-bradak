cmake_minimum_required(VERSION 3.16)

add_library(spl_libsockmsg STATIC
    source/api/sockmsg.cpp
    source/api/cdecl_wrapper.s
    source/backend/socket_adapter.cpp
    source/backend/binmsg_adapter.cpp
    source/backend/details/socket.cpp
    source/backend/details/binmsg.cpp
    source/backend/details/other.cpp
)

target_include_directories(spl_libsockmsg PUBLIC include)

target_compile_options(spl_libsockmsg PRIVATE
    -ffreestanding
    -fno-stack-protector
    -mno-red-zone
    -fno-builtin
    -fno-exceptions
    -fno-rtti
)

set(LIBSOCKMSG_ELF ${SPL_OBJECT_DIR}/libsockmsg.elf)
set(LIBSOCKMSG_SPLOBJ ${SPL_OBJECT_DIR}/libsockmsg.splobj)

add_custom_command(
    OUTPUT ${LIBSOCKMSG_ELF}
    DEPENDS spl_libsockmsg ${SPL_LINKER_SCRIPT}
    COMMAND ${CMAKE_CXX_COMPILER} -nostdlib -static -Wl,-r -T ${SPL_LINKER_SCRIPT}
        -Wl,--whole-archive $<TARGET_FILE:spl_libsockmsg>
        -Wl,--no-whole-archive
        -o ${LIBSOCKMSG_ELF}
    COMMENT "Linking libsockmsg.elf"
)

add_custom_command(
    OUTPUT ${LIBSOCKMSG_SPLOBJ}
    DEPENDS ${LIBSOCKMSG_ELF} spl_elf2splo
    COMMAND $<TARGET_FILE:spl_elf2splo> ${LIBSOCKMSG_ELF} ${LIBSOCKMSG_SPLOBJ}
    COMMENT "Converting libsockmsg.elf to libsockmsg.splobj"
)

add_custom_target(libsockmsg_splobj ALL DEPENDS ${LIBSOCKMSG_SPLOBJ})
set_property(GLOBAL PROPERTY LIBSOCKMSG_SPLOBJ_PATH ${LIBSOCKMSG_SPLOBJ})
