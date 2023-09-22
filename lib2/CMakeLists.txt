cmake_minimum_required(VERSION 3.0.0)
project(tinylang_lib)

set(SRC
    fs.cpp
    io.cpp
    num.cpp
    str.cpp
)

add_library(tinylang SHARED ${SRC})
