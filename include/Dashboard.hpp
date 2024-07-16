#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include <map>

#include "tmotor.hpp"
#include "Component.hpp"

struct AKPacket : public UpdatePacket {
  float current;
  float position;
  float velocity;
  float gear_ratio;
  int8_t temperature;
  TMotor::MotorFault motor_fault;

  AKPacket(float current, float position, float velocity, float gear_ratio, int8_t temperature, TMotor::MotorFault motor_fault):
    UpdatePacket(UpdatePacket::AK),
    current(current),
    position(position),
    velocity(velocity),
    gear_ratio(gear_ratio),
    temperature(temperature),
    motor_fault(motor_fault)
  {}
};

class Dashboard : public Component {
  bool m_shutdown;
  bool m_mounted;
  std::thread m_thread;
  int m_motor_id;

  std::string _fault_to_str(TMotor::MotorFault fault) {
    static std::map<TMotor::MotorFault, std::string> dict({
      {TMotor::MotorFault::NONE,            "NONE"           },
      {TMotor::MotorFault::OVERTEMPERATURE, "OVERTEMPERATURE"},
      {TMotor::MotorFault::OVERCURRENT,     "OVERCURRENT"    },
      {TMotor::MotorFault::OVERVOLTAGE,     "OVERVOLTAGE"    },
      {TMotor::MotorFault::UNDERVOLTAGE,    "UNDERVOLTAGE"   },
      {TMotor::MotorFault::ENCODER,         "ENCODER"        },
      {TMotor::MotorFault::HARDWARE,        "HARDWARE"       }
    });
    return dict[fault];
  }

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _clear() {
    mvwprintw(m_win, 2, 2+11, "%s", _get_empty(w-3-11).c_str());
    mvwprintw(m_win, 3, 2+11, "%s", _get_empty(w-3-11).c_str());
    mvwprintw(m_win, 4, 2+10, "%s", _get_empty(w-3-10).c_str());
    mvwprintw(m_win, 5, 2+13, "%s", _get_empty(w-3-13).c_str());
    mvwprintw(m_win, 6, 2+14, "%s", _get_empty(w-3-14).c_str());
    mvwprintw(m_win, 7, 2+14, "%s", _get_empty(w-3-14).c_str());
  }

  void _draw() {
    mvwprintw(m_win, 2, 2+11, "%.2f", position/gear_ratio);
    mvwprintw(m_win, 3, 2+11, "%.2f", velocity/gear_ratio);
    mvwprintw(m_win, 4, 2+10, "%.2f", current);
    mvwprintw(m_win, 5, 2+13, "%.2f", gear_ratio);
    mvwprintw(m_win, 6, 2+14, "%.2i", temperature);
    mvwprintw(m_win, 7, 2+14, "%s", _fault_to_str(motor_fault).c_str());
  }

public:
  float current;
  float position;
  float velocity;
  float gear_ratio;
  int temperature;
  TMotor::MotorFault motor_fault;

  Dashboard (int x, int y, int w, int h, int id) :
    Component(x, y, w, h),
    m_shutdown(false),
    m_motor_id(id)
  {}
 
  void focus() override {}

  void unfocus() override {}

  void mount() override {
    box(m_win, 0, 0);
    std::string title = std::string("AK ID: ") + std::to_string(m_motor_id);
    mvwprintw(m_win, 1, (w-title.size())/2, "%s", title.c_str());
    mvwprintw(m_win, 2, 2, "Position: ");
    mvwprintw(m_win, 3, 2, "Velocity: ");
    mvwprintw(m_win, 4, 2, "Current: ");
    mvwprintw(m_win, 5, 2, "Gear Ratio: ");
    mvwprintw(m_win, 6, 2, "Temperature: ");
    mvwprintw(m_win, 7, 2, "Motor Fault: ");
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::AK) return;
    AKPacket *update = static_cast<AKPacket *>(packet);
    _clear();
    current = update->current;
    position = update->position;
    velocity = update->velocity;
    gear_ratio = update->gear_ratio;
    temperature = update->temperature;
    motor_fault = update->motor_fault;
    _draw();
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }

};

#endif // DASHBOARD_HPP