cmake_minimum_required(VERSION 3.31)
project(ver4)

set(CMAKE_CXX_STANDARD 20)

add_executable(ver4 main.cpp MIDIexample.cpp)

target_link_libraries(ver4 winmm)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")