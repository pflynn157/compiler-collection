cmake_minimum_required(VERSION 3.0.0)
project(orka_main)

set(SRC
    main.cpp
)

add_executable(occ ${SRC})

target_link_libraries(occ
    ocparser
    occompiler_llvm
    occompiler_llir
)

