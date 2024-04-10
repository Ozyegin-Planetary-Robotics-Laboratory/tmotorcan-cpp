/**
 * @file Color.hpp
 * @author Toprak Efe Akkılıç (efe.akkilic@ozu.edu.tr)
 * @brief Header to include ncurses color palettes.
 * @version 0.1
 * @date 2024-04-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef COLOR_HPP
#define COLOR_HPP

#include <ncurses.h>

#define NORMAL_PAIR 0
#define CURSOR_PAIR 1
#define SELECTED_PAIR 2
#define HIGHLIGHT_PAIR 3
#define WARNING_PAIR 4

void init_colors() {
  start_color();
  init_pair(NORMAL_PAIR, COLOR_CYAN, COLOR_BLACK);
  init_pair(CURSOR_PAIR, COLOR_WHITE, COLOR_RED);
  init_pair(SELECTED_PAIR, COLOR_WHITE, COLOR_BLUE);
  init_pair(HIGHLIGHT_PAIR, COLOR_CYAN, COLOR_BLACK);
  init_pair(WARNING_PAIR, COLOR_GREEN, COLOR_RED);
}

#endif // COLOR_HPP