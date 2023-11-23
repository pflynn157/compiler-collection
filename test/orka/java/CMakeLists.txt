set(EO_TEST_SRC
    test1
    imath ilogic
    class1
)

## LLVM testing
foreach(ITEM ${EO_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.class
        COMMAND ${CMAKE_BINARY_DIR}/orka-lang/okjc ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ok
        COMMAND java ${ITEM} > output.txt
        COMMAND rm ${ITEM}.class
        COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/out/${ITEM}.out ./output.txt
        COMMAND rm output.txt
        COMMAND echo "[PASS] ${ITEM}.ok.java"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.class
    )
endforeach()

add_custom_target(test_orka_java
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_orka_java okjc)

