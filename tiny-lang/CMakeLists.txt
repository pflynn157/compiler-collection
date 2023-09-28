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
    
    preproc/Preproc.cpp
    
    lex/lex.cpp
    lex/lex_debug.cpp
    
    midend/ast_processor.cpp
    midend/midend.cpp    
)

# Build the LLVM-based compiler
add_executable(tlc ${SRC} main.cpp)
target_link_libraries(tlc compiler_llvm)

# Build the LLIR-based compiler
add_executable(tlcllir ${SRC} llir_main.cpp)
target_link_libraries(tlcllir compiler_llir)

