cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)

add_executable(yoloV3_voc_mod
        main.cpp
        ArgResolver.cpp
        ArgResolver.h
        )

if (CMAKE_BUILD_TOOL MATCHES nmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
elseif (CMAKE_BUILD_TOOL MATCHES gnu)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O1")
endif ()

find_package(tinyxml2)
target_link_libraries(yoloV3_voc_mod tinyxml2::tinyxml2)
