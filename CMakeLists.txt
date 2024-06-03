cmake_minimum_required(VERSION 3.20)
project(rtcheck)

set(CMAKE_CTEST_ARGUMENTS "--build-and-test;${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR};--build-generator;${CMAKE_GENERATOR};--test-command;ctest")
enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(src)
add_subdirectory(tests)
