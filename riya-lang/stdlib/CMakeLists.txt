project(stdlib)

##
## Start file
##
add_custom_command(
    OUTPUT amd64_start.o
    COMMAND ${CMAKE_BINARY_DIR}/as/x86/asx86 ${CMAKE_CURRENT_SOURCE_DIR}/amd64.asm amd64_start.o
    DEPENDS amd64.asm
)

add_custom_target(lib_start ALL DEPENDS amd64_start.o asx86)

##---------------------------------------
## The standard library
##---------------------------------------
add_subdirectory(corelib)


