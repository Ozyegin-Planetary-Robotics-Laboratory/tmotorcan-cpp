/**
 * @file TMotor.cpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief 
 * @version 0.1
 * @date 2024-05-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../include/tmotor.hpp"

using namespace TMotor;

std::string fault_to_string(MotorFault &fault) {
  switch (fault) {
    case MotorFault::NONE:
      return std::string("None");
    case MotorFault::OVERTEMPERATURE:
      return std::string("Overtemperature");
    case MotorFault::OVERCURRENT:
      return std::string("Overcurrent");
    case MotorFault::OVERVOLTAGE:
      return std::string("Overvoltage");
    case MotorFault::UNDERVOLTAGE:
      return std::string("Undervoltage");
    case MotorFault::ENCODER:
      return std::string("Encoder");
    case MotorFault::HARDWARE:
      return std::string("Hardware");
    default:
      return std::string("INVALID FAULT");
  }
}

void AKManager::__read_motor_message() {
  struct can_frame rframe;
  int nbytes = read(_can_fd, &rframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while reading from socket");
  }
  if (nbytes < sizeof(struct can_frame)) {
    return;
  }
  if (rframe.can_dlc != 8) {
    return;
  }

  std::lock_guard<std::mutex> lock(_mutex);
  _position = ((int16_t) (rframe.data[0] << 8 | rframe.data[1])) * 0.1f;
  _velocity = ((int16_t) (rframe.data[2] << 8 | rframe.data[3]));
  _current = ((int16_t) (rframe.data[4] << 8 | rframe.data[5])) * 0.01f;
  _temperature = rframe.data[6];
  _motor_fault = (MotorFault) rframe.data[7];
}

AKManager::AKManager() :
  _can_fd(-1),
  _shutdown(false),
  _motor_id(-1),
  _current(0.0f),
  _velocity(0.0f),
  _position(0.0f),
  _temperature(0),
  _motor_fault(MotorFault::NONE)
{
  return;
}

AKManager::AKManager(const uint8_t motor_id) :
  _can_fd(-1),
  _shutdown(false),
  _motor_id(motor_id),
  _current(0.0f),
  _velocity(0.0f),
  _position(0.0f),
  _temperature(0),
  _motor_fault(MotorFault::NONE)
{
  return;
}

AKManager::AKManager(const AKManager& other) :
  _can_fd(-1),
  _shutdown(false),
  _motor_id(other._motor_id),
  _current(0.0f),
  _velocity(0.0f),
  _position(0.0f),
  _temperature(0),
  _motor_fault(MotorFault::NONE)
{}

AKManager::~AKManager() {
  _shutdown = true;
  close(_can_fd);
}

void AKManager::setMotorID(const uint8_t motor_id) {
  _motor_id = motor_id;
}

uint8_t AKManager::getMotorID() {
  return _motor_id;
}

float AKManager::getCurrent() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _current;
}

float AKManager::getVelocity() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _velocity;
}

float AKManager::getPosition() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _position;
}

int8_t AKManager::getTemperature() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _temperature;
}

MotorFault AKManager::getFault() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _motor_fault;
}

void AKManager::connect(const char *can_interface) {
    /* create socket file descriptor */
  while (_can_fd < 0) {
    if ((_can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
      throw std::runtime_error("Error while creating the socket.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  /* input the correct network interface name */
  struct ifreq ifr;
  strcpy(ifr.ifr_name, can_interface);
  ioctl(_can_fd, SIOCGIFINDEX, &ifr);

  /* create the socket address and bind the interface to it */
  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  int bind = -1;
  while (bind < 0) {
    if ((bind = ::bind(_can_fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
      throw std::runtime_error("Error while binding the socket.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  /* Filter for the CAN messages. */
  struct can_filter rfilter;
  rfilter.can_id = 0x00002900 | _motor_id;
  rfilter.can_mask = CAN_SFF_MASK;
  setsockopt(_can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(struct can_filter));

  _can_reader = std::thread([this] {
    while (!_shutdown) {
      __read_motor_message();
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  });
  _can_reader.detach();
}

void AKManager::setOrigin(MotorOriginMode mode) {
  if (_can_fd < 0) {
    return;
  }
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::SETORIGIN;
  wframe.data[0] = mode;
  wframe.can_dlc = 1;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }
}

void AKManager::sendDutyCycle(float duty) {
  if (_can_fd < 0) {
    return;
  }
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::DUTY;
  int32_t duty_cmd_int = (int32_t) (duty * 100000.0f);
  wframe.data[0] = duty_cmd_int & 0xFF;
  wframe.data[1] = (duty_cmd_int >> 8) & 0xFF;
  wframe.data[2] = (duty_cmd_int >> 16) & 0xFF;
  wframe.data[3] = (duty_cmd_int >> 24) & 0xFF;
  wframe.can_dlc = 4;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket.");
  }
}

void AKManager::sendCurrent(float current) {
  if (_can_fd < 0) {
    return;
  }
  float current_max = 60.0;
  float current_min = -60.0;
  if (current > current_max) current = current_max;
  if (current < current_min) current = current_min;
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::CURRENTLOOP;
  int32_t current_cmd_int = (int32_t) (current * 100.0f);
  wframe.data[0] = current_cmd_int & 0xFF;
  wframe.data[1] = (current_cmd_int >> 8) & 0xFF;
  wframe.data[2] = (current_cmd_int >> 16) & 0xFF;
  wframe.data[3] = (current_cmd_int >> 24) & 0xFF;
  wframe.can_dlc = 4;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }
}

void AKManager::sendCurrentBrake(float current) {
  if (_can_fd < 0) {
    return;
  }
  float current_max = 60.0;
  float current_min = 0.0;
  if (current > current_max) current = current_max;
  if (current < current_min) current = current_min;
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::CURRENTBREAK;
  int32_t current_cmd_int = (int32_t) (current * 1000.0f);
  wframe.data[0] = current_cmd_int & 0xFF;
  wframe.data[1] = (current_cmd_int >> 8) & 0xFF;
  wframe.data[2] = (current_cmd_int >> 16) & 0xFF;
  wframe.data[3] = (current_cmd_int >> 24) & 0xFF;
  wframe.can_dlc = 4;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }
}

void AKManager::sendVelocity(float vel) {
  if (_can_fd < 0) {
    return;
  }
  int32_t vel_int = ((int32_t) vel)*TMOTOR_AK_POLE_PAIRS;
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::VELOCITY;
  wframe.data[3] = (vel_int & 0xFF);
  wframe.data[2] = (vel_int >> 8) & 0xFF;
  wframe.data[1] = (vel_int >> 16) & 0xFF;
  wframe.data[0] = (vel_int >> 24) & 0xFF;
  wframe.can_dlc = 4;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }

}

void AKManager::sendPosition(float pose) {
  if (_can_fd < 0) {
    return;
  }
  pose = pose > 36000.0f ? 36000.0f : pose;
  pose = pose < -36000.0f ? -36000.0f : pose;
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::POSITION;
  int32_t pos_cmd_int = (int32_t) (pose);
  wframe.data[0] = (pos_cmd_int >> 24) & 0xFF;
  wframe.data[1] = (pos_cmd_int >> 16) & 0xFF;
  wframe.data[2] = (pos_cmd_int >> 8) & 0xFF;
  wframe.data[3] = pos_cmd_int & 0xFF;
  wframe.can_dlc = 4;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }
}

void AKManager::sendPositionVelocityAcceleration(float pose, int16_t vel, int16_t acc) {
  if (_can_fd < 0) {
    return;
  }
  int16_t acc_max = 200;
  int16_t acc_min = 0;
  if (acc > acc_max) acc = acc_max;
  if (acc < acc_min) acc = acc_min;
  struct can_frame wframe;
  wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorModeID::POSITIONVELOCITY;
  int32_t pos_cmd_int = (int32_t) (pose * 10000.0f);
  int16_t vel_cmd_int = vel;
  int16_t acc_cmd_int = acc;
  wframe.data[0] = (pos_cmd_int >> 24) & 0xFF;
  wframe.data[1] = (pos_cmd_int >> 16) & 0xFF;
  wframe.data[2] = (pos_cmd_int >> 8) & 0xFF;
  wframe.data[3] = pos_cmd_int & 0xFF;
  wframe.data[4] = (vel_cmd_int >> 8) & 0xFF;
  wframe.data[5] = vel_cmd_int & 0xFF;
  wframe.data[6] = (acc_cmd_int >> 8) & 0xFF;
  wframe.data[7] = acc_cmd_int & 0xFF;
  wframe.can_dlc = 8;
  int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
  if (nbytes < 0) {
    throw std::runtime_error("Error while writing to the socket");
  }
}