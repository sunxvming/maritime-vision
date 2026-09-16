cmake_minimum_required(VERSION 3.16)

# core_db 库
add_library(core_db STATIC
    dbhead.h
    dbhelper.h
    dbhelper.cpp
    dbcleanthread.h
    dbcleanthread.cpp
    dbconnthread.h
    dbconnthread.cpp
    dbdelegate.h
    dbdelegate.cpp
    dbhttpthread.h
    dbhttpthread.cpp
    dbpage.h
    dbpage.cpp
    dbpagemodel.h
    dbpagemodel.cpp
    dbtablemodel.h
    dbtablemodel.cpp
    navpage.h
    navpage.cpp
)

target_link_libraries(core_db PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Sql
    Qt5::Widgets
    Qt5::Network
    Qt5::Xml
    core_qui
)

target_include_directories(core_db PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core_qui
    ${CMAKE_SOURCE_DIR}/core_qui/core
)
