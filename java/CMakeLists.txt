project(java)

include_directories(.)

set(SRC
    Java/JavaBuilder.cpp
    Java/JavaWriter.cpp
    Java/JavaCode.cpp
    
    JavaCompiler.cpp
    JavaUtils.cpp
)

add_library(coffee-maker STATIC ${SRC})

