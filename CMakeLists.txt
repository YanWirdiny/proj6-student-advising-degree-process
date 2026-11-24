cmake_minimum_required(VERSION 3.15)
project(student_advising)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(advisor
    src/main.cpp
    src/seeds.cpp
    src/fetcher.cpp
    src/parser.cpp
    src/politeness.cpp
    src/extractor_storage.cpp
    src/requirements.cpp
)

find_package(CURL REQUIRED)
target_link_libraries(advisor CURL::libcurl)

# ---- Tests ----
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(seeds_test
    test/seeds_test.cpp
    src/seeds.cpp
)
target_include_directories(seeds_test PRIVATE src)
target_link_libraries(seeds_test GTest::gtest_main)

add_executable(course_extractor_test
    test/course_extractor_test.cpp
    src/extractor_storage.cpp
)
target_include_directories(course_extractor_test PRIVATE src)
target_link_libraries(course_extractor_test GTest::gtest_main)

add_executable(course_code_test
    test/course_code_test.cpp
    src/course_code.cpp
)
target_include_directories(course_code_test PRIVATE src)
target_link_libraries(course_code_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(seeds_test)
gtest_discover_tests(course_extractor_test)
gtest_discover_tests(course_code_test)
