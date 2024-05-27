/**
 * @file ak60.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief An async C++ interface for controlling AK60 motors over CAN bus for Linux.
 * @note This library only supports servo mode for AK60 motors.
 * @version 0.1
 * @date 2024-02-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef H_AK60_HPP
#define H_AK60_HPP

#include <stdio.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

#define TMOTOR_AK_POLE_PAIRS 21

namespace TMotor
{

const uint8_t enterMotorControlMsg[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
const uint8_t exitMotorControlMsg[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
const uint8_t zeroMotorControlMSG[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE};

enum MotorModeID {
  DUTY = 0x00000000,
  CURRENTLOOP = 0x00000100,
  CURRENTBREAK = 0x00000200,
  VELOCITY = 0x00000300,
  POSITION = 0x00000400,
  SETORIGIN = 0x00000500,
  POSITIONVELOCITY = 0x00000600
};

enum MotorOriginMode {
  TEMPORARY = 0x00000000,
  PERMANENT = 0x00000001,
  RESTORE = 0x00000002
};

enum MotorFault {
  NONE = 0x00000000,
  OVERTEMPERATURE,
  OVERCURRENT,
  OVERVOLTAGE,
  UNDERVOLTAGE,
  ENCODER,
  HARDWARE
};

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


/**
 * @brief AK Motors CAN Interface
 * This class is used to communicate with the AK60 & AK70 motors via CAN bus. Accepted IDs are uint8_t types.
 * The class is designed to be used with a single instance. It is not thread-safe. After the appropriate control mode is selected,
 * members can be directly changed to control the motor. The class will handle the rest. Create one class for each motor.
 * 
 * @note Only tested with AK60 and AK70s.
 */
class AKManager {
protected:
  int _can_fd;
  bool _shutdown;
  uint8_t _motor_id;
  float _current;            // A   [-60, 60]
  float _velocity;           // rpm [-320000, 320000] 
  float _position;           // deg [-3200, 3200]
  int8_t _temperature;       // C   [-20, 127]
  MotorFault _motor_fault;   // motor fault type
  std::mutex _mutex;
  std::thread _can_reader;

  void __read_motor_message() {
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

public:

  /**
   * @class AKManager
   * @brief Class for managing AK60 motors over CAN interface.
   *
   * The AKManager class provides functionality to control AK60 motors
   * using the CAN interface. It allows setting the motor mode, position,
   * and reading motor messages.
   */
  AKManager(const uint8_t motor_id) :
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

  /**
   * @brief Destructor for the AKManager class.
   *
   * This destructor sets the _shutdown flag to true and closes the _can_fd file descriptor.
   */
  ~AKManager() {
    _shutdown = true;
    close(_can_fd);
  }

  /**
   * @brief Get the motor ID.
   * 
   * @return The motor ID.
  */
  uint8_t getMotorID() {
    return _motor_id;
  }

  /**
   * @brief Get the motor current.
   * 
   * @return The motor current.
  */
  float getCurrent() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _current;
  }

  /**
   * @brief Get the motor velocity.
   * 
   * @return The motor velocity.
  */
  float getVelocity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _velocity;
  }

  /**
   * @brief Get the motor position.
   * 
   * @return The motor position.
  */
  float getPosition() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _position;
  }

  /**
   * @brief Get the motor temperature.
   * 
   * @return The motor temperature.
  */
  int8_t getTemperature() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _temperature;
  }

  /**
   * @brief Get the motor fault.
   * 
   * @return The motor fault.
  */
  MotorFault getFault() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _motor_fault;
  }

  void connect(const char *can_interface) {
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

  /**
   * @warning This function is not tested with.
  */
  void setOrigin(MotorOriginMode mode) {
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
  
  /**
   * @warning This function is not tested with.
   * 
   * @param duty The duty cycle to apply to the motor. (Unit unknown, assumed to be between -100 and 100)
  */
  void sendDutyCycle(float duty) {
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
  
  /**
   * @warning This function is not tested with.
   * 
   * @param current The current value the motor will draw, between -60 and 60A.
   * 
   * @note The torque applied is equal to the current multiplied by the torque constant of the motor, which is 1/kv.
  */
  void sendCurrent(float current) {
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

  /**
   * @warning This function is not tested with.
   * 
   * @param current The current value the motor will draw, between 0 and 60A.
   * 
   * @brief Stops the motor at the current position, it will try to resist movement with up to the specified current.
  */
  void sendCurrentBrake(float current) {
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

  /**
   * @brief Sends a radial velocity command to the motor.
   * 
   * @param vel The radial velocity to move the motor with. (degrees/sec)
  */
  void sendVelocity(float vel) {
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

  /**
   * @brief Brings the motor to the specified position.
   * 
   * @param pose The position to bring the motor to. (degrees)
   * 
   * @note The input is between -36000 and 36000. Also, take care as the default speed is high.
  */
  void sendPosition(float pose) {
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

  /**
   * @brief Brings the motor to the specified position with the specified velocity and acceleration.
   * 
   * @param pose The position to bring the motor to. (degrees)
   * 
   * @param vel The velocity to move the motor with. (degrees/second)
   * 
   * @param acc The acceleration to move the motor with. (degrees/second^2)
  */
  void sendPositionVelocityAcceleration(float pose, int16_t vel, int16_t acc) {
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

};

} // namespace TMotor

#endif // H_AK60_HPP
