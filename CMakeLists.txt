cmake_minimum_required(VERSION 3.7)
project(DistributedMonitor)
#find_package(FindZeroMQ REQUIRED)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES proxy/main.cpp)
include_directories("/usr/local/include")
SET(GCC_COVERAGE_LINK_FLAGS    "-lzmq")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
add_executable(DistributedMonitor ${SOURCE_FILES})