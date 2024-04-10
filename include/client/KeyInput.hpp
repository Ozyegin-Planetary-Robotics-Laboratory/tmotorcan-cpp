/**
 * @file KeyInput.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief 
 * @version 0.1
 * @date 2024-04-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef KEYINPUT_HPP
#define KEYINPUT_HPP

#include "Component.hpp"

struct KeyInput : public UpdatePacket {
  int key_in;
};

#endif //KEYINPUT_HPP