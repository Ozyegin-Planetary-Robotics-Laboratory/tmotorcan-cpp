#include <ncurses.h>

#include <map>
#include <string>
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
    INPUT
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
    mvwprintw(m_win, 0, 0, msg.c_str());
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
    mvwprintw(m_win, h/2, (w-m_text.length())/2, m_text.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    wrefresh(m_win);
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
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }

};

// Inputs
struct InputUpdate : public UpdatePacket {
  char key_in;

  InputUpdate(char key_in):
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
      state.value = ButtonState::NORMAL;
    }
    wattron(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1, m_name.c_str());
    wattroff(m_win, COLOR_PAIR(state.value));
    mvwprintw(m_win, h/2, 1+m_name.length(), m_string.c_str());
    wrefresh(m_win);
  }

  void _clean() {
    mvwprintw(m_win, h/2, 1+m_name.length(), _get_empty(w-m_name.length()-2).c_str());
    wrefresh(m_win);
  }

  void _get_input_str() {
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
    m_string = std::string(str_input);
    curs_set(0);
    raw();
    noecho();
  }

public:
  T m_value;

  InputBuffer(std::string name, int x, int y, int w, int h):
    Component(x, y, w, h),
    m_state(ButtonState::NORMAL),
    m_name(name + ":"),
    m_string("")
  {}

  void focus() override {
    m_focused = true;
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    _draw();
  }

  void mount() override {
    box(m_win, 0, 0);
    _draw();
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::INPUT) return;
    InputUpdate *update = static_cast<InputUpdate *>(packet);
    char key_in = update->key_in;
    switch (key_in) {
    case '\n':
      _get_input_str();
      break;
    default:
      break;
    }

    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }
};

// First Menu
class Menu : public Component {
  std::vector <std::shared_ptr<Component>> m_buttons;
  int m_active_field;

public:
  Menu(int x, int y, int w, int h):
    Component(x, y, w, h),
    m_buttons({
      std::make_shared<Button>(Button(1, (w*3)/9, w/9, 3, "Button 1")),
      std::make_shared<Button>(Button(1, (w*5)/9, w/9, 3, "Button 2")),
      std::make_shared<Button>(Button(1, (w*7)/9, w/9, 3, "Button 3")),
      std::make_shared<InputBuffer <double>>(InputBuffer <double> ("Motor ID", 1, w/9, w/9, 3))
    }),
    m_active_field(0)
  {
  }

  void focus() override {
    m_focused = true;
    for (auto button : m_buttons) {
      button->focus();
    }
    wrefresh(m_win);
  }

  void unfocus() override {
    m_focused = false;
    for (auto button : m_buttons) {
      button->unfocus();
    }
    wrefresh(m_win);
  }

  void mount() override {
    box(m_win, 0, 0);
    wrefresh(m_win);
    for (auto button : m_buttons) {
      button->mount();
    }
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    static ButtonUpdate button_normal(ButtonState::NORMAL);
    static ButtonUpdate button_hover(ButtonState::HOVER);
    static ButtonUpdate button_active(ButtonState::ACTIVE);

    if (packet->type != UpdatePacket::INPUT) return;
    InputUpdate *update = static_cast<InputUpdate *>(packet);

    char key_in = update->key_in;
    switch (key_in) {
      case KEY_RIGHT:
        m_buttons[m_active_field]->update(&button_normal);
        m_active_field = (m_active_field + 1) % (m_buttons.size()+1);
        m_buttons[m_active_field]->update(&button_hover);
        break;
      case KEY_LEFT:
        m_buttons[m_active_field]->update(&button_normal);
        m_active_field = (m_active_field - 1) % (m_buttons.size()+1);
        m_buttons[m_active_field]->update(&button_hover);
        break;
      default:
        break;
    }
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    for (auto &button : m_buttons) {
      button->unmount();
    }
    wrefresh(m_win);
  }
};