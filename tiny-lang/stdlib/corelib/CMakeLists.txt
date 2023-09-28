set(LIB_FLAGS -nostdlib -c -Wno-builtin-declaration-mismatch)

add_custom_command(
    OUTPUT string.o print.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/string.c ${LIB_FLAGS} -o string.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/print.c ${LIB_FLAGS} -o print.o
    DEPENDS string.c print.c
)

add_custom_command(
    OUTPUT libcorelib.a
    COMMAND ar rcs libcorelib.a string.o print.o
    DEPENDS string.o print.o
)

add_custom_target(lib_corelib ALL DEPENDS libcorelib.a)

