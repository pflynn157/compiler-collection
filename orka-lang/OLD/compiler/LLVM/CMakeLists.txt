cmake_minimum_required(VERSION 3.0.0)
project(orka_compiler_llvm)

set(SRC
    Builder.cpp
    Compiler.cpp
    Flow.cpp
    Function.cpp
)

add_library(occompiler_llvm STATIC ${SRC})

llvm_map_components_to_libnames(llvm_libs support core irreader target asmparser
    X86AsmParser
    X86CodeGen
    X86Info
    NVPTXCodeGen
    NVPTXInfo
)

target_link_libraries(occompiler_llvm ${llvm_libs})


