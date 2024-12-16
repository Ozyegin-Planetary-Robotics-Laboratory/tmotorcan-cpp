#include "../include/tmotor.hpp"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <thread>
#include <atomic>
#include <chrono>

// Utility class for buffer operations
class BufferUtils {
public:
    static void append_int32(uint8_t* buffer, int32_t value, int& index) {
        buffer[index++] = (value >> 24) & 0xFF;
        buffer[index++] = (value >> 16) & 0xFF;
        buffer[index++] = (value >> 8) & 0xFF;
        buffer[index++] = value & 0xFF;
    }

    static void append_int16(uint8_t* buffer, int16_t value, int16_t& index) {
        buffer[index++] = (value >> 8) & 0xFF;
        buffer[index++] = value & 0xFF;
    }
};

// CAN communication handler
class CanBus {
public:
    explicit CanBus(const std::string& interface_name) : interface_name_(interface_name) {
        open_socket();
    }

    ~CanBus() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }

    void send_message(uint32_t id, const uint8_t* data, uint8_t len) {
        struct can_frame frame;
        frame.can_id = id | CAN_EFF_FLAG; // Use Extended Frame Format (EFF)
        frame.can_dlc = len;
        memcpy(frame.data, data, len);

        if (write(socket_fd_, &frame, sizeof(frame)) != sizeof(frame)) {
            throw std::runtime_error("SocketCAN: Error while writing CAN frame");
        }
    }

    std::optional<struct can_frame> receive_message() {
        struct can_frame frame;
        ssize_t nbytes = read(socket_fd_, &frame, sizeof(frame));

        if (nbytes < 0) {
            perror("SocketCAN: Error while reading CAN frame");
            return std::nullopt;
        }

        if (nbytes != sizeof(frame)) {
            std::cerr << "SocketCAN: Incomplete CAN frame received" << std::endl;
            return std::nullopt;
        }

        return frame;
    }

private:
    std::string interface_name_;
    int socket_fd_ = -1;

    void open_socket() {
        socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_fd_ < 0) {
            throw std::runtime_error("SocketCAN: Error while opening socket");
        }

        struct sockaddr_can addr;
        struct ifreq ifr;
        strcpy(ifr.ifr_name, interface_name_.c_str());

        if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
            close(socket_fd_);
            throw std::runtime_error("SocketCAN: Error while getting interface index");
        }

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(socket_fd_);
            throw std::runtime_error("SocketCAN: Error while binding to interface");
        }
    }
};



// Motor controller class
class MotorController {
public:
    MotorController(uint8_t controller_id, CanBus& can_bus)
      : controller_id_(controller_id), can_bus_(can_bus), running_(false) {}
    
    ~MotorController() {
        stop_polling();
      }
    void start_polling() {
        running_ = true;
        polling_thread_ = std::thread([this]() { poll_messages(); });
    }

    void stop_polling() {
        running_ = false;
        if (polling_thread_.joinable()) {
            polling_thread_.join();
        }
    }

    void print_motor_status() const {
        std::cout << "Motor ID: " << static_cast<int>(controller_id_) << "\n"
                  << "Position: " << motor_pos_ << "°\n"
                  << "Speed: " << motor_spd_ << " RPM\n"
                  << "Current: " << motor_cur_ << " A\n"
                  << "Temperature: " << static_cast<int>(motor_temp_) << "°C\n"
                  << "Error: " << static_cast<int>(motor_error_) << "\n";
    }

    void set_duty(float duty) {
      uint8_t buffer[4];
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(duty * 100000.0), index);
      send_command(CAN_PACKET_SET_DUTY, buffer, index);
    }

    void set_current(int current) {
      uint8_t buffer[4];
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(current * 1000.0), index);
      send_command(CAN_PACKET_SET_CURRENT, buffer, index);
    }

    void set_current_brake(float current) {
      uint8_t buffer[4];
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(current * 1000.0), index);
      send_command(CAN_PACKET_SET_CURRENT_BRAKE, buffer, index);
    }

    void set_velocity(float rpm) {
      uint8_t buffer[4];
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(rpm), index);
      send_command(CAN_PACKET_SET_RPM, buffer, index);
    }

    void set_position(float position) {
      uint8_t buffer[4];
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(position * 10000.0), index);
      send_command(CAN_PACKET_SET_POS, buffer, index);
    }

    void set_origin(uint8_t set_origin_mode) {
      uint8_t buffer;
      int32_t index = 0;
      buffer = set_origin_mode;
      send_command(CAN_PACKET_SET_ORIGIN_HERE, &buffer, index);
    }

    void set_pos_spd_acc(uint8_t controller_id, float pos,int16_t spd, int16_t acc ) {
      uint8_t buffer[8];
      int16_t index1 = 4;
      int32_t index = 0;
      BufferUtils::append_int32(buffer, static_cast<int32_t>(pos * 10000.0), index);
      BufferUtils::append_int16(buffer, static_cast<int16_t>(spd / 10.0), index1);
      BufferUtils::append_int16(buffer,static_cast<int16_t>(acc/ 10.0), index1);
      send_command(CAN_PACKET_SET_POS_SPD, buffer, index1);
    }


private:
    uint8_t controller_id_;
    CanBus& can_bus_;
    std::atomic<bool> running_;
    std::thread polling_thread_;

    // Motor state variables
    int16_t motor_pos_ = 0.0f;
    int16_t motor_spd_ = 0.0f;
    int16_t motor_cur_ = 0;
    int8_t motor_temp_ = 0;
    int8_t motor_error_ = 0;

    enum {
        CAN_PACKET_SET_DUTY = 0,
        CAN_PACKET_SET_CURRENT,
        CAN_PACKET_SET_CURRENT_BRAKE,
        CAN_PACKET_SET_RPM,
        CAN_PACKET_SET_POS,
        CAN_PACKET_SET_ORIGIN_HERE,
        CAN_PACKET_SET_POS_SPD,
    };

    uint32_t generate_can_eid(uint32_t packet_id) {
        return controller_id_ | (packet_id << 8);
    }

    void send_command(uint32_t packet_id, const uint8_t* data, uint8_t len) {
        can_bus_.send_message(generate_can_eid(packet_id), data, len);
    }
    void poll_messages() {
        while (running_) {
            auto frame = can_bus_.receive_message();
            if (frame && is_frame_for_this_motor(*frame)) {
                process_message(*frame);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust polling frequency
        }
    }

    bool is_frame_for_this_motor(const struct can_frame& frame) {
        uint8_t received_id = frame.can_id & 0xFF; // Extract the motor ID
        return received_id == controller_id_;
    }

    void process_message(const struct can_frame& frame) {
        if (frame.can_dlc < 8) {
            std::cerr << "Error: Received CAN frame with insufficient data" << std::endl;
            return;
        }

        // Decode position, speed, and current
        int16_t pos_int = (frame.data[0] << 8) | frame.data[1];
        int16_t spd_int = (frame.data[2] << 8) | frame.data[3];
        int16_t cur_int = (frame.data[4] << 8) | frame.data[5];

        motor_pos_ = pos_int / 10;
        motor_spd_ = spd_int * 10;
        motor_cur_ = cur_int / 1;

        motor_temp_ = frame.data[6];
        motor_error_ = frame.data[7];
    }
};


int main() {
    try {
        CanBus can_bus("can0");
        MotorController motor(12
        , can_bus);
        std::cout << "Starting motor control loop..." << std::endl;

        // Start polling messages for the motor
        motor.start_polling();

        // Example usage
        

        // Periodically print motor status
        while(true) {
            motor.print_motor_status();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            motor.set_current(10);

        }

        // Stop polling
        motor.stop_polling();

        std::cout << "Finished motor control loop." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}