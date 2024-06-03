#cmake_minimum_required(VERSION 3.20)
#project(rtcheck)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#======================================
add_library(rtcheck SHARED
    rtcheck.cpp
)

target_include_directories(rtcheck
    PUBLIC
        .
)

target_link_libraries(rtcheck
    pthread
    dl
)