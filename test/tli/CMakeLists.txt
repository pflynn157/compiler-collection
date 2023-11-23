set(CORE_TEST_SRC
    test1 test2 test3
    print_add1
    imath1 imath2
    iprint1
    func1 func2 func3 func4 func5 func6 func7
    func8 func9 func10
    cond1 cond2
    while1
    array1
    int_array1 int_array2
    str_array1
    char1
    string1 string2 string3
    string_func1
    array_len
    func_array1 func_array2
)

foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}_output.txt
        COMMAND ${CMAKE_BINARY_DIR}/tiny-lang/tlci ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry > ${ITEM}_output.txt
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./${ITEM}_output.txt
        COMMAND rm ${ITEM}_output.txt
        COMMAND echo "[PASS][RY_INTR] ${ITEM}.ry"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}_output.txt
    )
endforeach()

add_custom_target(test_tli
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_tli tlci)

