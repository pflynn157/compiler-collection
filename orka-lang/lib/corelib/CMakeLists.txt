set(LIB_FLAGS -nostdlib -c -Wno-builtin-declaration-mismatch)

add_custom_command(
    OUTPUT io.o str.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/io.c ${LIB_FLAGS} -o io.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/str.c ${LIB_FLAGS} -o str.o
    DEPENDS io.c str.c
)

add_custom_command(
    OUTPUT libcorelib.a
    COMMAND ar rcs libcorelib.a io.o str.o
    DEPENDS io.o str.o
)

add_custom_target(lib_orka_corelib ALL DEPENDS libcorelib.a)

