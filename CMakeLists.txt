
cmake_minimum_required(VERSION 3.10)

#set(CMAKE_CXX_COMPILER "/pathto/g++")
#set(CMAKE_C_COMPILER "/pathto/gcc")

project(StudProg)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "src/*.cpp")
add_executable(StudProg ${SOURCES})

target_include_directories(StudProg
    PRIVATE include
)
