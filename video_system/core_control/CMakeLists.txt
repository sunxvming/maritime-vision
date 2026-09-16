cmake_minimum_required(VERSION 3.16)

# core_control 库
add_library(core_control STATIC
    control.qrc
    bottomwidget.h
    bottomwidget.cpp
    colorcombobox.h
    colorcombobox.cpp
    cpumemorylabel.h
    cpumemorylabel.cpp
    customtitlebar.h
    customtitlebar.cpp
    devicebutton.h
    devicebutton.cpp
    lcddatetime.h
    lcddatetime.cpp
    panelwidget.h
    panelwidget.cpp
    switchbutton.h
    switchbutton.cpp
    xslider.h
    xslider.cpp
)

target_link_libraries(core_control PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

target_include_directories(core_control PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
