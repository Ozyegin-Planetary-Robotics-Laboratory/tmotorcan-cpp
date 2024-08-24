include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest)

add_executable(tmotortest tmotortest.cpp)

target_link_libraries(tmotortest
  PRIVATE
  tmotor
  GTest::gtest_main
)

gtest_discover_tests(tmotortest)