cmake_minimum_required(VERSION 3.20)

project(
    CppAudioEngine
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(portaudio CONFIG REQUIRED)

add_library(CppAudioEngineCore
    STATIC
        include/Audio/AudioEngine.h src/Audio/AudioEngine.cpp
        include/Audio/StreamData.h
        include/Audio/AudioData.h
)

target_link_libraries(CppAudioEngineCore
    PRIVATE
        portaudio_static
)

target_include_directories(CppAudioEngineCore
    PUBLIC
        src
        include
)



add_executable(AudioEngine
    src/main.cpp
)

target_link_libraries(AudioEngine
    PRIVATE
        CppAudioEngineCore
)