project(orka)

include_directories(.)

set(SRC
    lex/Lex.cpp
    
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    preproc/Preproc.cpp
    
    main.cpp
)

add_executable(orka ${SRC})
target_link_libraries(orka compiler_llvm)

