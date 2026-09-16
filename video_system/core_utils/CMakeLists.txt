cmake_minimum_required(VERSION 3.16)


# core_utils 库
add_library(core_utils STATIC
    HttpClient.cpp
)

target_link_libraries(core_utils PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

target_include_directories(core_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
