set(CORE_TEST_SRC
    test1
    func_syntax_all
    
    string1
    #str1
    
    array_func1 char_array1
    int64_array1 int64_array2
    int_array1 int_array2
    str_array1 str_array2
    uint64_array1 uint64_array2
    uint_array1 uint_array2
)

set(LLIR_CORE_TEST_SRC
    test1
    func_syntax_all
    
    string1
    #str1
    
    #array_func1
    #char_array1
    #int64_array1
    #int64_array2
    #int_array1
    #int_array2
    #str_array1
    #str_array2
    #uint64_array1
    #uint64_array2
    #uint_array1
    #uint_array2
)

## LLVM testing
foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/src/occ ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry -o ${ITEM}.exe
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

add_dependencies(test_output occ)


## LLIR testing
foreach(ITEM ${LLIR_CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.llir.exe
        COMMAND ${CMAKE_BINARY_DIR}/src/ocllir ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry -o ${ITEM}.llir.exe
        COMMAND ./${ITEM}.llir.exe > output.txt
        COMMAND rm ${ITEM}.llir.exe
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.ry"
    )
    
    set(TEST_OUTPUTS_LLIR
        ${TEST_OUTPUTS_LLIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.llir.exe
    )
endforeach()

add_custom_target(test_output_llir
    DEPENDS ${TEST_OUTPUTS_LLIR}
)

add_dependencies(test_output_llir ocllir)

