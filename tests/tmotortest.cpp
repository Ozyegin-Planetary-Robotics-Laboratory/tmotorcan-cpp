#include <sys/socket.h>
#include <tmotor.hpp>
#include <gtest/gtest.h>

static std::string can_name = "tmotortestcan";

int publish_can_message(uint8_t *data, size_t data_size)
{
    std::string command = "cansend " + can_name + " 00002901#";
    std::stringstream ss;
    for (size_t i = 0; i < data_size; i++)
    {
        if (data[i] < 0x10)
        {
            ss << "0";
        }
        ss << std::hex << static_cast<int> (data[i]);
    }
    command += ss.str();
    return system(command.c_str());
}

TEST(ThreadSafety, constructDestruct)
{
    TMotor::AKManager motor(0x01);
    EXPECT_EQ(motor.getMotorID(), 0x01);
    EXPECT_EQ(motor.getCurrent(), 0.0f);
    EXPECT_EQ(motor.getVelocity(), 0.0f);
    EXPECT_EQ(motor.getPosition(), 0.0f);
    EXPECT_EQ(motor.getTemperature(), 0);
    EXPECT_EQ(motor.getFault(), TMotor::MotorFault::NONE);
};

TEST(SetGet, setMotorID)
{
    TMotor::AKManager motor;(0x01);
    motor.setMotorID(0x02);
    EXPECT_EQ(motor.getMotorID(), 0x02);
};

TEST(SetGet, getPosition)
{
    TMotor::AKManager motor(0x01);
    motor.connect(can_name.c_str());
    float position_cases[8] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
    uint8_t data[8] = {0x00};
    for (size_t i = 0; i < 8; i++)
    {
        int16_t position = static_cast<int16_t> (position_cases[i] * 10);
        data[1] = position & 0xFF;
        data[0] = (position >> 8) & 0xFF;
        publish_can_message(data, 8);
        float pose = motor.getPosition();
        EXPECT_EQ(pose, position_cases[i]) << "i = " << i;
    }

};

// TEST(SetGet, getVelocity)
// {
//     TMotor::AKManager motor;(0x01);
//     motor.connect(can_name.c_str());
//     float velocity_cases[8] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
//     uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//     for (size_t i = 0; i < 8; i++)
//     {
//         int16_t velocity = static_cast<int16_t> (velocity_cases[i]);
//         data[3] = velocity & 0xFF;
//         data[2] = (velocity >> 8) & 0xFF;
//         publish_can_message(data, 8);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         EXPECT_EQ(motor.getVelocity(), velocity_cases[i]);
//     }
// }

// TEST(SetGet, getCurrent)
// {
//     TMotor::AKManager motor;(0x01);
//     motor.connect(can_name.c_str());
//     float current_cases[8] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f};
//     uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//     for (size_t i = 0; i < 8; i++)
//     {
//         int16_t current = static_cast<int16_t> (current_cases[i] * 100);
//         data[5] = current & 0xFF;
//         data[4] = (current >> 8) & 0xFF;
//         publish_can_message(data, 8);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         EXPECT_EQ(motor.getCurrent(), current_cases[i]);
//     }
// }

// TEST(SetGet, getTemperature)
// {
//     TMotor::AKManager motor;(0x01);
//     motor.connect(can_name.c_str());
//     int8_t temperature_cases[8] = {0, 10, 20, 30, 40, 50, 60, 70};
//     uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//     for (size_t i = 0; i < 8; i++)
//     {
//         int8_t temperature = temperature_cases[i];
//         data[6] = temperature;
//         publish_can_message(data, 8);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         EXPECT_EQ(motor.getTemperature(), temperature_cases[i]);
//     }
// }

// TEST(SetGet, getFault)
// {
//     TMotor::AKManager motor;(0x01);
//     motor.connect(can_name.c_str());
//     TMotor::MotorFault fault_cases[8] = {
//         TMotor::MotorFault::NONE,
//         TMotor::MotorFault::OVERTEMPERATURE,
//         TMotor::MotorFault::OVERCURRENT,
//         TMotor::MotorFault::OVERVOLTAGE,
//         TMotor::MotorFault::UNDERVOLTAGE,
//         TMotor::MotorFault::ENCODER,
//         TMotor::MotorFault::HARDWARE
//     };
//     uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//     for (TMotor::MotorFault fault_case : fault_cases)
//     {
//         data[7] = static_cast<uint8_t> (fault_case);
//         publish_can_message(data, 8);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         EXPECT_EQ(motor.getFault(), fault_case);
//     }
// }