project(sia-lang)

include_directories(.)

set(SRC
    lex/lex.cpp
    
    midend/midend.cpp
    
    parser/expression.cpp
    parser/function.cpp
    parser/parser.cpp

    main.cpp
)

add_executable(siacc ${SRC})

target_link_libraries(siacc compiler)

