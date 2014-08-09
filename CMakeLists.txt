cmake_minimum_required(VERSION 2.8)
project(json_archive)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++1y -g -O0")
find_package(Boost 1.48.0 REQUIRED)
add_executable(json_archive test.cpp json_oarchive.cpp json_iarchive.cpp json_archive.cpp)
target_link_libraries(json_archive  boost_serialization jsoncpp)

