project(tlc_frontend)

add_subdirectory(stdlib)

include_directories(.)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lex/lex.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/lex/lex.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/lex/lex.py ${CMAKE_SOURCE_DIR}/lex
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lex/config.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lex
)

set(SRC
    parser/Expression.cpp
    parser/Flow.cpp
    parser/Function.cpp
    parser/Parser.cpp
    parser/Structure.cpp
    parser/Variable.cpp
    
    lex/lex.cpp
    
    midend/midend.cpp
)

add_executable(tlc ${SRC} main.cpp)
target_link_libraries(tlc compiler)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(tlc PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/tiny-lang/stdlib"
    )
endif()

