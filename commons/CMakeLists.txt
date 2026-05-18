file(GLOB COMMONS_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(compiler_commons STATIC ${COMMONS_SOURCES})

target_include_directories(compiler_commons
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(compiler_commons
    PRIVATE
        ${PROJECT_WARNING_FLAGS}
)
