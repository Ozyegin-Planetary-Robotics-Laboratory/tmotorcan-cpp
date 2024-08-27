# TMotor library target
add_library(tmotor STATIC src/tmotor.cpp)
target_include_directories(tmotor PUBLIC include)
set_property(TARGET tmotor PROPERTY POSITION_INDEPENDENT_CODE ON)

# Install targets
install(TARGETS tmotor
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install (FILES include/tmotor.hpp DESTINATION include)
