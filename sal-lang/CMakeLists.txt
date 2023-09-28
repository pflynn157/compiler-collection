set(SRC
    main.cpp
)

add_executable(salc ${SRC})
target_link_libraries(salc compiler_llvm)

