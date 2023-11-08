project(sia-lang)

include_directories(.)

set(SRC
    lex/lex.cpp

    main.cpp
)

add_executable(siacc ${SRC})

target_link_libraries(siacc compiler)

