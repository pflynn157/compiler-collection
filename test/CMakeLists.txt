add_subdirectory(core)
add_subdirectory(output)
add_subdirectory(lex)

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
add_custom_target(test_llir DEPENDS test_core_llir)

