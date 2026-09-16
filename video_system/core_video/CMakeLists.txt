cmake_minimum_required(VERSION 3.16)

# core_video 库
add_library(core_video STATIC
    videohead.h
    videostruct.h
    videourl.h
    videourl.cpp
    videobox.h
    videobox.cpp
    videoplayback.h
    videoplayback.cpp
    videohelper.h
    videohelper.cpp
    videothread.h
    videothread.cpp
    videowidgetx.h
    videowidgetx.cpp
    videomanage.h
    videomanage.cpp
    videotask.h
    videotask.cpp
)

target_link_libraries(core_video PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    core_videobase
)

target_include_directories(core_video PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
