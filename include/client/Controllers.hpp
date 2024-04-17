/**
 * @file Controllers.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief 
 * @version 0.1
 * @date 2024-04-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef CONTROLLERS_HPP
#define CONTROLLERS_HPP

#include <ncurses.h>
#include "../ak60.hpp"
#include "KeyInput.hpp"
#include "Component.hpp"
#include "Selection.hpp"

struct Controller : public Component {
  bool mounted;
  bool selected;
  bool cursor_locked;
  MotorSelection type;
  TMotor::AK60Manager *ak60handle;

  Controller(int h, int w, int y, int x, MotorSelection type_in, TMotor::AK60Manager *ak60handle_in):
    Component(h, w, y, x),
    mounted(false),
    selected(false),
    cursor_locked(false),
    type(type_in),
    ak60handle(ak60handle_in)
  {
    return;
  }

  ~Controller() { return; }

  virtual void select() {
    return;
  }

  virtual void deselect() {
    return;
  }

};

struct DutyController : public Controller {
  DutyController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, DUTY, ak60handle)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

};

struct CurrentLoopController : public Controller {
  CurrentLoopController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, CURRENTLOOP, ak60handle)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

};

struct CurrentBreakController : public Controller {
  CurrentBreakController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, CURRENTBREAK, ak60handle)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

};

struct VelocityController : public Controller {
  VelocityController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, VELOCITY, ak60handle)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

};

struct PositionController : public Controller {
private:
  std::thread _command_thread;
  bool _command_active;
  int _selected_field;
  std::string _input_buffer;
  float _position;

  void __flush_str_input_buff() {
    mvwprintw(win, h/3, w/2-7+6, "             ");
    wrefresh(win);
  }

  void __process_str_input(char *str_in) {
    try {
      float new_pose = std::stof(str_in);
      _position = new_pose;
      _input_buffer = std::to_string(_position);
    } catch (const std::invalid_argument& e) {}
    __flush_str_input_buff();
    mvwprintw(win, h/3, w/2-7+6, _input_buffer.c_str());
    wrefresh(win);
  }

  void __change_selection() {
    _selected_field = !_selected_field;
    if (_selected_field) {
      mvwprintw(win, h/3, w/2-7, "Pose: ");
      wattron(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
      mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
      wattroff(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
    } else {
      mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
      wattron(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
      mvwprintw(win, h/3, w/2-7, "Pose: ");
      wattroff(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
    }
    wrefresh(win);
  }

public:
  PositionController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, POSITION, ak60handle),
    _command_active(false),
    _selected_field(0),
    _input_buffer("0.0"),
    _position(0.0)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 1, (w-17)/2, "Position Control");
    mvwprintw(win, h/3, w/2-7, "Pose: ");
    mvwprintw(win, h/3, w/2, _input_buffer.c_str());
    mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
    wrefresh(win);
    return;
  }

  void update(UpdatePacket *data) override {
    int key = static_cast<KeyInput *> (data)->key_in;
    switch (key) {
    case KEY_RIGHT:
      if (!_command_active) {
        __change_selection();
      }
      break;
    case KEY_LEFT:
      if (!_command_active) {
        __change_selection();
      }
      break;
    case '\n':
      if (!_selected_field) {
        cbreak();
        echo();
        wattron(win, COLOR_PAIR(0));
        char str_in[12];
        __flush_str_input_buff();
        wmove(win, h/3, w/2-7+6);
        wrefresh(win);
        wgetnstr(win, str_in, 12);
        wattroff(win, COLOR_PAIR(0));
        __process_str_input(str_in);
        wrefresh(win);
        raw();
        noecho();
      } else {
        if (_command_active) {
          _command_active = false;
          cursor_locked = false;
          _command_thread.join();
          wattron(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
          mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
          wattroff(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
        } else {
          _command_active = true;
          cursor_locked = true;
          _command_thread = std::thread([this] {
            while (_command_active) {
              ak60handle->sendPosition(_position);
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
          });
          wattron(win, A_NORMAL | COLOR_PAIR(4) | A_BOLD);
          mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
          wattroff(win, A_NORMAL | COLOR_PAIR(4) | A_BOLD);
        }
        wrefresh(win);
      }
      break;
    default:
      break;
    }
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

  void select() override {
    selected = true;
    switch (_selected_field) {
      case 0:
        wattron(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
        mvwprintw(win, h/3, w/2-7, "Pose:");
        wattroff(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
        break;
      case 1:
        wattron(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
        mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
        wattroff(win, A_NORMAL | COLOR_PAIR(2) | A_BOLD);
        break;
      default:
        break;
    }
    wrefresh(win);
    return;
  }

  void deselect() override {
    selected = false;
    switch (_selected_field) {
      case 0:
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, h/3, w/2-7, "Pose:");
        wattroff(win, COLOR_PAIR(3));
        break;
      case 1:
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, 2*h/3, w/2-9/2, "Delegate");
        wattroff(win, COLOR_PAIR(3));
        break;
      default:
        break;
    }
    wrefresh(win);
    return;
  }

};

struct PositionVelocityController : public Controller {
  PositionVelocityController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, POSITIONVELOCITY, ak60handle)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

};

struct SetOriginController : public Controller {
private:
  int _activated_field;
  int _cursor_position;
  enum ButtonState : uint8_t {
    NORMAL,
    ACTIVE,
    SELECTED
  };
  ButtonState _button_states[4] = {
    SELECTED,
    NORMAL,
    NORMAL,
    NORMAL
  };
  const char *_button_labels[4] = {
    "Temporary",
    "Permanent",
    "Restore",
    "Delegate"
  };

  void __update_buttons() {
    for (int i = 0; i < 4; i++) {
      int color = static_cast<int> (_button_states[i]);
      wattron(win, A_NORMAL | COLOR_PAIR(color) | A_BOLD);
      mvwprintw(win, h/3+i + i/3, 2, _button_labels[i]);
      wattroff(win, A_NORMAL | COLOR_PAIR(color) | A_BOLD);
    }
  }

public:
  SetOriginController(int h, int w, int y, int x, TMotor::AK60Manager *ak60handle):
    Controller(h, w, y, x, SETORIGIN, ak60handle),
    _activated_field(-1),
    _cursor_position(0)
  {
    return;
  }

  void mount() override {
    if (mounted) return;
    mounted = true;
    wbkgd(win, A_NORMAL | COLOR_PAIR(3));
    wclear(win);
    mvwprintw(win, 1, (w-11)/2, "Set Origin");
    __update_buttons();
    box(win, 0, 0);
    wrefresh(win);
    return;
  }

  void update(UpdatePacket *data) override {
    int key = static_cast<KeyInput *> (data)->key_in;
    switch (key) {
      case KEY_RIGHT:
        if (_button_states[_cursor_position] == SELECTED) {
          _button_states[_cursor_position] = NORMAL;
        }
        if (_cursor_position < 3) {
          _cursor_position++;
        } else {
          _cursor_position = 0;
        }
        if (_button_states[_cursor_position] == NORMAL) {
          _button_states[_cursor_position] = SELECTED;
        }
        break;
      case KEY_LEFT:
        if (_button_states[_cursor_position] == SELECTED) {
          _button_states[_cursor_position] = NORMAL;
        }
        if (_cursor_position > 0) {
          _cursor_position--;
        } else {
          _cursor_position = 3;
        }
        if (_button_states[_cursor_position] == NORMAL) {
          _button_states[_cursor_position] = SELECTED;
        } 
        break;
      case '\n':
        if (_cursor_position != 3) {
          if (_activated_field == -1) {
            _activated_field = _cursor_position;
            _button_states[_cursor_position] = ACTIVE;
          } else {
            if (_button_states[_cursor_position] == ACTIVE) {
              _button_states[_activated_field] = SELECTED;
              _activated_field = -1;
            } else {
              _button_states[_activated_field] = NORMAL;
              _activated_field = _cursor_position;
              _button_states[_cursor_position] = ACTIVE;
            }
          }
        } else {
          if (_activated_field != -1) {
            ak60handle->setOrigin(static_cast<TMotor::MotorOriginMode> (_activated_field));
            _button_states[3] = ACTIVE;
            __update_buttons();
            wrefresh(win);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            _button_states[3] = SELECTED;
            __update_buttons();
            wrefresh(win);
          }
        }
        break;
    }
    __update_buttons();
    wrefresh(win);
  }

  void unmount() override {
    mounted = false;
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
  }

  void select() override {
    
  }

  void deselect() override {

  }

};

#endif // CONTROLLERS_HPP