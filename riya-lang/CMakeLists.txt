project(riya)

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

add_library(riya STATIC ${SRC})

add_executable(riyac main.cpp)
target_link_libraries(riyac riya compiler)

add_executable(riyai main_intr.cpp)
target_link_libraries(riyai riya compiler_base compiler_intr)

add_executable(riyajc main_java.cpp)
target_link_libraries(riyajc riya compiler_base coffee-maker)

# Enable dev linking mode
if (DEV_LINK_MODE)
    target_compile_options(riyac PUBLIC
        -DLINK_LOCATION="${CMAKE_BINARY_DIR}/riya-lang/stdlib"
    )
endif()

