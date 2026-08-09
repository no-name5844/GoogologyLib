cmake_minimum_required(VERSION 3.10)
project(googology C)

set(CMAKE_C_STANDARD 11)

# Library
add_library(googology
    src/core/capability.c
    src/core/notation.c
    src/notations/number/knuth.c
    src/notations/number/conway.c
)

target_include_directories(googology PUBLIC include)

# Test
add_executable(test_googology
    tests/test_all.c
)
target_link_libraries(test_googology googology)
target_include_directories(test_googology PRIVATE include)