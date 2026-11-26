cmake_minimum_required(VERSION 3.15)
project(student_advising)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Writes build/compile_commands.json so the editor (clangd) finds gtest, httplib and json headers.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# ---- Web server (src/server/*): the "advisor" program, HTTP API over the backend ----
# cpp-httplib: single-header HTTP server. Plain HTTP on localhost only, so no TLS/compression.
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE    OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE  OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(httplib)

file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS src/server/*.cpp)
list(REMOVE_ITEM SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/server/main.cpp)
add_library(server_lib ${SERVER_SOURCES})
target_include_directories(server_lib PUBLIC src/server)
target_link_libraries(server_lib PUBLIC backend nlohmann_json::nlohmann_json httplib::httplib)

add_executable(advisor src/server/main.cpp)
target_link_libraries(advisor server_lib)

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

# Server tests: test/server/*_test.cpp, linked to the server library
file(GLOB SERVER_TESTS CONFIGURE_DEPENDS test/server/*_test.cpp)
foreach(test_src ${SERVER_TESTS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} server_lib GTest::gtest_main)
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
