set(EO_TEST_SRC
    test1
    Inner
    Inner2
    Inner3
    imath
)

## LLVM testing
foreach(ITEM ${EO_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.class
        COMMAND ${CMAKE_BINARY_DIR}/espresso-lang/eoc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.eo
        COMMAND java ${ITEM} > output.txt
        COMMAND rm ${ITEM}.class
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.eo"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.class
    )
endforeach()

add_custom_target(test_espresso
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_espresso eoc)

