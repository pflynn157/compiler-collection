add_subdirectory(llir)

add_subdirectory(tiny-lang/core)
add_subdirectory(tiny-lang/output)
add_subdirectory(tiny-lang/lex)

add_subdirectory(espresso)
add_subdirectory(orka)

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

## For LLIR
add_custom_target(test_llir DEPENDS
    test_core_llir
    test_output_llir
    test_lex
)

add_custom_target(test_libllir DEPENDS
    test_llir_base
)

## For LLIR and the custom assembler
add_custom_target(test_toolchain DEPENDS
    test_core_toolchain
)

##
## The target for all tests
##
add_custom_target(test DEPENDS
    test_llvm
    test_llir
    test_libllir
    test_toolchain
    test_orka
    test_espresso
)

