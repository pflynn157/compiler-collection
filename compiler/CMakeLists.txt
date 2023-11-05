project(libcompiler)

set(SRC
    ast/ast.cpp
    ast/ast_builder.cpp
    ast/AstDebug.cpp
    ast/astdot.cpp
    
    parser/base_parser.cpp
    parser/ErrorManager.cpp
    
    midend/ast_midend.cpp
)

set(COMPILER_SRC
    llvm/Builder.cpp
    llvm/Compiler.cpp
    llvm/Flow.cpp
    llvm/Function.cpp
    llvm/Variable.cpp
)

add_library(compiler_base STATIC ${SRC})
add_library(compiler STATIC ${COMPILER_SRC})

llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
    X86AsmParser
    X86CodeGen
    X86Info
)

target_link_libraries(compiler
    compiler_base
    ${llvm_libs}
)

