#include <iostream>
#include <string>
#include <assert.h>
#include "../include/client/itox.hpp"

int main() {
  std::cout << "Running tests..." << std::endl;
  
  assert(itox(0) == "0");
  std::cout << "Test 1 passed" << std::endl;
  
  assert(itox(1) == "1");
  std::cout << "Test 2 passed" << std::endl;
  
  assert(itox(10) == "A");
  std::cout << "Test 3 passed" << std::endl;
  
  assert(itox(15) == "F");
  std::cout << "Test 4 passed" << std::endl;
  
  assert(itox(16) == "10");
  std::cout << "Test 5 passed" << std::endl;
  
  assert(itox(255) == "FF");
  std::cout << "Test 6 passed" << std::endl;
  
  assert(itox(256) == "100");
  std::cout << "Test 7 passed" << std::endl;

  assert(itox(4095) == "FFF");
  std::cout << "Test 8 passed" << std::endl;
  
  std::cout << "All tests passed!" << std::endl;
  
  return 0;
}