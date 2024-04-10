/**
 * @file Renderable.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief
 * @version 0.1
 * @date 2024-04-08
 *
 *
 */

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <ncurses.h>

struct UpdatePacket {};

struct Component {
  WINDOW *win;
  int h;
  int w;
  int y;
  int x;
  
  Component(int h, int w, int y, int x):
    h(h), w(w), y(y), x(x)
  {
    win = newwin(h, w, y, x);
  }

  ~Component() {
    delwin(win);
  }
  
  virtual void mount() { return; }
  
  virtual void update(UpdatePacket *data) { return; }
  
  virtual void unmount() { return; }

};

#endif // COMPONENT_HPP