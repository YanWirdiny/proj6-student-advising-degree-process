cmake_minimum_required(VERSION 3.15)
project(student_advising)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# nlohmann/json: reads the JSON files the scraper wrote (header-only)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# ---- Backend (src/*): the advising logic. Never includes scraper code. ----
file(GLOB BACKEND_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(backend ${BACKEND_SOURCES})
target_include_directories(backend PUBLIC src)
target_link_libraries(backend PRIVATE nlohmann_json::nlohmann_json)

# ---- Scraper (src/scraper/*): fetches the UMB pages and writes the JSON files ----
file(GLOB SCRAPER_SOURCES CONFIGURE_DEPENDS src/scraper/*.cpp)
list(REMOVE_ITEM SCRAPER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/scraper/main.cpp)
add_library(scraper_lib ${SCRAPER_SOURCES})
target_include_directories(scraper_lib PUBLIC src/scraper src)

find_package(CURL REQUIRED)
target_link_libraries(scraper_lib PUBLIC CURL::libcurl)

add_executable(scraper src/scraper/main.cpp)
target_link_libraries(scraper scraper_lib)

# ---- Tests ----
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# Backend tests: test/*_test.cpp, each its own executable linked to the backend
file(GLOB BACKEND_TESTS CONFIGURE_DEPENDS test/*_test.cpp)
foreach(test_src ${BACKEND_TESTS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} backend GTest::gtest_main)
    # lets tests open the real JSON files at the project root
    target_compile_definitions(${test_name} PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
    gtest_discover_tests(${test_name})
endforeach()

# Scraper tests: test/scraper/*_test.cpp, linked to the scraper library
file(GLOB SCRAPER_TESTS CONFIGURE_DEPENDS test/scraper/*_test.cpp)
foreach(test_src ${SCRAPER_TESTS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} scraper_lib GTest::gtest_main)
    gtest_discover_tests(${test_name})
endforeach()
