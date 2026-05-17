file(GLOB MIDDLEEND_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(midend ${MIDDLEEND_SOURCES})

target_include_directories(midend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(midend PRIVATE compiler_commons)

target_compile_options(midend
    PRIVATE
        ${PROJECT_APP_WARNING_FLAGS}
)
