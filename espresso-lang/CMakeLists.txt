include_directories(.)

set(SRC
    lex/Lex.cpp
    
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    main.cpp
)

add_executable(eoc ${SRC})

target_link_libraries(eoc
    compiler_base
    coffee-maker
)

