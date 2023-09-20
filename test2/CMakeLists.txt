set(TEST_SRC
    test1
    test2
    test3
)

foreach(ITEM ${TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/src/tlc ${CMAKE_CURRENT_SOURCE_DIR}/core/${ITEM}.ry -o ${ITEM}.exe
        COMMAND ./${ITEM}.exe
        COMMAND rm ${ITEM}.exe
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test tlc)

