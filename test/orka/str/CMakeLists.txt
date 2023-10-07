set(CORE_TEST_SRC
    str1
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

add_custom_target(test_orka_str
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_orka_str okcc)

