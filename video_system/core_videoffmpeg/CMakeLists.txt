cmake_minimum_required(VERSION 3.16)

# core_videoffmpeg 库
add_library(core_videoffmpeg STATIC
    ffmpeginclude.h
    ffmpegstruct.h
    ffmpeghelper.h
    ffmpeghelper.cpp
    ffmpegfilter.h
    ffmpegfilter.cpp
    ffmpegrun.h
    ffmpegrun.cpp
    ffmpegrunthread.h
    ffmpegrunthread.cpp
    # 视频保存功能（videosave 宏定义时需要）
    ffmpegsave.h
    ffmpegsave.cpp
    # videoffmpeg 功能
    ffmpegsync.h
    ffmpegsync.cpp
    ffmpegthread.h
    ffmpegthread.cpp
)

# 确定FFmpeg版本路径
set(FFMPEG_VERSION "ffmpeg4")
if(DEFINED ffmpeg5)
    set(FFMPEG_VERSION "ffmpeg5")
elseif(DEFINED ffmpeg3)
    set(FFMPEG_VERSION "ffmpeg3")
elseif(DEFINED ffmpeg2)
    set(FFMPEG_VERSION "ffmpeg2")
endif()

# 确定库路径（根据平台和架构）
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/winlib64")
    else()
        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/winlib")
    endif()
elseif(APPLE)
    set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/maclib64")
elseif(UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/linuxlib64")
    else()
        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/linuxlib")
    endif()
endif()

# FFmpeg头文件路径
target_include_directories(core_videoffmpeg PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_VERSION}/include
)

# FFmpeg库链接
target_link_directories(core_videoffmpeg PUBLIC ${FFMPEG_LIB_DIR})

target_link_libraries(core_videoffmpeg PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    avformat
    avcodec
    avfilter
    swscale
    avutil
    swresample
    avdevice
)

# 引入本地设备支持
add_definitions(-Dffmpegdevice)

# Linux特定链接
if(UNIX AND NOT APPLE)
    target_link_libraries(core_videoffmpeg PUBLIC
        postproc
        x264
        x265
    )
    # 设置RPATH以便在可执行文件目录查找动态库
    set_target_properties(core_videoffmpeg PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
