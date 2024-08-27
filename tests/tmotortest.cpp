#include <sys/socket.h>
#include <tmotor.hpp>
#include <gtest/gtest.h>

TEST(ThreadSafety, constructDestruct)
{
  TMotor::AKManager motor(0x01);
};

TEST(Defaults, memberVariables)
{
  TMotor::AKManager motor(0x01);
  ASSERT_EQ(motor.getMotorID(), 0x01);
  ASSERT_EQ(motor.getCurrent(), 0.0f);
  ASSERT_EQ(motor.getVelocity(), 0.0f);
  ASSERT_EQ(motor.getPosition(), 0.0f);
  ASSERT_EQ(motor.getTemperature(), 0);
  ASSERT_EQ(motor.getFault(), TMotor::MotorFault::NONE);
};

TEST(SetGet, setMotorID)
{
  TMotor::AKManager motor(0x01);
  motor.setMotorID(0x02);
  ASSERT_EQ(motor.getMotorID(), 0x02);
};