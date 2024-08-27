add_executable(tmotorui src/tmotorui.cpp)
target_include_directories(tmotorui PUBLIC include)
target_link_libraries(tmotorui PRIVATE tmotor PUBLIC ncurses pthread)

install(TARGETS tmotorui
  RUNTIME DESTINATION bin
)
