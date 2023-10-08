set(CORE_TEST_SRC
    f32_cond1
    f32_cond2
    f32_cond3
    f32_cond4
    f32_cond5
    f32_cond6
    f32_math1
    f32_math2
    f64_math1
    f64_math2
    f64_math3
)

foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/orka-lang/okcc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ok -o ${ITEM}.exe
        COMMAND ./${ITEM}.exe > output.txt
        COMMAND rm ${ITEM}.exe
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.ok"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test_orka_float
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_orka_float okcc)

