set(CORE_TEST_SRC
    lex1
)

foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}_lex.out
        COMMAND ${CMAKE_BINARY_DIR}/src/tlc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry --test-lex > ${ITEM}_lex.out
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./${ITEM}_lex.out
        COMMAND rm ${ITEM}_lex.out
        COMMAND echo "[PASS] ${ITEM}.lex"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}_lex.out
    )
endforeach()

add_custom_target(test_lex
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_lex tlc)

