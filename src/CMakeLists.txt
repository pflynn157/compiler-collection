project(tl_frontend)

if(PTASM)
    set(COMPILER_SRC
        compiler/PTASM/compiler.cpp
        compiler/PTASM/elf.cpp
        compiler/PTASM/ptasm.cpp
        compiler/PTASM/x86.cpp
    )
else()
endif()

set(LLVM_COMPILER_SRC
    compiler/LLVM/Builder.cpp
    compiler/LLVM/Compiler.cpp
    compiler/LLVM/Flow.cpp
    compiler/LLVM/Function.cpp
    compiler/LLVM/Variable.cpp
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

if(PTASM)
    add_executable(tlc ${SRC} ptasm_main.cpp)
else()
    # Build the LLVM-based compiler
    add_executable(tlc ${SRC} ${LLVM_COMPILER_SRC} main.cpp)

    llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
        X86AsmParser
        X86CodeGen
        X86Info
    )

    target_link_libraries(tlc
        ${llvm_libs}
    )
    
    # Build the LLIR-based compiler
    add_executable(ocllir ${SRC} ${LLIR_COMPILER_SRC} llir_main.cpp)
    target_link_libraries(ocllir llir)
    
    # Enable dev linking mode
    if (DEV_LINK_MODE)
        target_compile_options(tlc PUBLIC
            -DLINK_LOCATION="${CMAKE_BINARY_DIR}/stdlib"
        )
        
        target_compile_options(ocllir PUBLIC
            -DLINK_LOCATION="${CMAKE_BINARY_DIR}/stdlib"
            -DAS_LOCATION="${CMAKE_BINARY_DIR}/as"
        )
    endif()
endif()

