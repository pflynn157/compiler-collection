set(AS_TEST_SRC
    first
)

foreach(ITEM ${AS_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/as/asx86 ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.asm
        COMMAND ld a.out -o ${ITEM}.exe
        COMMAND as ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.asm -o ${ITEM}.o
        COMMAND ld ${ITEM}.o -o ${ITEM}.exe2
        COMMAND ./${ITEM}.exe > output1.txt
        COMMAND ./${ITEM}.exe2 > output2.txt
        COMMAND rm ${ITEM}.exe
        COMMAND rm ${ITEM}.exe2
        COMMAND diff ./output1.txt ./output2.txt
        COMMAND rm output1.txt
        COMMAND rm output2.txt
        COMMAND rm a.out
        COMMAND rm ${ITEM}.o
        COMMAND echo "[PASS] ${ITEM}.asm"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test_as
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_as asx86)


