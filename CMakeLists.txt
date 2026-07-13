cmake_minimum_required(VERSION 3.15)
project(GoogologyLib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: larger integer type for parse parameters / expansion indices
# (e.g. arrow height beyond 2^63). NOTE: the library never computes numeric
# values for a notation regardless of this setting (see doc/design.md).
#   option(GOOGOLOGY_USE_BOOST "Use boost::multiprecision::cpp_int" OFF)
#   if(GOOGOLOGY_USE_BOOST)
#     find_package(Boost REQUIRED)
#     add_compile_definitions(GOOGOLOGY_USE_BOOST)
#   endif()

add_library(googology
  src/notations/number/knuth/Knuth.cpp
  src/notations/number/conway/Conway.cpp
  src/notations/ordinal/sequence/difference/prss/Prss.cpp
  src/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.cpp
  src/notations/ordinal/veblen/weakveblen/WeakVeblen.cpp
)
target_include_directories(googology PUBLIC include)

add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE googology)

add_executable(test_knuth tests/unit/test_knuth.cpp)
target_link_libraries(test_knuth PRIVATE googology)

add_executable(test_conway tests/unit/test_conway.cpp)
target_link_libraries(test_conway PRIVATE googology)

add_executable(test_prss tests/unit/test_prss.cpp)
target_link_libraries(test_prss PRIVATE googology)

add_executable(test_epspss tests/unit/test_epspss.cpp)
target_link_libraries(test_epspss PRIVATE googology)

add_executable(test_weakveblen tests/unit/test_weakveblen.cpp)
target_link_libraries(test_weakveblen PRIVATE googology)

enable_testing()
add_test(NAME test_knuth COMMAND test_knuth)
add_test(NAME test_conway COMMAND test_conway)
add_test(NAME test_prss COMMAND test_prss)
add_test(NAME test_epspss COMMAND test_epspss)
add_test(NAME test_weakveblen COMMAND test_weakveblen)
