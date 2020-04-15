if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    include(modules/yoloV3_voc_mod/CMakeLists.txt)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
endif()