cmake_minimum_required(VERSION 3.16)

# core_videosave 库
add_library(core_videosave STATIC
    saveinclude.h
    savehelper.h
    savehelper.cpp
    saveaudio.h
    saveaudio.cpp
    savevideo.h
    savevideo.cpp
)

target_link_libraries(core_videosave PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    core_videobase
)

target_include_directories(core_videosave PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
