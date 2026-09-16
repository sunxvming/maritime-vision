cmake_minimum_required(VERSION 3.16)

# core_videobase 库
add_library(core_videobase STATIC
    widgethead.h
    widgetstruct.h
    widgethelper.h
    widgethelper.cpp
    urlhelper.h
    urlhelper.cpp
    filterhelper.h
    filterhelper.cpp
    bannerwidget.h
    bannerwidget.cpp
    abstractvideothread.h
    abstractvideothread.cpp
    abstractvideowidget.h
    abstractvideowidget.cpp
    abstractsavethread.h
    abstractsavethread.cpp
    imagelabel.h
    imagelabel.cpp
)

# 注意：audioplayer.cpp 已在 core_audio 中定义，此处不再包含

target_link_libraries(core_videobase PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

# FFmpeg版本路径（与 core_videoffmpeg 保持一致）
set(FFMPEG_VERSION "ffmpeg4")
if(DEFINED ffmpeg5)
    set(FFMPEG_VERSION "ffmpeg5")
elseif(DEFINED ffmpeg3)
    set(FFMPEG_VERSION "ffmpeg3")
elseif(DEFINED ffmpeg2)
    set(FFMPEG_VERSION "ffmpeg2")
endif()

target_include_directories(core_videobase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core_videoffmpeg
    ${CMAKE_SOURCE_DIR}/core_videoffmpeg/${FFMPEG_VERSION}/include
)

add_definitions(-Dimagelabel)
