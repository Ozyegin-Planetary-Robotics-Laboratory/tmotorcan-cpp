#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <ncurses.h>
#include <map>
#include <string>
#include <sstream>
#include <memory>

// screen_init
void paint_scr() {
  bkgd(COLOR_PAIR(1));
  for (int i = 0; i < LINES; i++) {
    for (int j = 0; j < COLS; j++) {
      mvprintw(i, j, " ");
    }
  }
  refresh();
}

// Colors init
void init_colors() {
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_WHITE, COLOR_GREEN);
}

// Baseline component classes
struct UpdatePacket {
  enum Type : uint16_t {
    NONE=0,
    BUTTON,
    INPUT,
    AK
  };

  Type type;

  UpdatePacket(Type type): type(type) {}
};

class Component {
protected:
  int x, y, w, h;
  WINDOW *m_win;
  bool m_focused;
  bool m_locking;

  void _log(std::string msg) {
    const char *msg_c = msg.c_str();
    mvwprintw(m_win, 1, 1, "%s", msg_c);
    wrefresh(m_win);
  }

public:
  Component(int x, int y, int w, int h):
    x(x), y(y), w(w), h(h)
  {
    m_win = newwin(h, w, y, x);
    m_focused = false;
    m_locking = false;
  }

  ~Component() {
    delwin(m_win);
  }

  virtual void focus() = 0;
  virtual void unfocus() = 0;
  virtual void mount() = 0;
  virtual void update(UpdatePacket *packet) = 0;
  virtual void unmount() = 0;
};

// Buttons
struct ButtonState {
  enum Value : uint8_t {
    NORMAL=1,
    HOVER,
    ACTIVE
  };

  Value value;

  ButtonState(Value value): value(value) {}

  void operator++() {
    value = static_cast<Value>((value + 1) % 4);
  }

  void operator--() {
    value = static_cast<Value>((value - 1) % 4);
  }

};

struct ButtonUpdate : public UpdatePacket {
  ButtonState state;

  ButtonUpdate(ButtonState state):
    UpdatePacket(Type::BUTTON),
    state(state)
  {}
};

#endif // COMPONENT_HPP