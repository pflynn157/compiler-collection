set(CORE_TEST_SRC
    test1 test2 test3
    print_add1
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

