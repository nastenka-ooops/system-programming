cmake_minimum_required(VERSION 3.24)
project(lapa1)

set(CMAKE_CXX_STANDARD 17)

add_executable(lapa1 main.cpp)
set_target_properties(lapa1 PROPERTIES
        COMPILE_DEFINITIONS "WIN32"
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )

target_link_libraries(lapa1 Gdiplus)
