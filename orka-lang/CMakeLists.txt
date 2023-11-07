project(orka)

add_subdirectory(lib)

include_directories(.)

set(SRC
    parser/Expression.cpp
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    preproc/Preproc.cpp
    
    lex/lex.cpp
    lex/lex_debug.cpp
    
    midend/midend.cpp
)

# Build the LLVM-based compiler
add_executable(okcc ${SRC} main.cpp)
target_link_libraries(okcc compiler)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(okcc PUBLIC
        -DLINK_CORELIB_LOCATION="${CMAKE_BINARY_DIR}/orka-lang/lib/corelib"
        -DLINK_STDLIB_LOCATION="${CMAKE_BINARY_DIR}/orka-lang/lib/stdlib"
        -DORKA_HEADER_LOCATION="${CMAKE_SOURCE_DIR}/orka-lang/lib/stdlib/include"
    )
endif()

