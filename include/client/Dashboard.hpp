/**
 * @file Dashboard.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief 
 * @version 0.1
 * @date 2024-04-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include "../ak60.hpp"
#include "Component.hpp"
#include "Color.hpp"
#include "itox.hpp"

struct DashboardData : public UpdatePacket {
  float current;
  float velocity;
  float position;
  int8_t temperature;
  TMotor::MotorFault fault;
  bool mounted;

  DashboardData():
    current(0.0),
    velocity(0.0),
    position(0.0),
    temperature(0),
    fault(TMotor::MotorFault::NONE),
    mounted(false)
  {
    return;
  }
};

struct DashboardMenu : public Component {
  DashboardData data;
  std::string motor_id;
  bool mounted;

  ~DashboardMenu() { return; }
  
  DashboardMenu(std::string id, int h, int w, int y, int x):
    Component(12, w, y, x),
    data(),
    motor_id(id),
    mounted(false)
  {
    return;
  } 

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 1, w/2-(motor_id.size()+13)/2, "Motor ID: %s", motor_id.c_str());
    mvwprintw(win, 3, 1, "Current: ");
    mvwprintw(win, 4, 1, "Velocity: ");
    mvwprintw(win, 5, 1, "Position: ");
    mvwprintw(win, 6, 1, "Temperature: ");
    mvwprintw(win, 7, 1, "Fault: ");
    wrefresh(win);
    return;
  }

  void update(UpdatePacket *data_in) override {
    if (!mounted) return;
    data = *(static_cast<DashboardData *>(data_in));
    mvwprintw(win, 3, 10, "         ");
    mvwprintw(win, 3, 10, "%.2f A", data.current);
    mvwprintw(win, 4, 11, "              ");
    mvwprintw(win, 4, 11, "%.2f rpm", data.velocity);
    mvwprintw(win, 5, 11, "            ");
    mvwprintw(win, 5, 11, "%.2f deg", data.position);
    mvwprintw(win, 6, 14, "        ");
    mvwprintw(win, 6, 14, "%d C", data.temperature);
    mvwprintw(win, 7, 8, "               ");
    mvwprintw(win, 7, 8, "%s", TMotor::fault_to_string(data.fault).c_str());
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    if (!mounted) return;
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
    return;
  }

};

#endif //DASHBOARD_HPP