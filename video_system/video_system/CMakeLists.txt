cmake_minimum_required(VERSION 3.16)

project(video_system)

# 收集所有源文件
file(GLOB_RECURSE MAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

file(GLOB_RECURSE MAIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB_RECURSE MAIN_FORMS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

# 主资源文件
set(MAIN_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qrc/main.qrc
)

# 预编译头文件
set(PRECOMPILE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/head.h
)

# 创建可执行文件
add_executable(video_system
    main.cpp
    ${MAIN_SOURCES}
    ${MAIN_HEADERS}
    ${MAIN_FORMS}
    ${MAIN_RESOURCES}
)

# 设置预编译头文件
if(MSVC)
    target_precompile_headers(video_system PRIVATE ${PRECOMPILE_HEADER})
endif()

# Windows资源文件
if(WIN32)
    target_sources(video_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qrc/main.rc)
endif()

# 包含目录
target_include_directories(video_system PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/class
    ${CMAKE_CURRENT_SOURCE_DIR}/class/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/class/api
    ${CMAKE_CURRENT_SOURCE_DIR}/class/app
    ${CMAKE_CURRENT_SOURCE_DIR}/class/devicevideo
    ${CMAKE_CURRENT_SOURCE_DIR}/class/usercontrol
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmipc
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmmain
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmvideo
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmconfig
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmmodule
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmalarm
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmalgorithm
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/frmscene
)

# 链接所有核心库
target_link_libraries(video_system PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    Qt5::Xml
    Qt5::Concurrent
    ${QT_WEB_LIBS}
    core_audio
    core_common
    core_utils
    core_control
    core_dataout
    core_db
    core_form
    core_onvif
    core_qui
    core_video
    core_videobase
    core_videoffmpeg
    core_videoopengl
    core_videosave
    core_webview
    core_tts
)

# Windows平台额外链接
if(WIN32 AND DEFINED webminiblink)
    target_link_libraries(video_system PRIVATE ${EXTRA_LIBS})
endif()

# 设置输出目录
# set_target_properties(video_system PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
# )
