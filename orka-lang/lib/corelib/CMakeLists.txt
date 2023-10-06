set(LIB_FLAGS -nostdlib -c -Wno-builtin-declaration-mismatch)

add_custom_command(
    OUTPUT io.o str.o sys.o syscall_x86.o x64_start.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/io.c ${LIB_FLAGS} -o io.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/str.c ${LIB_FLAGS} -o str.o
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/sys.c ${LIB_FLAGS} -o sys.o
    COMMAND as ${CMAKE_CURRENT_SOURCE_DIR}/syscall_x86.asm -o syscall_x86.o
    COMMAND as ${CMAKE_CURRENT_SOURCE_DIR}/x64_start.asm -o x64_start.o
    DEPENDS io.c str.c sys.c syscall_x86.asm x64_start.asm
)

add_custom_command(
    OUTPUT libcorelib.a
    COMMAND ar rcs libcorelib.a io.o str.o sys.o syscall_x86.o x64_start.o
    DEPENDS io.o str.o sys.o syscall_x86.o x64_start.o
)

add_custom_target(lib_orka_corelib ALL DEPENDS libcorelib.a)

