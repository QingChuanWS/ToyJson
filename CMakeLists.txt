cmake_minimum_required (VERSION 2.6)
project (JSON_TOY)

# if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
# endif()

add_library(json_toy json_toy.cc json_toy_node.cc json_toy_basic.cc json_toy_enum.cc)
add_executable(json_toy_test test.cc)
target_link_libraries(json_toy_test json_toy)