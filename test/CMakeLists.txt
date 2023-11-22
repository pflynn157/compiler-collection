add_subdirectory(tiny-lang/core)
add_subdirectory(tiny-lang/output)
add_subdirectory(tiny-lang/lex)

add_subdirectory(espresso)
add_subdirectory(orka)

add_subdirectory(tli)

if (DISABLE_OUTPUT_TEST)
    add_custom_target(test_llvm DEPENDS
        test_core
        test_lex
    )
else()
    add_custom_target(test_llvm DEPENDS
        test_core
        test_output
        test_lex
    )
endif()

##
## The target for all tests
##
add_custom_target(test DEPENDS
    test_llvm
    test_orka
    test_espresso
    test_tli
)

