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

include(CTest)
if(BUILD_TESTING)
    add_executable(course_extractor_test
        tests/course_extractor_test.cpp
        extractor_storage.cpp
    )
    target_include_directories(course_extractor_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME course_extractor COMMAND course_extractor_test)
endif()

