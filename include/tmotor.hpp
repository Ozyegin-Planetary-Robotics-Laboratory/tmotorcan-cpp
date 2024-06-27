#ifndef H_TMOTOR_HPP
#define H_TMOTOR_HPP

/**
 * @file tmotor.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief An async C++ interface for controlling AK motors over CAN bus for Linux.
 * @note This library only supports servo mode for AK motors.
 * @version 0.1
 * @date 2024-02-22
 *
 * @copyright Copyright (c) 2024
 *
 */

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

std::string fault_to_string(MotorFault &fault);

/**
 * @brief AK Motors CAN Interface
 * This class is used to communicate with the AK60 & AK70 motors via CAN bus. Accepted IDs are uint8_t types.
 * The class is designed to be used with a single instance. It is not thread-safe. After the appropriate control mode is selected,
 * members can be directly changed to control the motor. The class will handle the rest. Create one object for each motor.
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

  void __read_motor_message();

public:

  /**
   * @brief Default constructor for the AKManager class.
   *
   * This constructor initializes the AKManager object without an active ID, you must set the ID before
   * calling connect() by using setMotorID().
   */
  AKManager();

  /**
   * @brief Constructor for the AKManager class.
   * 
   * This constructor initializes the AKManager object with the motor ID.
   */
  AKManager(const uint8_t motor_id);

  /**
   * @brief Copy constructor for the AKManager class.
   *
   * This constructor creates a new AKManager object by copying the values from another AKManager object.
   *
   * @param other The AKManager object to copy from.
   */
  AKManager(const AKManager& other);

  /**
   * @brief Destructor for the AKManager class.
   *
   * This destructor sets the _shutdown flag to true and closes the _can_fd file descriptor.
   */
  ~AKManager();

  /**
   * @brief Set the motor ID.
   * 
   * @param motor_id The motor ID to set.
   */
  void setMotorID(const uint8_t motor_id);

  /**
   * @brief Get the motor ID.
   * 
   * @return The motor ID.
  */
  uint8_t getMotorID();

  /**
   * @brief Get the motor current.
   * 
   * @return The motor current.
  */
  float getCurrent();

  /**
   * @brief Get the motor velocity.
   * 
   * @return The motor velocity.
  */
  float getVelocity();

  /**
   * @brief Get the motor position.
   * 
   * @return The motor position.
  */
  float getPosition();

  /**
   * @brief Get the motor temperature.
   * 
   * @return The motor temperature.
  */
  int8_t getTemperature();

  /**
   * @brief Get the motor fault.
   * 
   * @return The motor fault.
  */
  MotorFault getFault();

  /**
   * @brief Connect to the CAN interface.
   * 
   * @param can_interface The CAN interface to connect to. ("vcan0", "can0", etc.)
   */
  void connect(const char *can_interface);

  /**
   * @warning This function is not tested with.
  */
  void setOrigin(MotorOriginMode mode);
  
  /**
   * @warning This function is not tested with.
   * 
   * @param duty The duty cycle to apply to the motor. (Unit unknown, assumed to be between -100 and 100)
  */
  void sendDutyCycle(float duty);
  
  /**
   * @warning This function is not tested with.
   * 
   * @param current The current value the motor will draw, between -60 and 60A.
   * 
   * @note The torque applied is equal to the current multiplied by the torque constant of the motor, which is 1/kv.
  */
  void sendCurrent(float current);

  /**
   * @warning This function is not tested with.
   * 
   * @param current The current value the motor will draw, between 0 and 60A.
   * 
   * @brief Stops the motor at the current position, it will try to resist movement with up to the specified current.
  */
  void sendCurrentBrake(float current);

  /**
   * @brief Sends a radial velocity command to the motor.
   * 
   * @param vel The radial velocity to move the motor with. (degrees/sec)
  */
  void sendVelocity(float vel);

  /**
   * @brief Brings the motor to the specified position.
   * 
   * @param pose The position to bring the motor to. (degrees)
   * 
   * @note The input is between -36000 and 36000. Also, take care as the default speed is high.
  */
  void sendPosition(float pose);

  /**
   * @brief Brings the motor to the specified position with the specified velocity and acceleration.
   * 
   * @param pose The position to bring the motor to. (degrees)
   * 
   * @param vel The velocity to move the motor with. (degrees/second)
   * 
   * @param acc The acceleration to move the motor with. (degrees/second^2)
  */
  void sendPositionVelocityAcceleration(float pose, int16_t vel, int16_t acc);

};

} // namespace TMotor

#endif // H_TMOTOR_HPP
