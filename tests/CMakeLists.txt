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
    ${PROJECT_SOURCE_DIR}/src/linereader/utfstring.cpp
)

target_link_libraries(
    linebuffer_test
    GTest::gtest_main
)

add_executable(utf8utils_test utf8utils_test.cpp)
target_include_directories(utf8utils_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
    utf8utils_test
    GTest::gtest_main
)

add_executable(utfstring_test)
target_include_directories(utfstring_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(utfstring_test  PRIVATE
    utfstring_test.cpp
    ${PROJECT_SOURCE_DIR}/src/linereader/utfstring.cpp
)

target_link_libraries(
    utfstring_test
    GTest::gtest_main
)

add_executable(shell_expansion_test)
target_include_directories(shell_expansion_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(shell_expansion_test  PRIVATE
    shell_expansion_test.cpp
    ${PROJECT_SOURCE_DIR}/src/cmd/expansion.cpp
    ${PROJECT_SOURCE_DIR}/src/cmd/variable.cpp
)

target_link_libraries(
    shell_expansion_test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(parser_test)
gtest_discover_tests(byteutils_test)
gtest_discover_tests(linebuffer_test)
gtest_discover_tests(utfstring_test)
gtest_discover_tests(utf8utils_test)
gtest_discover_tests(shell_expansion_test)
