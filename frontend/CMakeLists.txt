find_program(FLEX_EXECUTABLE flex)
find_program(BISON_EXECUTABLE bison)

if(NOT FLEX_EXECUTABLE)
    message(FATAL_ERROR "flex not found")
endif()

if(NOT BISON_EXECUTABLE)
    message(FATAL_ERROR "bison not found")
endif()

set(FRONTEND_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
set(LEXER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l)
set(PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y)
set(LEXER_OUT ${FRONTEND_GENERATED_DIR}/lexer.cpp)
set(PARSER_OUT ${FRONTEND_GENERATED_DIR}/parser.cpp)
set(PARSER_HDR ${FRONTEND_GENERATED_DIR}/parser.hpp)

file(MAKE_DIRECTORY ${FRONTEND_GENERATED_DIR})

add_custom_command(
    OUTPUT ${PARSER_OUT} ${PARSER_HDR}
    COMMAND ${BISON_EXECUTABLE} -d -o ${PARSER_OUT} ${PARSER_SRC}
    DEPENDS ${PARSER_SRC}
    COMMENT "Generating parser.cpp/parser.hpp"
    VERBATIM
)

add_custom_command(
    OUTPUT ${LEXER_OUT}
    COMMAND ${FLEX_EXECUTABLE} -o ${LEXER_OUT} ${LEXER_SRC}
    DEPENDS ${LEXER_SRC} ${PARSER_HDR}
    COMMENT "Generating lexer.cpp"
    VERBATIM
)

add_executable(frontend
    src/main.cpp
    src/parser_utils.cpp
    ${PARSER_OUT}
    ${LEXER_OUT}
)

target_include_directories(frontend
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/gen
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(frontend PRIVATE compiler_commons)

target_compile_options(frontend
    PRIVATE
        ${PROJECT_APP_WARNING_FLAGS}
)
