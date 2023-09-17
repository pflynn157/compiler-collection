cmake_minimum_required(VERSION 3.0.0)
project(tl_frontend)

if(PTASM)
    set(COMPILER_SRC
        compiler/PTASM/compiler.cpp
        compiler/PTASM/elf.cpp
        compiler/PTASM/ptasm.cpp
        compiler/PTASM/x86.cpp
    )
else()
    set(COMPILER_SRC
        compiler/LLVM/Builder.cpp
        compiler/LLVM/Compiler.cpp
        compiler/LLVM/Flow.cpp
        compiler/LLVM/Function.cpp
        compiler/LLVM/Variable.cpp
    )
endif()

set(LEX_SRC lex/lex.c)

set(SRC
    ast/ast.cpp
    ast/ast_builder.cpp
    ast/AstDebug.cpp
    ast/astdot.cpp
    
    parser/ErrorManager.cpp
    parser/Expression.cpp
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    preproc/Preproc.cpp
    
    # Compile sources
    ${COMPILER_SRC}
)

add_library(tlclex STATIC ${LEX_SRC})

if(PTASM)
    add_executable(tlc ${SRC} ptasm_main.cpp)
    target_link_libraries(tlc tlclex)
else()
    add_executable(tlc ${SRC} main.cpp)

    llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
        X86AsmParser
        X86CodeGen
        X86Info
    )

    target_link_libraries(tlc
        tlclex
        ${llvm_libs}
    )
endif()

