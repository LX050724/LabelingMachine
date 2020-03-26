cmake_minimum_required(VERSION 3.15)
project(LabelingMachine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER /usr/bin/gcc)

add_compile_options(-O3)
add_link_options(-O1)

#Qt自动生成选项
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#添加源文件
add_executable(${PROJECT_NAME}
    Src/aboutwindow.cpp
    Src/bandbox.cpp
    Src/bandboxview.cpp
    Src/classeditor.cpp
    Src/helpwindow.cpp
    Src/imagedata.cpp
    Src/labelingmaching.cpp
    Src/main.cpp
    Src/mainwindow.cpp
    Src/openvideo.cpp
    Inc/aboutwindow.h
    Inc/bandbox.h
    Inc/bandboxview.h
    Inc/classeditor.h
    Inc/helpwindow.h
    Inc/imagedata.h
    Inc/Labels.h
    Inc/labelingmaching.h
    Inc/mainwindow.h
    Inc/openvideo.h
    Src/aboutwindow.ui
    Src/classeditor.ui
    Src/helpwindow.ui
    Src/mainwindow.ui
    Src/openvideo.ui
    Src/TCP_Server.cpp
    Inc/TCP_Server.h
    Src/TCP_Client.cpp
    Inc/TCP_Client.h
    Src/ClientUI.cpp
    Src/ClientUI.ui
    Inc/ClientUI.h
    Src/ServerUI.cpp
    Src/ServerUI.ui
    Inc/ServerUI.h
    Src/module.cpp
    Inc/module.h
    Src/module.ui
    Resource.qrc
)

#添加include路径
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ./Inc)

#添加多线程库
target_link_libraries(${PROJECT_NAME} pthread)
#添加编译选项
add_compile_options(-pipe -O3)
add_link_options(-O2)

#设置Qt库
set(CMAKE_PREFIX_PATH /usr/local/Qt_5.14.0/5.14.0/gcc_64/lib/cmake)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5Network)
qt5_use_modules(${PROJECT_NAME} Core Widgets Gui Xml Network)

#OpenCV
find_package(OpenCV 4.1.1 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
