cmake_minimum_required(VERSION 3.0.0)
project(orka_frontend)

set(SRC
    lex/Lex.cpp
    
    debug/AstDebug.cpp
    debug/LexDebug.cpp
    
    error/Manager.cpp
    
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    preproc/Preproc.cpp
)

add_library(ocparser STATIC ${SRC})

