cmake_minimum_required(VERSION 3.16)

project(compiler_for_mipt_ded_bradak LANGUAGES CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(PROJECT_WARNING_FLAGS
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wempty-body
    -Wformat-security
    -Wformat=2
    -Wignored-qualifiers
    -Wlogical-op
    -Wno-missing-field-initializers
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wpointer-arith
    -Wsign-promo
    -Wstrict-aliasing
    -Wstrict-null-sentinel
    -Wtype-limits
    -Wwrite-strings
    -fexceptions
    -pipegi
)

set(PROJECT_APP_WARNING_FLAGS
    ${PROJECT_WARNING_FLAGS}
    -Wconversion
    -Wstack-usage=8192
)

file(MAKE_DIRECTORY
    ${CMAKE_SOURCE_DIR}/logs
    ${CMAKE_SOURCE_DIR}/logs/dot_files
    ${CMAKE_SOURCE_DIR}/logs/images
)

add_custom_target(clean_logs
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_SOURCE_DIR}/logs/dot_files/*.dot
        ${CMAKE_SOURCE_DIR}/logs/images/*.png
    COMMENT "Cleaning graph dump logs"
)

add_executable(elf2splo tools/elf2splo.cpp)
add_executable(pan2lang tools/pan2lang.cpp)

add_subdirectory(commons)
add_subdirectory(frontend)
add_subdirectory(backend)
add_subdirectory(midend)
add_subdirectory(linker)
add_subdirectory(libs)
