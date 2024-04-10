/**
 * @file Entry.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief
 * @version 0.1
 * @date 2024-04-08
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ENTRY_HPP
#define ENTRY_HPP

#include <string.h>
#include <iomanip>
#include <string>
#include "Component.hpp"
#include "KeyInput.hpp"
#include "itox.hpp"

struct EntryMenu : public Component {
  int h, w;
  int motor_id;

  EntryMenu(int h, int w):
    Component(h, w, (LINES-h)/2, (COLS-w)/2),
    h(h), w(w),
    motor_id(0)
  {
    return;
  }

  ~EntryMenu() { return; }

  void mount() override {
    wclear(win);
    const char *label = "Enter Motor ID: 0";
    size_t label_len = 16;
    box(win, 0, 0);
    mvwprintw(win, h/2, (w-label_len)/2, label);
    wrefresh(win);
  }

  void update(UpdatePacket *data_in) override {
    int key_in = static_cast<KeyInput *>(data_in)->key_in;
    switch (key_in) {
      case KEY_RIGHT:
        if (motor_id < 0xFF) {
          motor_id++;
        }
        break;
      case KEY_LEFT:
        if (motor_id > 0) {
          motor_id--;
        }
        break;
      default:
        break;
    }
    if (key_in == KEY_RIGHT || key_in == KEY_LEFT) {
      size_t label_len = 16;
      mvwprintw(win, h/2, (w+label_len)/2, "  ");
      mvwprintw(win, h/2, (w+label_len)/2, itox(motor_id).c_str());
      wrefresh(win);
    }
    return;
  }

  void unmount() override {
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
    return;
  }

};

#endif // ENTRY_HPP