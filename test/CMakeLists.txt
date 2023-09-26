add_subdirectory(core)
add_subdirectory(output)
add_subdirectory(lex)
add_subdirectory(llir)

if (DISABLE_OUTPUT_TEST)
    add_custom_target(test DEPENDS
        test_core
        test_lex
    )
else()
    add_custom_target(test DEPENDS
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

add_custom_target(test_all DEPENDS
    test
    test_llir
    test_libllir
)

## For LLIR and the custom assembler
add_custom_target(test_toolchain DEPENDS
    test_core_toolchain
)

