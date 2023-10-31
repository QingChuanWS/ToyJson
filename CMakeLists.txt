# 设置CMake的最低版本要求
cmake_minimum_required(VERSION 3.0)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

file(GLOB SRC_FILES "src/*.cc")

add_library(TJsonLib ${SRC_FILES})

add_subdirectory(test)