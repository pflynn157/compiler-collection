cmake_minimum_required(VERSION 3.10)
project(llir)

add_compile_options(-std=c++17 -g)

include_directories(src compiler)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(compiler)
