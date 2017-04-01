cmake_minimum_required(VERSION 3.7)
project(DistributedMonitor)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(DistributedMonitor ${SOURCE_FILES})