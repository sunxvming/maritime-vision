cmake_minimum_required(VERSION 3.16)

# core_form 库
add_library(core_form STATIC
    frmconfigdb.ui
    frmconfiguser.ui
    frmlogin.ui
    frmlogout.ui
    formhelper.h
    formhelper.cpp
    frmconfigdb.h
    frmconfigdb.cpp
    frmconfiguser.h
    frmconfiguser.cpp
    frmlogin.h
    frmlogin.cpp
    frmlogout.h
    frmlogout.cpp
    userhelper.h
    userhelper.cpp
)

target_link_libraries(core_form PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    core_db
)

target_include_directories(core_form PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core_db
)
