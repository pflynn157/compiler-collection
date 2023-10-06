set(LIB_FLAGS -nostdlib -c -Wno-builtin-declaration-mismatch)

add_custom_command(
    OUTPUT io.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/io.c ${LIB_FLAGS} -o io.o
    DEPENDS io.c
)

add_custom_command(
    OUTPUT libstdlib.a
    COMMAND ar rcs libstdlib.a io.o
    DEPENDS io.o
)

add_custom_target(lib_orka_stdlib ALL DEPENDS libstdlib.a)

