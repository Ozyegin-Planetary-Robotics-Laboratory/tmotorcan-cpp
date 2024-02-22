#include "ak60.hpp"

int main(int argc, char **argv) {
	TMotor::AK60Manager motor(11, "can0");
	motor.cmd_angle = 0.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	motor.cmd_angle = 90.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	motor.cmd_angle = 180.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	motor.cmd_angle = 270.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	motor.cmd_angle = 360.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	motor.cmd_angle = 0.0;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return 0;
}