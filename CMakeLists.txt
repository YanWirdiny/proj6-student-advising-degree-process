<<<<<<< Updated upstream
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
)

find_package(CURL REQUIRED)
target_link_libraries(advisor CURL::libcurl)
=======

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_COMPILER "/pathto/g++")
set(CMAKE_C_COMPILER "/pathto/gcc")

project(StudentAdvisingAndProgress)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "src/*.cpp")
add_executable(StudentAdvisingAndProgress ${SOURCES})

target_include_directories(StudentAdvisingAndProgress
    PRIVATE include
)
>>>>>>> Stashed changes
