cmake_minimum_required(VERSION 3.15)
project(student_advising)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(advisor
    main.cpp
    seeds.cpp
    fetcher.cpp
    parser.cpp
    politeness.cpp
    extractor_storage.cpp
    requirements.cpp
)

find_package(CURL REQUIRED)
target_link_libraries(advisor CURL::libcurl)
