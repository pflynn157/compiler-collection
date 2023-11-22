project(orka)

add_subdirectory(lib)

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

# Build the LLVM-based compiler
add_executable(okcc ${SRC} main.cpp)
target_link_libraries(okcc compiler)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(okcc PUBLIC
        -DLINK_CORELIB_LOCATION="${CMAKE_BINARY_DIR}/orka-lang/lib/corelib"
        -DLINK_STDLIB_LOCATION="${CMAKE_BINARY_DIR}/orka-lang/lib/stdlib"
        -DORKA_HEADER_LOCATION="${CMAKE_SOURCE_DIR}/orka-lang/lib/stdlib/include"
        -DLINK_MEMGC_LOCATION="${CMAKE_BINARY_DIR}/runtime/gc"
    )
endif()

