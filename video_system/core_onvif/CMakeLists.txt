cmake_minimum_required(VERSION 3.16)

# core_onvif 库
add_library(core_onvif STATIC
    onvif.qrc
    onvifhead.h
    onvifstruct.h
    onvifbase.h
    onvifbase.cpp
    onvifdevice.h
    onvifdevice.cpp
    onvifevent.h
    onvifevent.cpp
    onvifhelper.h
    onvifhelper.cpp
    onvifother.h
    onvifother.cpp
    onvifptz.h
    onvifptz.cpp
    onvifquery.h
    onvifquery.cpp
    onvifrequest.h
    onvifrequest.cpp
    onvifsearch.h
    onvifsearch.cpp
    onvifsnap.h
    onvifsnap.cpp
    onvifthread.h
    onvifthread.cpp
    onvifvideo.h
    onvifvideo.cpp
    onvifxml.h
    onvifxml.cpp
)

target_link_libraries(core_onvif PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
    Qt5::Xml
    Qt5::PrintSupport
)

target_include_directories(core_onvif PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
