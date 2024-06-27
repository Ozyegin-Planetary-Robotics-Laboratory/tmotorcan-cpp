cmake_minimum_required(VERSION 3.12)
project(MyProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create a static library
add_library(tmotor STATIC
  src/tmotor.cpp
)

set_property(TARGET tmotor PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create an executable
add_executable(tmotorui
  src/main.cpp
)

# Link the library to the executable
target_link_libraries(tmotorui PRIVATE tmotor PUBLIC ncurses pthread)

# Install the executable and the library.
install(TARGETS tmotor tmotorui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Install the header files
install (FILES include/tmotor.hpp DESTINATION include)