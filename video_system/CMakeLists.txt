cmake_minimum_required(VERSION 3.16)

project(video_system VERSION 1.0.0 LANGUAGES CXX C)

# Qt设置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 输出目录设置
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 编译选项

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    #禁用qdebug打印输出 
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()
add_definitions(-DQT_MESSAGELOGCONTEXT)

# 功能模块开关
option(openglx "Enable OpenGL rendering" ON)
option(audiox "Enable audio support" ON)
option(videosave "Enable video save" ON)
option(ffmpeg "Enable ffmpeg" ON)

add_definitions(-Daudiox)
if(openglx)
    add_definitions(-Dopenglx)
endif()
add_definitions(-Dvideosave)
add_definitions(-Dffmpeg)
add_definitions(-Dffmpeg4)
add_definitions(-Dvideoffmpeg)
add_definitions(-Dquistyle)

# Windows平台特定设置
if(WIN32)
    add_compile_options(/utf-8)
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)
    # 关闭所有警告
    add_compile_options(/W0)
endif()

# Unix平台特定设置
if(UNIX AND NOT APPLE)
    add_compile_options(-Wno-narrowing -fpermissive)
    add_compile_options(-w)  # 关闭所有警告
endif()

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
    Xml
    Concurrent
    Multimedia
    PrintSupport
)

# WebKit/WebEngine支持检测
find_package(Qt5WebKitWidgets QUIET)
if(Qt5WebKitWidgets_FOUND)
    add_definitions(-Dwebkit)
    set(QT_WEB_LIBS Qt5::WebKitWidgets)
else()
    find_package(Qt5WebEngineWidgets QUIET)
    if(Qt5WebEngineWidgets_FOUND)
        add_definitions(-Dwebengine)
        set(QT_WEB_LIBS Qt5::WebEngineWidgets)
    else()
        message(STATUS "未找到浏览器模块，将使用第三方浏览器模块")
        if(WIN32)
            add_definitions(-Dwebminiblink)
            # miniblink需要user32库
            set(EXTRA_LIBS ${EXTRA_LIBS} user32)
        endif()
    endif()
endif()

# 包含目录设置
include_directories(${CMAKE_SOURCE_DIR}/video_system)
include_directories(${CMAKE_SOURCE_DIR}/video_system/class)
include_directories(${CMAKE_SOURCE_DIR}/video_system/class/api)
include_directories(${CMAKE_SOURCE_DIR}/video_system/class/app)
include_directories(${CMAKE_SOURCE_DIR}/video_system/class/devicevideo)
include_directories(${CMAKE_SOURCE_DIR}/video_system/class/usercontrol)
include_directories(${CMAKE_SOURCE_DIR}/video_system/ui/frmmain)
# 添加所有核心模块的包含路径（因为 head.h 被广泛使用）
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/core_audio)
include_directories(${CMAKE_SOURCE_DIR}/core_common)
include_directories(${CMAKE_SOURCE_DIR}/core_utils)
include_directories(${CMAKE_SOURCE_DIR}/core_control)
include_directories(${CMAKE_SOURCE_DIR}/core_dataout)
include_directories(${CMAKE_SOURCE_DIR}/core_db)
include_directories(${CMAKE_SOURCE_DIR}/core_form)
include_directories(${CMAKE_SOURCE_DIR}/core_onvif)
include_directories(${CMAKE_SOURCE_DIR}/core_qui)
include_directories(${CMAKE_SOURCE_DIR}/core_qui/core)
include_directories(${CMAKE_SOURCE_DIR}/core_qui/helper)
include_directories(${CMAKE_SOURCE_DIR}/core_qui/form)
include_directories(${CMAKE_SOURCE_DIR}/core_video)
include_directories(${CMAKE_SOURCE_DIR}/core_videobase)
include_directories(${CMAKE_SOURCE_DIR}/core_videoffmpeg)
include_directories(${CMAKE_SOURCE_DIR}/core_videoopengl)
include_directories(${CMAKE_SOURCE_DIR}/core_videosave)
include_directories(${CMAKE_SOURCE_DIR}/core_webview)
include_directories(${CMAKE_SOURCE_DIR}/core_tts)



# 添加核心模块子目录
add_subdirectory(core_audio)
add_subdirectory(core_common)
add_subdirectory(core_utils)
add_subdirectory(core_control)
add_subdirectory(core_dataout)
add_subdirectory(core_db)
add_subdirectory(core_form)
add_subdirectory(core_onvif)
add_subdirectory(core_qui)
add_subdirectory(core_video)
add_subdirectory(core_videobase)
add_subdirectory(core_videoffmpeg)
add_subdirectory(core_videoopengl)
add_subdirectory(core_videosave)
add_subdirectory(core_webview)
add_subdirectory(core_tts)

# 添加主应用程序
add_subdirectory(video_system)
