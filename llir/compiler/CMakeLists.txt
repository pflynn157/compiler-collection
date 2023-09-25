project(llir_compiler)

set(SRC
    Lex.cpp
    main.cpp
    parser.cpp
)

add_executable(llircc ${SRC})

target_link_libraries(llircc llir)
