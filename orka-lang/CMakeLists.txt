project(orka)

#add_subdirectory(stdlib)

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
)

# Build the LLVM-based compiler
add_executable(okcc ${SRC} main.cpp)
target_link_libraries(okcc compiler_llvm)


