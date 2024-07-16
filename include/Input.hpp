#ifndef INPUT_HPP
#define INPUT_HPP

#include <sstream>
#include "Component.hpp"

struct InputUpdate : public UpdatePacket {
  int key_in;

  InputUpdate(int key_in):
    UpdatePacket(Type::INPUT),
    key_in(key_in)
  {}
};

template <typename T> class InputBuffer : public Component {
private:
  std::string m_name;
  std::string m_string;
  ButtonState m_state;

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _draw() {
    ButtonState state = m_state;
    if (!m_focused) {
      state.value = ButtonState::NORMAL ;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1, "%s", m_name.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
  }

  void _clean() {
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", _get_empty(w-m_name.length()-2).c_str());
    wrefresh(m_win);
  }

  void _get_input_str() {
    m_state = ButtonState::ACTIVE;
    _draw();
    wrefresh(m_win);
    cbreak();
    echo();
    _clean();
    wmove(m_win, h/2, m_name.length()+1);
    curs_set(1);
    size_t len = w-m_name.length()-2;
    char str_input[len];
    wgetnstr(m_win, str_input, len);
    box(m_win, 0, 0);
    wrefresh(m_win);
    std::string str_input_wrapped(str_input);
    std::istringstream iss(str_input_wrapped);
    iss >> m_value;
    m_string = std::string(m_value);
    curs_set(0);
    raw();
    noecho();
    _draw();
    wrefresh(m_win);
  }

public:
  T m_value;

  InputBuffer(std::string name, int x, int y, int w, int h):
    Component(x, y, w, h),
    m_state(ButtonState::NORMAL),
    m_name(name + ": "),
    m_string("")
  {}

  void focus() override {
    m_focused = true;
    _draw();
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    _draw();
    wrefresh(m_win);
  }

  void mount() override {
    box(m_win, 0, 0);
    _draw();
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::INPUT && packet->type != UpdatePacket::BUTTON) return;
    if (packet->type == UpdatePacket::BUTTON) {
      ButtonUpdate *update = static_cast<ButtonUpdate *>(packet);
      m_state = update->state;
      _draw();
      wrefresh(m_win);
    } else {
      InputUpdate *update = static_cast<InputUpdate *>(packet);
      char key_in = update->key_in;
      switch (key_in) {
      case '\n':
        _get_input_str();
        break;
      default:
        break;
      }
    }
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }
};

template <> class InputBuffer <int> : public Component {
private:
  std::string m_name;
  std::string m_string;
  ButtonState m_state;

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _draw() {
    ButtonState state = m_state;
    if (!m_focused) {
      state.value = ButtonState::NORMAL ;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1, "%s", m_name.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
  }

  void _clean() {
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", _get_empty(w-m_name.length()-2).c_str());
  }

  void _get_input_str() {
    m_state = ButtonState::ACTIVE;
    _draw();
    wrefresh(m_win);
    cbreak();
    echo();
    _clean();
    wmove(m_win, h/2, m_name.length()+1);
    curs_set(1);
    size_t len = w-m_name.length()-2;
    char str_input[len];
    wgetnstr(m_win, str_input, len);
    try {
      m_value = std::stoi(std::string(str_input));
      m_string = std::to_string(m_value);
    } catch (std::exception e) {
      m_value = 0;
      m_string = "0";
    }
    _clean();
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
    box(m_win, 0, 0);
    curs_set(0);
    raw();
    noecho();
    _draw();
    wrefresh(m_win);
  }

public:
  int m_value;

  InputBuffer(std::string name, int x, int y, int w, int h):
    Component(x, y, w, h),
    m_state(ButtonState::NORMAL),
    m_name(name + ": "),
    m_value(0),
    m_string("")
  {}

  void focus() override {
    m_focused = true;
    _draw();
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    _draw();
    wrefresh(m_win);
  }

  void mount() override {
    box(m_win, 0, 0);
    _draw();
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::INPUT && packet->type != UpdatePacket::BUTTON) return;
    if (packet->type == UpdatePacket::BUTTON) {
      ButtonUpdate *update = static_cast<ButtonUpdate *>(packet);
      m_state = update->state;
      _draw();
      wrefresh(m_win);
    } else {
      InputUpdate *update = static_cast<InputUpdate *>(packet);
      char key_in = update->key_in;
      switch (key_in) {
      case '\n':
        _get_input_str();
        break;
      default:
        break;
      }
    }
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }
};

class InputBufferHex : public Component {
private:
  std::string m_name;
  std::string m_string;
  ButtonState m_state;

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _draw() {
    ButtonState state = m_state;
    if (!m_focused) {
      state.value = ButtonState::NORMAL ;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 2, "%s", m_name.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 2+m_name.length(), "%s", m_string.c_str());
  }

  void _clean() {
    mvwprintw(m_win, h/2, 2+m_name.length(), "%s", _get_empty(w-m_name.length()-3).c_str());
  }

  void _get_input_str() {
    m_state = ButtonState::ACTIVE;
    _draw();
    wrefresh(m_win);
    cbreak();
    echo();
    _clean();
    wmove(m_win, h/2, m_name.length()+2);
    curs_set(1);
    size_t len = w-m_name.length()-4;
    char str_input[len];
    wgetnstr(m_win, str_input, len);
    try {
      std::stringstream ss;
      ss << std::hex << str_input;
      ss >> m_value;
      m_string = std::to_string(m_value);
    } catch (std::exception e) {
      m_value = 0;
      m_string = "0";
    }
    _clean();
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
    box(m_win, 0, 0);
    curs_set(0);
    raw();
    noecho();
    _draw();
    wrefresh(m_win);
  }

public:
  int m_value;

  InputBufferHex(std::string name, int x, int y, int w, int h):
    Component(x, y, w, h),
    m_state(ButtonState::NORMAL),
    m_name(name + ": "),
    m_value(0),
    m_string("")
  {}

  void focus() override {
    m_focused = true;
    _draw();
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    _draw();
    wrefresh(m_win);
  }

  void mount() override {
    box(m_win, 0, 0);
    _draw();
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::INPUT && packet->type != UpdatePacket::BUTTON) return;
    if (packet->type == UpdatePacket::BUTTON) {
      ButtonUpdate *update = static_cast<ButtonUpdate *>(packet);
      m_state = update->state;
      _draw();
      wrefresh(m_win);
    } else {
      InputUpdate *update = static_cast<InputUpdate *>(packet);
      char key_in = update->key_in;
      switch (key_in) {
      case '\n':
        _get_input_str();
        break;
      default:
        break;
      }
    }
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }
};

template <> class InputBuffer <float> : public Component {
private:
  std::string m_name;
  std::string m_string;
  ButtonState m_state;

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _draw() {
    ButtonState state = m_state;
    if (!m_focused) {
      state.value = ButtonState::NORMAL ;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1, "%s", m_name.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
  }

  void _clean() {
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", _get_empty(w-m_name.length()-3).c_str());
  }

  void _get_input_str() {
    m_state = ButtonState::ACTIVE;
    _draw();
    wrefresh(m_win);
    cbreak();
    echo();
    _clean();
    wmove(m_win, h/2, m_name.length()+1);
    curs_set(1);
    size_t len = w-m_name.length()-2;
    char str_input[len];
    wgetnstr(m_win, str_input, len);
    try {
      m_value = std::stof(std::string(str_input));
    } catch (std::exception e) {
      m_value = 0.0f;
    }
    m_string = std::to_string(m_value);
    _clean();
    mvwprintw(m_win, h/2, 1+m_name.length(), "%s", m_string.c_str());
    box(m_win, 0, 0);
    curs_set(0);
    raw();
    noecho();
    _draw();
    wrefresh(m_win);
  }

public:
  float m_value;

  InputBuffer(std::string name, int x, int y, int w, int h):
    Component(x, y, w, h),
    m_state(ButtonState::NORMAL),
    m_name(name + ": "),
    m_value(0.0f),
    m_string("")
  {}

  void focus() override {
    m_focused = true;
    _draw();
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    _draw();
    wrefresh(m_win);
  }

  void mount() override {
    box(m_win, 0, 0);
    _draw();
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::INPUT && packet->type != UpdatePacket::BUTTON) return;
    if (packet->type == UpdatePacket::BUTTON) {
      ButtonUpdate *update = static_cast<ButtonUpdate *>(packet);
      m_state = update->state;
      _draw();
      wrefresh(m_win);
    } else {
      InputUpdate *update = static_cast<InputUpdate *>(packet);
      char key_in = update->key_in;
      switch (key_in) {
      case '\n':
        _get_input_str();
        break;
      default:
        break;
      }
    }
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }
};


#endif // INPUT_HPP