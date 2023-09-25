set(LLIR_TEST_SRC
    array_func1
    bitwise1
    bool1
    byte1
    char1
    cond1
    cond2
    cond3
    cond4
    cond_byte
    cond_int64
    cond_short
    cond_ubyte
    cond_uint64
    cond_ushort
    const1
    first
    int64_1
    int64_array1
    int64_array2
    int_array1
    int_array2
    math1
    neg1
    op_pred
    short1
    ubyte1
    uint1
    uint64_1
    uint64_array1
    uint64_array2
    uint_array1
    uint_array2
    ushort1
)

foreach(ITEM ${LLIR_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/llir/compiler/llircc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.li -o ${ITEM}.exe
        COMMAND ./${ITEM}.exe > output.txt
        COMMAND rm ${ITEM}.exe
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.txt ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.li"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test_llir_base
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_llir_base llircc)


