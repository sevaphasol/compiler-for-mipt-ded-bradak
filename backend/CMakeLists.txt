file(GLOB BACKEND_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(backend ${BACKEND_SOURCES})

target_include_directories(backend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(backend PRIVATE compiler_commons)

target_compile_options(backend
    PRIVATE
        ${PROJECT_APP_WARNING_FLAGS}
)
