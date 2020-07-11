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
set(ICON "")

if (MSVC)
    set(app_WINRC app_win32.rc)
    set_property(SOURCE app_win32.rc APPEND PROPERTY
            OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/bitbug_favicon.ico)
    set(ICON ${app_WINRC})
endif ()

#添加源文件
file(GLOB_RECURSE SOURCES
        "Inc/*.h"
        "Src/*.*"
        "Resource.qrc"
        "LabelingMachine_zh_CN.ts")

add_executable(LabelingMachine ${SOURCES} ${ICON})

#添加include路径
target_include_directories(LabelingMachine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ./Inc
        ${CMAKE_CURRENT_BINARY_DIR}/LabelingMachine_autogen/include)


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

if (CMAKE_BUILD_TOOL MATCHES nmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    #    set(FFMPEG_DIR D:/Library/ffmpeg-4.3-win64-dev)
    #
    #    target_link_directories(LabelingMachine PUBLIC "${FFMPEG_DIR}/lib")
    #    target_include_directories(LabelingMachine PUBLIC "${FFMPEG_DIR}/include")
    #    file(GLOB FFMPEG_LIBS "${FFMPEG_DIR}/lib/*.lib")
    #    target_link_libraries(LabelingMachine PUBLIC ${FFMPEG_LIBS})
elseif (CMAKE_BUILD_TOOL MATCHES gnu)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O1")
    target_link_libraries(LabelingMachine pthread)
endif ()

#设置Qt库
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5Network)
qt5_use_modules(LabelingMachine LINK_PUBLIC Core Widgets Gui Xml Network)

#OpenCV
find_package(OpenCV 4.1.1 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(LabelingMachine PUBLIC ${OpenCV_LIBS})