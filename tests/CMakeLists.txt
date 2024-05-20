#cmake_minimum_required(VERSION 3.20)
#project(lib_rt_check_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#======================================
message(CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB test_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" test_*.cpp)
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
      rt_check
  )

  target_link_options(${test_name} PRIVATE
      "-rdynamic"
  )

  add_test (NAME ${test_name} COMMAND ${test_name})
  set_property(TEST ${test_name} PROPERTY WILL_FAIL TRUE)

endforeach()
