cmake_minimum_required(VERSION 3.10)
project(LabelingMachine)

add_subdirectory(modules)

set(CMAKE_CXX_STANDARD 14)



#Qt自动生成选项
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#添加源文件
file(GLOB_RECURSE SOURCES
        "Inc/*.h"
        "Src/*.*"
        "Resource.qrc"
        "LabelingMachine_zh_CN.ts"
        )
add_executable(LabelingMachine ${SOURCES})

#添加include路径
target_include_directories(LabelingMachine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ./Inc
        ${CMAKE_CURRENT_BINARY_DIR}/LabelingMachine_autogen/include)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O1")
    target_link_libraries(LabelingMachine pthread)
    #添加多线程库
    set(CMAKE_PREFIX_PATH /usr/local/Qt_5.14.0/5.14.0/gcc_64/lib/cmake)
elseif (WIN32)
    #设置Qt库
    set(CMAKE_PREFIX_PATH D:/Qt/5.14.0/msvc2017_64/lib/cmake)
    set(OpenCV_DIR D:/Library/opencv/nmake_build/install)
endif ()

#设置Qt库
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5Network)
qt5_use_modules(LabelingMachine Core Widgets Gui Xml Network)

#OpenCV
find_package(OpenCV 4.1.1 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(LabelingMachine ${OpenCV_LIBS})