project(tl_frontend)

set(LLVM_COMPILER_SRC
    llvm/Builder.cpp
    llvm/Compiler.cpp
    llvm/Flow.cpp
    llvm/Function.cpp
    llvm/Variable.cpp
)

set(LLIR_COMPILER_SRC
    llir/Builder.cpp
    llir/Compiler.cpp
    llir/Flow.cpp
    llir/Function.cpp
)

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
    
    lex/lex.cpp
    lex/lex_debug.cpp
    
    midend/ast_processor.cpp
    midend/midend.cpp    
)

# Build the LLVM-based compiler
add_executable(occ ${SRC} ${LLVM_COMPILER_SRC} main.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
    X86AsmParser
    X86CodeGen
    X86Info
)

target_link_libraries(occ
    ${llvm_libs}
)

# Build the LLIR-based compiler
add_executable(ocllir ${SRC} ${LLIR_COMPILER_SRC} llir_main.cpp)
target_link_libraries(ocllir llir)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(occ PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/stdlib"
    )
    
    target_compile_options(ocllir PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/stdlib"
        -DAS_LOCATION="${CMAKE_BINARY_DIR}/as"
    )
endif()

