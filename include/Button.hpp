#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "Component.hpp"

class Button : public Component {
private:
  std::string m_text;
  ButtonState m_state;

  void _draw() {
    ButtonState state = m_state;
    if (!m_focused) {
      state = ButtonState::NORMAL;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, (w-m_text.length())/2, "%s", m_text.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
  }

public:
  Button(int x, int y, int w, int h, std::string text):
    Component(x, y, w, h),
    m_text(text),
    m_state(ButtonState::NORMAL)
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
    if (packet->type != UpdatePacket::BUTTON) return;
    ButtonUpdate *update = static_cast<ButtonUpdate *>(packet);
    m_state = update->state;
    _draw();
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }

};

#endif // BUTTON_HPP