project(riscv-as)

set(SRC
    lex.cpp
    main.cpp
    pass1.cpp
    pass2.cpp
)

add_executable(asrv ${SRC})

