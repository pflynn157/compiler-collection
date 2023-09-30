project(libcompiler)

set(SRC
    ast/ast.cpp
    ast/ast_builder.cpp
    ast/AstDebug.cpp
    ast/astdot.cpp
    
    parser/base_parser.cpp
    parser/ErrorManager.cpp
)

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

add_library(compiler_base STATIC ${SRC})
add_library(compiler_llvm STATIC ${LLVM_COMPILER_SRC})
add_library(compiler_llir STATIC ${LLIR_COMPILER_SRC})

llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
    X86AsmParser
    X86CodeGen
    X86Info
)

target_link_libraries(compiler_llvm
    ${llvm_libs}
    compiler_base
)

target_link_libraries(compiler_llir llir compiler_base)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(compiler_llvm PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/tiny-lang/stdlib"
    )
    
    target_compile_options(compiler_llir PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/tiny-lang/stdlib"
        -DAS_LOCATION="${CMAKE_BINARY_DIR}/as"
    )
endif()

