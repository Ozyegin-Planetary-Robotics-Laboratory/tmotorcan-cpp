/**
 * @file Selection.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief
 * @version 0.1
 * @date 2024-04-08
 *
 *
 */

#ifndef SELECTION_HPP
#define SELECTION_HPP

#include "Component.hpp"
#include "KeyInput.hpp"
#include "Color.hpp"

enum MotorSelection {
  NONE,
  DUTY,
  CURRENTLOOP,
  CURRENTBREAK,
  VELOCITY,
  POSITION,
  POSITIONVELOCITY,
  SETORIGIN
};


struct SelectionMenu : public Component {
private:
  MotorSelection _selected;
  MotorSelection _cursor;
  const std::vector<std::string> _buttons = {
    "DUTY",
    "CURRENT-LOOP",
    "CURRENT-BREAK",
    "VELOCITY",
    "POSITION",
    "POSITION-VELOCITY",
    "SET-ORIGIN"
  };
  
  void __hover_button(MotorSelection button_in) {
    if (button_in == _cursor) return;
    MotorSelection _cursor_prev = _cursor;
    _cursor = button_in;

    if (_cursor_prev != _selected && _cursor_prev != NONE) {
      int i = _cursor_prev-1;
      std::string text = _buttons[i];
      mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    }

    if (_cursor != _selected && _cursor != NONE) {
      int i = _cursor-1;
      std::string text = _buttons[i];
      wattron(win, COLOR_PAIR(CURSOR_PAIR));
      mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
      wattroff(win, COLOR_PAIR(CURSOR_PAIR));
    }
    return;
  }

  void __select_button(MotorSelection button_in) {
    if (button_in == _selected || button_in == NONE) return;
    MotorSelection _select_prev = _selected;
    _selected = button_in;

    if (_select_prev != NONE) {
      int i = _select_prev-1;
      std::string text = _buttons[i];
      mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    }

    int i = _selected -1;
    std::string text = _buttons[i];
    wattron(win, COLOR_PAIR(SELECTED_PAIR));
    mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    wattroff(win, COLOR_PAIR(SELECTED_PAIR));
    return;
  }

public:
  
  ~SelectionMenu() { return; }
  
  SelectionMenu():
    Component(5, COLS, 0, 0),
    _selected(NONE),
    _cursor(DUTY)
  {
    return;
  }
  
  void mount() override {
    wclear(win);
    box(win, 0, 0);

    for (int i = 0; i < _buttons.size(); i++) {
      std::string text = _buttons[i];
      mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    }

    wattron(win, COLOR_PAIR(CURSOR_PAIR));
    std::string text = _buttons[_cursor-1];
    mvwprintw(win, 2, _cursor*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    wattroff(win, COLOR_PAIR(CURSOR_PAIR));

    wrefresh(win);
  }

  void update(UpdatePacket *data) override {
    int key_in = static_cast<KeyInput *>(data)->key_in;
    switch (key_in) {
      case KEY_LEFT:
        if (_cursor > 1) {
          __hover_button(static_cast<MotorSelection>(_cursor-1));
        }
        break;
      case KEY_RIGHT:
        if (_cursor < 7) {
          __hover_button(static_cast<MotorSelection>(_cursor+1));
        }
        break;
      case '\n':
        __select_button(_cursor);
        break;
      default:
        break;
    }
    wrefresh(win);
  }

  void hideCursor() {
    if (_cursor == _selected) return;
    int i = _cursor-1;
    std::string text = _buttons[i];
    mvwprintw(win, 2, (i+1)*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    wrefresh(win);
  }

  void showCursor() {
    if (_cursor == _selected) return;
    wattron(win, COLOR_PAIR(CURSOR_PAIR));
    std::string text = _buttons[_cursor-1];
    mvwprintw(win, 2, _cursor*COLS/(_buttons.size()+1)-text.size()/2, text.c_str());
    wattroff(win, COLOR_PAIR(CURSOR_PAIR));
    wrefresh(win);
  }
  
  void unmount() override {
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(win);
    wrefresh(win);
    return;
  }

  MotorSelection getSelection() {
    return _selected;
  }
};

#endif // SELECTION_HPP