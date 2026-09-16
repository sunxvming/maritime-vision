cmake_minimum_required(VERSION 3.16)

find_package(Qt5 COMPONENTS TextToSpeech QUIET)

add_library(core_tts STATIC
    ttsengine.h
    qtttsengine.h
    qtttsengine.cpp
    ttsmanager.h
    ttsmanager.cpp
)

if(Qt5TextToSpeech_FOUND)
    target_link_libraries(core_tts PUBLIC
        Qt5::Core
        Qt5::TextToSpeech
    )
    target_compile_definitions(core_tts PUBLIC HAVE_QT_TTS)
else()
    message(STATUS "Qt5TextToSpeech not found — TTS will be silently disabled")
    target_link_libraries(core_tts PUBLIC Qt5::Core)
endif()

target_include_directories(core_tts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
