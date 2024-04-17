#include <signal.h>
#include <ncurses.h>
#include <memory>
#include <map>
#include "../include/ak60.hpp"
#include "../include/client/Color.hpp"
#include "../include/client/Entry.hpp"
#include "../include/client/Selection.hpp"
#include "../include/client/Dashboard.hpp"
#include "../include/client/Controllers.hpp"

void cleanup() {
  endwin();
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <gear_ratio>" << std::endl;
    return 1;
  }
  std::string gear_ratio_string = argv[1];
  float gear_ratio_value;
  try {
    gear_ratio_value = std::stof(gear_ratio_string);
  } catch (const std::exception& e) {
    std::cerr << "Invalid gear ratio: " << gear_ratio_string << std::endl;
    return 1;
  }

  std::cout << "Starting TMotorCAN Client" << std::endl;
  std::cout << "Gear Ratio: " << gear_ratio_value << " " << gear_ratio_string << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  initscr();
  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  noecho();
  refresh();

  init_colors();

  EntryMenu entryMenu(5, 80);
  entryMenu.mount();

  int key = getch();
  while (key != '\n') {
    KeyInput keyInput;
    keyInput.key_in = key;
    entryMenu.update(&keyInput);
    key = getch();
  }

  entryMenu.unmount();

  int id = entryMenu.motor_id;
  TMotor::AK60Manager ak60(id, gear_ratio_value);
  try {
    ak60.connect("can0");
  }
  catch(const std::exception& e)
  { 
    clear();
    attron(COLOR_PAIR(WARNING_PAIR));
    mvprintw(LINES/2, (COLS-strlen(e.what()))/2, e.what());
    attroff(COLOR_PAIR(WARNING_PAIR));
    getch();
    cleanup();
    return 1;
  }

  SelectionMenu selectionMenu;
  selectionMenu.mount();

  DashboardMenu dashboardMenu(itox(id), 12, COLS/8, 5, 0);
  dashboardMenu.mount();

  bool shutdown = false;
  std::thread dashUpdater([&shutdown, &ak60, &dashboardMenu](){
    while (!shutdown) {
      DashboardData dashboardData;
      dashboardData.current = ak60.current;
      dashboardData.velocity = ak60.velocity;
      dashboardData.position = ak60.position;
      dashboardData.temperature = ak60.temperature;
      dashboardData.fault = ak60.motor_fault;
      dashboardMenu.update(&dashboardData);
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  });

  bool menu = true;
  KeyInput key_data;
  MotorSelection selected_menu = NONE;
  std::map<MotorSelection, std::shared_ptr<Controller>> controlMenus = {
    {DUTY,             std::make_shared<DutyController>             (12, COLS/8, 5, COLS*1/8, &ak60)},
    {CURRENTLOOP,      std::make_shared<CurrentLoopController>      (12, COLS/8, 5, COLS*2/8, &ak60)},
    {CURRENTBREAK,     std::make_shared<CurrentBreakController>     (12, COLS/8, 5, COLS*3/8, &ak60)},
    {VELOCITY,         std::make_shared<VelocityController>         (12, COLS/8, 5, COLS*4/8, &ak60)},
    {POSITION,         std::make_shared<PositionController>         (12, COLS/8, 5, COLS*5/8, &ak60)},
    {POSITIONVELOCITY, std::make_shared<PositionVelocityController> (12, COLS/8, 5, COLS*6/8, &ak60)},
    {SETORIGIN,        std::make_shared<SetOriginController>        (12, COLS/8, 5, COLS*7/8, &ak60)}
  };
  mvprintw(LINES-1, 0, "Gear Ratio: %f", gear_ratio_value);

  while (key != 'q') {
    key = getch();
    if (selected_menu != NONE && key == 'q') {
      if (controlMenus.at(selected_menu)->cursor_locked) {
        continue;
      }
    }
    key_data.key_in = key;

    if (menu) {
      switch (key) {
        case KEY_UP:
          break;
        case KEY_DOWN:
          if (selected_menu != NONE) {
            menu = false;
            selectionMenu.hideCursor();
            controlMenus.at(selected_menu)->select();
          }
          break;
        case KEY_LEFT:
          selectionMenu.update(&key_data);
          break;
        case KEY_RIGHT:
          selectionMenu.update(&key_data);
          break;
        case '\n':
          selectionMenu.update(&key_data);
          if (selected_menu == selectionMenu.getSelection()) break;
          if (selected_menu != NONE) {
            controlMenus.at(selected_menu)->unmount();
          }
          selected_menu = selectionMenu.getSelection();
          controlMenus.at(selected_menu)->mount();
          break;
        default:
          break;
      }
    } else {
      switch (key) {
        case KEY_UP:
          if (!controlMenus.at(selected_menu)->cursor_locked) {
            menu = true;
            selectionMenu.showCursor();
            controlMenus.at(selected_menu)->deselect();
          }
          break;
        case KEY_DOWN:
          break;
        default:
          controlMenus.at(selected_menu)->update(&key_data);
          break;
      }
    }
  }

  shutdown = true;
  dashUpdater.join();
  dashboardMenu.unmount();
  selectionMenu.unmount();
  atexit(cleanup);
  return 0;
}
