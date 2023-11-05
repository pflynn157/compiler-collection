project(tlc_frontend)

add_subdirectory(stdlib)

include_directories(.)

set(SRC
    parser/Expression.cpp
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    lex/lex.cpp
    lex/lex_debug.cpp
    
    midend/midend.cpp
)

add_executable(tlc ${SRC} main.cpp)
target_link_libraries(tlc compiler)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(tlc PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/tiny-lang/stdlib"
    )
endif()

