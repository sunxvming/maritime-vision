cmake_minimum_required(VERSION 3.16)

# core_qui 库
add_library(core_qui STATIC
    # 核心文件
    core/quihead.h
    core/quisingleton.h
    core/customstyle.h
    core/customstyle.cpp
    core/iconhelper.h
    core/iconhelper.cpp
    core/quiconfig.h
    core/quiconfig.cpp
    core/quistyle.h
    core/quistyle.cpp

    # 窗体文件
    form/quiabout.ui
    form/quiabout.h
    form/quiabout.cpp
    form/quidateselect.ui
    form/quidateselect.h
    form/quidateselect.cpp
    form/quiinputbox.ui
    form/quiinputbox.h
    form/quiinputbox.cpp
    form/quimessagebox.ui
    form/quimessagebox.h
    form/quimessagebox.cpp
    form/quisplash.ui
    form/quisplash.h
    form/quisplash.cpp
    form/quitipbox.ui
    form/quitipbox.h
    form/quitipbox.cpp
    form/quiwidget.ui
    form/quiwidget.h
    form/quiwidget.cpp

    # 辅助函数文件
    helper/quihelper.h
    helper/quihelper.cpp
    helper/quihelpercore.h
    helper/quihelpercore.cpp
    helper/quihelperdata.h
    helper/quihelperdata.cpp
    helper/quihelperfile.h
    helper/quihelperfile.cpp
    helper/quihelperform.h
    helper/quihelperform.cpp
    helper/quihelperimage.h
    helper/quihelperimage.cpp
    helper/quihelpernet.h
    helper/quihelpernet.cpp
    helper/quihelperother.h
    helper/quihelperother.cpp

    # 资源文件
    qrc/qss.qrc
    qrc/qm.qrc
    qrc/font.qrc
)

target_link_libraries(core_qui PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
)

target_include_directories(core_qui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/form
    ${CMAKE_CURRENT_SOURCE_DIR}/helper
)
