#include <signal.h>
#include <ncurses.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>

#include <Component.hpp>
#include <Dashboard.hpp>
#include <Button.hpp>
#include <Input.hpp>
#include <Menu.hpp>
#include <tmotor.hpp>

int main(int argc, char **argv) {
  float gear_ratio;
  std::string can_interface;
  try
  {
    gear_ratio = std::stof(argv[1]);
    can_interface = argv[2];
    if (can_interface != "vcan0" && can_interface != "can0") throw std::runtime_error("Invalid CAN interface");
  }
  catch(const std::exception& e)
  {
    gear_ratio = 1.0f;
    can_interface = "vcan0";
  }

  initscr();
  atexit((void (*)()) endwin);

  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  noecho();
  refresh();

  init_colors();
  paint_scr();

  int motor_id = 0;
  { // Input motor ID
    InputBufferHex input("Motor ID", (COLS-30)/2, LINES/2, 30, 5);
    attron(COLOR_PAIR(2));
    mvprintw(LINES/2+6, (COLS-54)/2, "Please enter the motor ID using hexadecimal notation.");
    attroff(COLOR_PAIR(2));
    refresh();
    input.mount();
    InputUpdate packet('\n');
    input.update(&packet);
    motor_id = input.m_value;
    if (char c = getch() == 'q') return 0;
    input.unmount();
    clear();
    refresh();
  }

  { // Main loop
    std::shared_ptr<TMotor::AKManager> manager = std::make_shared<TMotor::AKManager>(motor_id);
    manager->connect(can_interface.c_str());

    bool shutdown = false;
    Menu menu(1+COLS/5, 0, 4*COLS/5, (COLS)/(5*2), &shutdown, manager, gear_ratio);
    Dashboard dashboard(0, 0, COLS/5, (COLS)/(5*2), motor_id);
    
    menu.mount();
    dashboard.mount();
    menu.focus();
    dashboard.focus();

    std::thread dashboard_updater([&shutdown, &dashboard, &manager, gear_ratio] {
      while (!shutdown) {
        AKPacket motor_packet(
          manager->getCurrent(),  //current
          manager->getPosition(),  //position
          manager->getVelocity(),  //velocity
          gear_ratio,
          manager->getTemperature(),  //temperature
          manager->getFault()
        );
        dashboard.update(&motor_packet);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
    
    InputUpdate packet('0');
    while (((packet.key_in = getch()) != 'q') && (!shutdown)) {
      menu.update(&packet);
    }
    shutdown = true;
    dashboard.unmount();
    menu.unmount();
    dashboard_updater.join();
  }

  return 0;
}
