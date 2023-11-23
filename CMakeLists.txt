cmake_minimum_required(VERSION 3.15)
project(laado_lang)

find_package(LLVM REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

include_directories(compiler)
include_directories(compiler/java)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})

option(DISABLE_OUTPUT_TEST "Disable output-based testing" OFF)
if (DISABLE_OUTPUT_TEST)
    message(STATUS "Output-based testing disabled.")
endif()

option(DEV_LINK_MODE "Enable development linking" ON)
if (DEV_LINK_MODE)
    add_definitions(-DDEV_LINK_MODE)
    message(STATUS "Using development linking.")
endif()

# Build supporting projects
add_subdirectory(as)
add_subdirectory(compiler)
add_subdirectory(runtime)

# Build the core compiler
add_subdirectory(riya-lang)
add_subdirectory(orka-lang)
add_subdirectory(test)

