cmake_minimum_required(VERSION 3.10)
add_subdirectory(yoloV3_voc_mod)

if (UNIX)
    #Linux only modules
elseif (WIN32)
    #Windows only modules
endif ()