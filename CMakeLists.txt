cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)

project(stush VERSION 0.0.1)
add_executable(stush)

target_include_directories(stush PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_sources(stush PRIVATE
    src/main.cpp
    src/parser.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
