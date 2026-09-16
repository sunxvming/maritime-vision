cmake_minimum_required(VERSION 3.16)

# core_webview 库
add_library(core_webview STATIC
    webjsdata.h
    webjsdata.cpp
    webview.h
    webview.cpp
    # miniblink支持（webminiblink 宏定义时需要）
    wke.h
    miniblink.h
    miniblink.cpp
)

# miniblink支持
add_definitions(-Dsavehtmlfile)

# WebKit支持
if(DEFINED webkit)
    # Unix系统需要保存为本地文件
    if(UNIX)
        add_definitions(-Dsavehtmlfile)
    endif()
endif()

target_link_libraries(core_webview PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    ${QT_WEB_LIBS}
)

target_include_directories(core_webview PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
