#cmake_minimum_required(VERSION 3.20)
#project(lib_rt_check)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#======================================
add_library(rt_check SHARED
   lib_rt_check.cpp
)

target_include_directories(rt_check
    PUBLIC
        .
)

target_link_libraries(rt_check
    pthread
    dl
)