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
