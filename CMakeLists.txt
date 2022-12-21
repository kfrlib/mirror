cmake_minimum_required(VERSION 3.16)

project(mirror CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mirror example.cpp include/mirror.hpp)

target_include_directories(mirror PRIVATE include)
