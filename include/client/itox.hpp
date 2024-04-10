/**
 * @file itox.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief 
 * @version 0.1
 * @date 2024-04-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ITOX_HPP
#define ITOX_HPP

#include <iomanip>

std::string itox(int num) {
  std::stringstream ss;
  ss << std::hex << num;
  std::string hex_str = ss.str();
  for (size_t i = 0; i < hex_str.size(); i++) {
    hex_str[i] = toupper(hex_str[i]);
  }
  return hex_str;
}

#endif //ITOX_HPP