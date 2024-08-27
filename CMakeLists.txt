cmake_minimum_required(VERSION 3.12)
project(tmotorcan-cpp)

# Settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#add_definitions(-DLOCK_ENABLED)

add_subdirectory(src)
if (BUILD_TESTS)
  add_subdirectory(tests)
endif(BUILD_TESTS)