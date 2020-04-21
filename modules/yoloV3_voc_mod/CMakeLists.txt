cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER /usr/bin/gcc)

add_compile_options(-O3)
add_link_options(-O1)

add_executable(yoloV3_voc_mod
        main.cpp
        ArgResolver.cpp
        ArgResolver.h
)

add_compile_options(-O3)
add_link_options(-O1)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
target_link_libraries(yoloV3_voc_mod tinyxml2)
