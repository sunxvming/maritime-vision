cmake_minimum_required(VERSION 3.16)

# core_dataout 库
add_library(core_dataout STATIC
    datahead.h
    datastruct.h
    datacsv.h
    datacsv.cpp
    datahelper.h
    datahelper.cpp
    dataother.h
    dataother.cpp
    dataprint.h
    dataprint.cpp
    datareport.h
    datareport.cpp
    dataxls.h
    dataxls.cpp
)

target_link_libraries(core_dataout PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

target_include_directories(core_dataout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
