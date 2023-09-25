cmake_minimum_required(VERSION 3.10)
project(llir_test)

add_executable(test1 test1.cpp)
target_link_libraries(test1 llir)

add_executable(test2 test2.cpp)
target_link_libraries(test2 llir)

add_executable(test3 test3.cpp)
target_link_libraries(test3 llir)
