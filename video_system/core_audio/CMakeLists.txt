cmake_minimum_required(VERSION 3.16)

# core_audio 库
add_library(core_audio STATIC
    audioinclude.h
    audiohelper.h
    audiohelper.cpp
    audiolevel.h
    audiolevel.cpp
    audioplot.h
    audioplot.cpp
    audioplayer.h
    audioplayer.cpp
    audiorecorder.h
    audiorecorder.cpp
)

target_link_libraries(core_audio PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    Qt5::Multimedia
)

target_include_directories(core_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
