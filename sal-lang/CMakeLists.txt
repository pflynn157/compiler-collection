set(SRC
    lex/lex.cpp
    
    midend/ast_processor.cpp
    midend/midend.cpp
    
    parser/parser.cpp
    
    main.cpp
)

include_directories(.)

add_executable(salc ${SRC})
target_link_libraries(salc compiler)

