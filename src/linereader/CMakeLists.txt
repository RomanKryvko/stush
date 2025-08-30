cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)

target_sources(stush PRIVATE
    cmd.cpp
    terminal.cpp
)
