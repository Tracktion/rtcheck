#cmake_minimum_required(VERSION 3.20)
#project(lib_rt_check_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#======================================
message(CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE test_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" pass_*.cpp fail_*.cpp)
message(test_files: ${test_files})

#======================================
foreach(file ${test_files})
  message(STATUS "Processing file: ${file}")
  get_filename_component(test_name ${file} NAME_WLE)
  message(STATUS "Creating test: ${test_name}")

  add_executable(${test_name}
      ${file}
  )

  target_link_libraries(${test_name}
      rtcheck
  )

  if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries(${test_name}
        atomic
    )
  endif()

  target_link_options(${test_name} PRIVATE
      "-rdynamic"
  )

  add_test (NAME ${test_name} COMMAND ${test_name})

  if(${test_name} MATCHES "fail")
    set_property(TEST ${test_name} PROPERTY WILL_FAIL TRUE)
  endif()
endforeach()
