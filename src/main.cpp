#include <signal.h>
#include <ncurses.h>

#include <map>
#include <string>
#include <vector>

#include "../include/Component.hpp"

int main(int argc, char **argv) {
  initscr();
  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  noecho();
  refresh();

  init_colors();
  paint_scr();

  char pause = getch();

  Menu menu(0, 0, COLS, 20);
  menu.mount();

  InputUpdate packet(getch());

  endwin();
  return 0;
}