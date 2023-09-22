add_subdirectory(core)
add_subdirectory(output)

add_custom_target(test DEPENDS test_core test_output)

