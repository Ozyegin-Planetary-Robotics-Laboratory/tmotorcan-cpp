cmake_minimum_required(VERSION 3.12)
project(MyProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Comment the below line to disable the lock feature when sending commands
# add_definitions(-DLOCK_ENABLED)

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

# Install the executable and the library
install(TARGETS tmotor tmotorui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Install the header files
install(FILES include/tmotor.hpp DESTINATION include)

# Find Python interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Install the Python script and make it executable
install(
  FILES ${CMAKE_SOURCE_DIR}/src/tm.py
  DESTINATION /usr/local/bin
  RENAME tmotor4
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

add_custom_target(instructions
  COMMAND ${CMAKE_COMMAND} -E echo "Please install the required system packages: xdotool and wmctrl"
  COMMENT "Providing instructions for system package installation..."
)