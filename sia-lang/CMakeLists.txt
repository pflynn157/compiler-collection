project(sia-lang)

include_directories(.)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lex/lex.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/lex/lex.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/lex/lex.py ${CMAKE_SOURCE_DIR}/lex
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lex/config.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lex
)

set(SRC
    lex/lex.cpp
    
    midend/midend.cpp
    
    parser/expression.cpp
    parser/flow.cpp
    parser/function.cpp
    parser/parser.cpp
    parser/variable.cpp

    main.cpp
)

add_executable(siacc ${SRC})

target_link_libraries(siacc compiler)

