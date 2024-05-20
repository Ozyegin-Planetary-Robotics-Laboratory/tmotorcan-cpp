#ifndef MENU_HPP
#define MENU_HPP

#include "ak60.hpp"
#include "Component.hpp"
#include "Input.hpp"

class Menu : public Component {
  std::shared_ptr<TMotor::AK60Manager> m_manager;
  typedef std::shared_ptr<Component> ComponentPtr;
  std::vector<std::vector <ComponentPtr>> m_buttons;
  int m_cursor_index[2];
  int m_active_index[2];
  bool *m_shutdown_ptr;
  bool m_locked;
  std::thread m_command_thread;
  float m_gear_ratio;
  
  ComponentPtr _get_curs_button() {
    return m_buttons[m_cursor_index[0]][m_cursor_index[1]];
  }

  ComponentPtr _get_active_button() {
    if (m_active_index[0] == -1) return nullptr;
    return m_buttons[m_active_index[0]][m_active_index[1]];
  }

  inline void _delegate_command() {
    m_manager->sendPositionVelocityAcceleration(
      static_cast<InputBuffer <float> *> (m_buttons[1][0].get())->m_value*m_gear_ratio,
      static_cast<InputBuffer <float> *> (m_buttons[1][1].get())->m_value*m_gear_ratio,
      static_cast<InputBuffer <float> *> (m_buttons[1][2].get())->m_value*m_gear_ratio
    );
  }

public:
  Menu(int x, int y, int w, int h, bool *shutdown_ptr, std::shared_ptr<TMotor::AK60Manager> &manager, float gear_ratio) :
    Component(x, y, w, h),
    m_buttons{{
      std::make_shared <Button>                  (x+(w*1)/7, y+1, w/7, 3, "Permanent"),
      std::make_shared <Button>                  (x+(w*3)/7, y+1, w/7, 3, "Temporary"),
      std::make_shared <Button>                  (x+(w*5)/7, y+1, w/7, 3, "Restore")
      },
      {
      std::make_shared <InputBuffer <float>>     ("Pos", x+(w*1)/7, y+5, w/7, 3),
      std::make_shared <InputBuffer <float>>     ("Vel", x+(w*3)/7, y+5, w/7, 3),
      std::make_shared <InputBuffer <float>>     ("Acc", x+(w*5)/7, y+5, w/7, 3),
      },
      {
      std::make_shared <Button>                  (x+(w*1)/7, y+9, w/7, 3, "Send")
      }
     },
    m_cursor_index{0, 0},
    m_active_index{-1, -1},
    m_shutdown_ptr(shutdown_ptr),
    m_manager(manager),
    m_locked(false),
    m_gear_ratio(gear_ratio)
  {}

  void focus() override {
    m_focused = true;
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->focus();
      }
    }
  }

  void unfocus() override {
    m_focused = false;
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->unfocus();
      }
    }
  }

  void mount() override {
    box(m_win, 0, 0);
    mvwprintw(m_win, 2, w/7-16, "Origin Actions:");
    mvwprintw(m_win, 6, w/7-16, "Setpoint:");
    wrefresh(m_win);
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->mount();
      }
    }
    ButtonUpdate hover_state(ButtonState::HOVER);
    m_buttons[m_cursor_index[0]][m_cursor_index[1]]->update(&hover_state);
  }

  void update(UpdatePacket *packet) override {
    static ButtonUpdate button_normal(ButtonState::NORMAL);
    static ButtonUpdate button_hover(ButtonState::HOVER);
    static ButtonUpdate button_active(ButtonState::ACTIVE);

    if (packet->type != UpdatePacket::INPUT) return;
    InputUpdate *update = static_cast<InputUpdate *>(packet);

    int key_in = update->key_in;
    // if (m_locked && key_in != '\n') {
    //   return;
    // }
    switch (key_in) {
      case KEY_RIGHT:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[1] = (m_cursor_index[1] + 1) % (m_buttons[m_cursor_index[0]].size());
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_LEFT:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[1]--;
        if (m_cursor_index[1] == -1) { // Modulos doesn't map -1.
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        } else {
          m_cursor_index[1] %= m_buttons[m_cursor_index[0]].size();
        }
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_UP:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[0]--;
        if (m_cursor_index[0] == -1) { // Modulos doesn't map -1.
          m_cursor_index[0] = m_buttons.size()-1;
        } else {
          m_cursor_index[0] %= m_buttons.size();
        }

        // Second index safeguard
        if (m_cursor_index[1] > m_buttons[m_cursor_index[0]].size()-1) {
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        }
        if (m_cursor_index[1] < 0) {
          m_cursor_index[1] = 0;
        }
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_DOWN:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[0]++;
        if (m_cursor_index[0] == -1) { // Modulos doesn't map -1.
          m_cursor_index[0] = m_buttons.size()-1;
        } else {
          m_cursor_index[0] %= m_buttons.size();
        }

        // Second index safeguard
        if (m_cursor_index[1] > m_buttons[m_cursor_index[0]].size()-1) {
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        }
        if (m_cursor_index[1] < 0) {
          m_cursor_index[1] = 0;
        }
        _get_curs_button()->update(&button_hover);
        break;
      case '\n':
        if (m_cursor_index[0] == 1) {             // Input Buffers
          _get_curs_button()->update(packet);
        } else {                                  // Buttons
          _get_curs_button()->update(&button_active);
          if (m_cursor_index[0] == 0) {           // Origin Actions
            if (m_cursor_index[1] == 0) {         // Permanent
              m_manager->setOrigin(TMotor::MotorOriginMode::PERMANENT);
            }
            if (m_cursor_index[1] == 1) {
              m_manager->setOrigin(TMotor::MotorOriginMode::TEMPORARY);
            }
            if (m_cursor_index[1] == 2) {
              m_manager->setOrigin(TMotor::MotorOriginMode::RESTORE);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            _get_curs_button()->update(&button_hover);
          }
          if (m_cursor_index[0] == 2) {           // Send
            if (m_locked) {                       // Deactivate send
              m_locked = false;
              m_command_thread.join();
              _get_curs_button()->update(&button_hover);
            } else {                              // Activate send
              m_locked = true;
              m_command_thread = std::thread([this] {
                while (m_locked) {
                  _delegate_command();
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
              });
              _get_curs_button()->update(&button_active);
            }
          }          
        }
        break;
      default:
        break;
    }
    wrefresh(m_win);
  }

  void unmount() override {
    m_locked = false;
    if (m_command_thread.joinable()) m_command_thread.join();
    werase(m_win);
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->unmount();
      }
    }
    wrefresh(m_win);
  }
};

#endif // MENU_HPP