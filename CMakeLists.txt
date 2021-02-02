cmake_minimum_required(VERSION 3.16)
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
        "Resource.qrc")

add_executable(LabelingMachine ${SOURCES})

#添加include路径
target_include_directories(LabelingMachine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ./Inc)

if (CMAKE_BUILD_TYPE MATCHES Release)
    # Remove the console of gui program
    if (WIN32)
        if (MSVC)
            set_target_properties(${PROJECT_NAME} PROPERTIES
                    WIN32_EXECUTABLE YES
                    LINK_FLAGS "/ENTRY:mainCRTStartup"
                    )
        elseif (CMAKE_COMPILER_IS_GNUCXX)
            # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
        else ()
            message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
        endif (MSVC)
    elseif (APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
                MACOSX_BUNDLE YES
                )
    elseif (UNIX)
        # Nothing special required
    else ()
        message(SEND_ERROR "You are on an unsupported platform! (Not Win32, Mac OS X or Unix)")
    endif (WIN32)
else ()
    target_compile_definitions(LabelingMachine PUBLIC __DEBUG__)
endif ()

if (CMAKE_BUILD_TOOL MATCHES nmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
elseif (CMAKE_BUILD_TOOL MATCHES gnu)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O1")
endif ()

#设置Qt库
find_package(Qt5 COMPONENTS Widgets LinguistTools Network Xml REQUIRED)
target_link_libraries(LabelingMachine PUBLIC Qt5::Widgets Qt5::Network Qt5::Xml)

#OpenCV
find_package(OpenCV 4.1.1 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(LabelingMachine PUBLIC ${OpenCV_LIBS})

#https://github.com/gabime/spdlog.git
#http://www.kdrobot.top/git/LX050724/KdrobotCppLibs.git
find_package(spdlog)
find_package(loggerFactory)
find_package(Qt_Util)
find_package(RobotCommSystem)
target_link_libraries(LabelingMachine PUBLIC spdlog::spdlog KdrobotCppLibs::loggerFactory KdrobotCppLibs::RobotCommSystem)
