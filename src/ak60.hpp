/**
 * @file ak60.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief
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
#include <map>
#include <vector>
#include <thread>
#include <chrono>

namespace TMotor
{

const uint8_t enterMotorControlMsg[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
const uint8_t exitMotorControlMsg[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
const uint8_t zeroMotorControlMSG[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE};

enum MotorMode {
  DUTY = 0x00000000,
  CURRENTLOOP = 0x00000100,
  CURRENTBREAK = 0x00000200,
  VELOCITY = 0x00000300,
  POSITION = 0x00000400,
  SETORIGIN = 0x00000500,
  POSITIONVELOCITY = 0x00000600
};

enum MotorFault {
  NONE = 0x00000000,
  OVERTEMPERATURE,
  OVERCURRENT,
  OVERVOLTAGE,
  UNDERVOLTAGE,
  ENCODER,
  HARDWARE
}

/**
 * @brief AK60 Motor CAN Interface
 * This class is used to communicate with the AK60 motors via CAN bus. Accepted IDs are uint8_t types.
 * The class is designed to be used with a single instance. It is not thread-safe. After the appropriate control mode is selected,
 * members can be directly changed to control the motor. The class will handle the rest. Create one class for each motor.
 */
class AK60Manager {
protected:
  int _can_fd;
  bool _shutdown;
  uint8_t _motor_id;
  MotorMode _motor_mode;
  MotorFault _motor_fault;

  int64_t _command_period; //ms
  std::thread _can_writer;
  std::thread _can_reader;

  void _setPosition() {
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::POSITION;
    int32_t pos_cmd_int = (int32_t) (cmd_angle * 10000.0f);
    wframe.data[0] = (pos_cmd_int >> 24) & 0xFF;
    wframe.data[1] = (pos_cmd_int >> 16) & 0xFF;
    wframe.data[2] = (pos_cmd_int >> 8) & 0xFF;
    wframe.data[3] = pos_cmd_int & 0xFF;
    wframe.can_dlc = 4;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
    }
  }

  void read_motor_message() {
    struct can_frame rframe;
    int nbytes = read(_can_fd, &rframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while reading from socket");
    }
    if (nbytes < sizeof(struct can_frame)) {
      return;
    }

    position = ((int16_t) (rframe.data[0] << 8 | rframe.data[1])) / 10.0;
    velocity = ((int16_t) (rframe.data[2] << 8 | rframe.data[3])) / 10.0;
    current = ((int16_t) (rframe.data[4] << 8 | rframe.data[5])) / 100.0;
    temperature = rframe.data[6];
    _motor_fault = (MotorFault) rframe.data[7];
  }

public:
  float cmd_angle;
  float current; //mA
  float velocity; //rpm
  float position; //deg
  int8_t temperature; //C

  /**
   * @class AK60Manager
   * @brief Class for managing AK60 motors over CAN interface.
   *
   * The AK60Manager class provides functionality to control AK60 motors
   * using the CAN interface. It allows setting the motor mode, position,
   * and reading motor messages.
   */
  AK60Manager(const uint8_t motor_id, const char *can_interface) :
    cmd_angle(0.0),
    _motor_mode(MotorMode::POSITION),
    _shutdown(false)
  {
    _motor_id = motor_id;

    /* create socket file descriptor */
    _can_fd = -1;
    while (_can_fd < 0) {
      if ((_can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Error while opening socket");
        std::chrono::seconds dura(1);
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
        perror("Error in socket bind");
        std::chrono::seconds dura(1);
      }
    }

    /* Filter for the CAN messages. */
    struct can_filter rfilter;
    rfilter.can_id = 0x00002900 | _motor_id;
    rfilter.can_mask = CAN_SFF_MASK;
    setsockopt(_can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(struct can_filter));

    _can_writer = std::thread([this] {
      while (!_shutdown) {
        switch (_motor_mode)
        {
        case MotorMode::DUTY:
          continue;
        case MotorMode::CURRENTLOOP:
          continue;
        case MotorMode::CURRENTBREAK:
          continue;
        case MotorMode::VELOCITY:
          continue;
        case MotorMode::POSITION:
          _setPosition();
          continue;
        default:
          continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
    _can_writer.detach();

    _can_reader = std::thread([this] {
      while (!_shutdown) {
        read_motor_message();
        std::this_thread::sleep_for(std::chrono::milliseconds(1.0f/400.0f));
      }
    });
    _can_reader.detach();
  }

  /**
   * @brief Destructor for the AK60Manager class.
   *
   * This destructor sets the _shutdown flag to true and closes the _can_fd file descriptor.
   */
  ~AK60Manager() {
    _shutdown = true;
    close(_can_fd);
  }

  /**
   * @brief Get the current motor fault.
   *
   * @return MotorFault The current motor fault.
   */
  MotorFault getFault() {
    return _motor_fault;
  }
};

} // namespace TMotor

#endif // H_AK60_HPP