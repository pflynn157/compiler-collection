add_subdirectory(basic)
add_subdirectory(cond)
add_subdirectory(str)
add_subdirectory(struct)
add_subdirectory(syntax)

add_custom_target(test_orka DEPENDS
    test_orka_basic
    test_orka_cond
    test_orka_str
    test_orka_struct
    test_orka_syntax
)

