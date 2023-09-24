project(stdlib)

##
## Start file
##
add_custom_command(
    OUTPUT amd64_start.o
    COMMAND as ${CMAKE_CURRENT_SOURCE_DIR}/amd64.asm -o amd64_start.o
    DEPENDS amd64.asm
)

add_custom_target(lib_start ALL DEPENDS amd64_start.o)

##---------------------------------------
## The standard library
##---------------------------------------
add_subdirectory(corelib)


