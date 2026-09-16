cmake_minimum_required(VERSION 3.16)

# core_videoopengl 库
add_library(core_videoopengl STATIC
    openglinclude.h
)

# 根据是否定义openglx来决定使用OpenGL还是普通widget
if(openglx)
    target_sources(core_videoopengl PRIVATE
        rgbopenglwidget.h
        rgbopenglwidget.cpp
        yuvopenglwidget.h
        yuvopenglwidget.cpp
        nv12openglwidget.h
        nv12openglwidget.cpp
    )

    find_package(OpenGL REQUIRED)
    target_link_libraries(core_videoopengl PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::Xml
        Qt5::Network
        Qt5::PrintSupport
        OpenGL::GL
        OpenGL::GLU
    )
else()
    target_sources(core_videoopengl PRIVATE
        rgbwidget.h
        rgbwidget.cpp
        yuvwidget.h
        yuvwidget.cpp
        nv12widget.h
        nv12widget.cpp
    )

    target_link_libraries(core_videoopengl PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::Xml
        Qt5::Network
        Qt5::PrintSupport
    )
endif()

target_include_directories(core_videoopengl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
