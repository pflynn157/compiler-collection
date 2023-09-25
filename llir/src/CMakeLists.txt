project(llir_src)

set(AMD64_SRC
    amd64/amd64.cpp
    amd64/x86ir.cpp
)

set(SRC
    ${AMD64_SRC}
    irbuilder.cpp
    llir.cpp
    print.cpp
    transform.cpp
)

add_library(llir STATIC ${SRC})
