cmake_minimum_required(VERSION 3.15)
project(GoogologyLib VERSION 0.1.1 LANGUAGES CXX)

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
  src/notations/real_sequence/ns/Ns.cpp
  src/notations/ordinal/matrix/bms/BMS.cpp
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

add_executable(test_ns tests/unit/test_ns.cpp)
target_link_libraries(test_ns PRIVATE googology)

# Standard-form (§12) decision — header-only (merged into OrdinalNotation.hpp; uses Ordinal.hpp),
# scoped to ORDINAL notations; compiled straight from the test .cpp.
add_executable(test_standard_form tests/unit/test_standard_form.cpp)
target_include_directories(test_standard_form PRIVATE include)
target_link_libraries(test_standard_form PRIVATE googology)

enable_testing()
add_test(NAME test_knuth COMMAND test_knuth)
add_test(NAME test_conway COMMAND test_conway)
add_test(NAME test_prss COMMAND test_prss)
add_test(NAME test_epspss COMMAND test_epspss)
add_test(NAME test_weakveblen COMMAND test_weakveblen)
add_test(NAME test_ns COMMAND test_ns)
add_test(NAME test_standard_form COMMAND test_standard_form)

# BMS (Bashicu Matrix System) — 序数记号矩阵。BM4 为唯一完整规则，
# BM1 / BM3.3 为差异子类；其余版本（a_{k,m} 公式仅见于 basmat 源码）
# 头注释标明待补，暂不接入。
add_executable(test_bms tests/unit/test_bms.cpp)
target_link_libraries(test_bms PRIVATE googology)
add_test(NAME test_bms COMMAND test_bms)
