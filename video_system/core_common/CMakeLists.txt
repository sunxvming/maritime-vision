cmake_minimum_required(VERSION 3.16)

# core_common 库
add_library(core_common STATIC
    base64helper.h
    base64helper.cpp
    commonnav.h
    commonnav.cpp
    commonstyle.h
    commonstyle.cpp
    framelesswidget2.h
    framelesswidget2.cpp
    playwav.h
    playwav.cpp
    savelog.h
    savelog.cpp
    saveruntime.h
    saveruntime.cpp
)

# 检查是否有multimedia模块
find_package(Qt5Multimedia QUIET)
if(Qt5Multimedia_FOUND)
    target_link_libraries(core_common PUBLIC Qt5::Multimedia)
    add_definitions(-Dplaysound)
endif()

target_link_libraries(core_common PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

target_include_directories(core_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
