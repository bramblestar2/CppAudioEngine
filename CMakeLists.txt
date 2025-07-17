cmake_minimum_required(VERSION 3.20)

project(
    CppAudioEngine
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(portaudio CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(Aubio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-libmagic REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(SampleRate CONFIG REQUIRED)

add_library(CppAudioEngineCore
    STATIC
        include/Audio/AudioEngine.h src/Audio/AudioEngine.cpp
        include/Audio/StreamData.h
        include/Audio/AudioBuffer.h src/Audio/AudioBuffer.cpp
        include/Audio/AudioManager.h src/Audio/AudioManager.cpp
        include/Audio/AudioPlayer.h src/Audio/AudioPlayer.cpp
        include/Audio/Clip.h src/Audio/Clip.cpp
        include/Audio/Mixer.h src/Audio/Mixer.cpp

        include/Utility/IdPool.h src/Utility/IdPool.cpp
)

target_link_libraries(CppAudioEngineCore
    PRIVATE
        SndFile::sndfile
        Aubio::aubio
        spdlog::spdlog
        unofficial::libmagic::libmagic
        unofficial::sqlite3::sqlite3
        SampleRate::samplerate
        $<$<AND:$<BOOL:UNIX>,$<NOT:$<BOOL:APPLE>>>:portaudio_static>
        $<$<BOOL:WIN32>:portaudio>
)

target_include_directories(CppAudioEngineCore
    PUBLIC
        src
        include
)

target_compile_definitions(CppAudioEngineCore PRIVATE MAGIC_DB_PATH="${unofficial-libmagic_DICTIONARY}")




add_executable(AudioEngine
    src/main.cpp
)

target_link_libraries(AudioEngine
    PRIVATE
        CppAudioEngineCore
        spdlog::spdlog
)