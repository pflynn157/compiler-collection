set(SRC
    lex/lex.cpp
    
    parser/parser.cpp
    
    main.cpp
)

include_directories(.)

add_executable(salc ${SRC})
target_link_libraries(salc compiler_llvm)

