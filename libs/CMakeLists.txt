cmake_minimum_required(VERSION 3.16)

set(SPL_OBJECT_DIR ${CMAKE_BINARY_DIR}/splobj)
set(SPL_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/merge.ld)
file(MAKE_DIRECTORY ${SPL_OBJECT_DIR})

add_subdirectory(libstd)
add_subdirectory(libsockmsg)
