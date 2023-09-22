set(CORE_TEST_SRC
    test1
    func_syntax_all
    
    string1 str1
    
    array_func1 char_array1
    int64_array1 int64_array2
    int_array1 int_array2
    str_array1 str_array2
    uint64_array1 uint64_array2
    uint_array1 uint_array2
)

foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/src/tlc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry -o ${ITEM}.exe
        COMMAND ./${ITEM}.exe > output.txt
        COMMAND rm ${ITEM}.exe
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.ry"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test_output
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_output tlc)

