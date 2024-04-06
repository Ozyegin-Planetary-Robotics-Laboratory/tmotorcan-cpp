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
};

enum MotorOriginMode {
  TEMPORARY = 0x00000000,
  PERMANENT = 0x00000001,
  RESTORE = 0x00000002
};

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

  void __read_motor_message() {
    struct can_frame rframe;
    int nbytes = read(_can_fd, &rframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while reading from socket");
    }
    if (nbytes < sizeof(struct can_frame)) {
      return;
    }

    position = ((int16_t) (rframe.data[0] << 8 | rframe.data[1])) * 0.1f;
    velocity = ((int16_t) (rframe.data[2] << 8 | rframe.data[3])) * 10.0f;
    current = ((int16_t) (rframe.data[4] << 8 | rframe.data[5])) * 0.01f;
    temperature = rframe.data[6];
    _motor_fault = (MotorFault) rframe.data[7];
  }

public:
  float cmd_angle;
  float current;      // A   [-60, 60]
  float velocity;     // rpm [-320000, 320000] 
  float position;     // deg [-3200, 3200]
  int8_t temperature; // C   [-20, 127]

  /**
   * @class AK60Manager
   * @brief Class for managing AK60 motors over CAN interface.
   *
   * The AK60Manager class provides functionality to control AK60 motors
   * using the CAN interface. It allows setting the motor mode, position,
   * and reading motor messages.
   */
  AK60Manager(const uint8_t motor_id, const char *can_interface) :
    _shutdown(false),
    _motor_mode(MotorMode::POSITION),
    cmd_angle(0.0)
  {
    _motor_id = motor_id;

    /* create socket file descriptor */
    _can_fd = -1;
    while (_can_fd < 0) {
      if ((_can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Error while opening socket");
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
        perror("Error in socket bind");
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

  /**
   * @warning This function is not tested with.
  */
  void setOrigin(MotorOriginMode mode) {
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::SETORIGIN;
    wframe.data[0] = mode;
    wframe.can_dlc = 1;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
    }
  }
  
  /**
   * @warning This function is not tested with.
   * 
   * @param duty The duty cycle to apply to the motor. (Unit unknown, assumed to be between -100 and 100)
  */
  void sendDutyCycle(float duty) {
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::DUTY;
    int32_t duty_cmd_int = (int32_t) (duty * 100000.0f);
    wframe.data[0] = duty_cmd_int & 0xFF;
    wframe.data[1] = (duty_cmd_int >> 8) & 0xFF;
    wframe.data[2] = (duty_cmd_int >> 16) & 0xFF;
    wframe.data[3] = (duty_cmd_int >> 24) & 0xFF;
    wframe.can_dlc = 4;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
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
    float current_max = 60.0;
    float current_min = -60.0;
    if (current > current_max) current = current_max;
    if (current < current_min) current = current_min;
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::CURRENTLOOP;
    int32_t current_cmd_int = (int32_t) (current * 100.0f);
    wframe.data[0] = current_cmd_int & 0xFF;
    wframe.data[1] = (current_cmd_int >> 8) & 0xFF;
    wframe.data[2] = (current_cmd_int >> 16) & 0xFF;
    wframe.data[3] = (current_cmd_int >> 24) & 0xFF;
    wframe.can_dlc = 4;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
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
    float current_max = 60.0;
    float current_min = 0.0;
    if (current > current_max) current = current_max;
    if (current < current_min) current = current_min;
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::CURRENTBREAK;
    int32_t current_cmd_int = (int32_t) (current * 1000.0f);
    wframe.data[0] = current_cmd_int & 0xFF;
    wframe.data[1] = (current_cmd_int >> 8) & 0xFF;
    wframe.data[2] = (current_cmd_int >> 16) & 0xFF;
    wframe.data[3] = (current_cmd_int >> 24) & 0xFF;
    wframe.can_dlc = 4;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
    }
  }

  /**
   * @warning This function is not tested with.
   * 
   * @warning The velocity unit is assumed to be RPM, however, it is not clear.
  */
  void sendVelocity(int32_t velocity) {
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::VELOCITY;
    int32_t vel_cmd_int = velocity;
    wframe.data[0] = vel_cmd_int & 0xFF;
    wframe.data[1] = (vel_cmd_int >> 8) & 0xFF;
    wframe.data[2] = (vel_cmd_int >> 16) & 0xFF;
    wframe.data[3] = (vel_cmd_int >> 24) & 0xFF;
    wframe.can_dlc = 4;
    int nbytes = write(_can_fd, &wframe, sizeof(struct can_frame));
    if (nbytes < 0) {
      perror("Error while writing to socket");
    }
  }

  /**
   * @brief Brings the motor to the specified position.
   * 
   * @param pose The position to bring the motor to. (Unit unknown)
  */
  void sendPosition(float pose) {
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::POSITION;
    int32_t pos_cmd_int = (int32_t) (pose * 10000.0f);
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

  /**
   * @warning This function is not tested with.
   * 
   * @brief Brings the motor to the specified position with the specified velocity and acceleration.
   * 
   * @param pose The position to bring the motor to. (Unit unknown)
   * 
   * @param vel The velocity to move the motor with. (Unit unknown)
   * 
   * @param acc The acceleration to move the motor with. (Unit unknown)
  */
  void sendPositionVelocity(float pose, int16_t vel, int16_t acc) {
    int16_t acc_max = 200;
    int16_t acc_min = 0;
    if (acc > acc_max) acc = acc_max;
    if (acc < acc_min) acc = acc_min;
    struct can_frame wframe;
    wframe.can_id = CAN_EFF_FLAG | _motor_id | MotorMode::POSITIONVELOCITY;
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
      perror("Error while writing to socket");
    }
  }

};

} // namespace TMotor

#endif // H_AK60_HPP