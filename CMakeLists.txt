cmake_minimum_required(VERSION 3.3.2)
project(DistributedMonitor)
#find_package(FindZeroMQ REQUIRED)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES proxy/main.cpp)

include_directories("/usr/local/include")
include_directories("${PROJECT_BINARY_DIR}/client")
#add_subdirectory("${PROJECT_SOURCE_DIR}/client")
SET(GCC_COVERAGE_LINK_FLAGS    "-lzmq -lpthread -lboost_serialization")

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
add_executable(DistributedMonitor ${SOURCE_FILES})

set(SOURCE_FILES client/client.cpp client/client.h client/consumer.h client/producer.h client/spinbuf.h)
add_executable(Client ${SOURCE_FILES})

set(SOURCE_FILES client/philosopher.cpp client/philosopher.h)
add_executable(Philo ${SOURCE_FILES})
