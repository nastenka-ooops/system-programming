cmake_minimum_required(VERSION 3.29)
project(sound_mixer)

set(CMAKE_CXX_STANDARD 20)

add_executable(sound_mixer main.cpp)

target_link_libraries(sound_mixer winmm)

