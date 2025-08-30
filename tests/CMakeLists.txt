cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)

add_executable(parser_test)

target_include_directories(parser_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(parser_test PRIVATE
    parser_test.cpp
    ${PROJECT_SOURCE_DIR}/src/parser.cpp
)

target_link_libraries(
    parser_test
    GTest::gtest_main
)

add_executable(byteutils_test byteutils_test.cpp)
target_include_directories(byteutils_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
    byteutils_test
    GTest::gtest_main
)

add_executable(linebuffer_test)
target_include_directories(linebuffer_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(linebuffer_test  PRIVATE
    linebuffer_test.cpp
    ${PROJECT_SOURCE_DIR}/src/linereader/linebuffer.cpp
)

target_link_libraries(
    linebuffer_test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(parser_test)
gtest_discover_tests(byteutils_test)
gtest_discover_tests(linebuffer_test)
