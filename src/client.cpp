#include "ak60.hpp"
#include <ncurses.h>
#include <csignal>

enum Mode: uint8_t {
  ORIGIN_MODE = 0,
  DUTY_MODE,
  CURRENT_MODE,
  CURRENT_BRAKE_MODE,
  VELOCITY_MODE,
  POSITION_MODE,
  POSITION_VEL_ACC_MODE
};

enum WindowMode: uint8_t {
  NO_MENU = 0,
  DUTY_MENU,
  CURRENT_MENU,
  CURRENT_BRAKE_MENU,
  VELOCITY_MENU,
  POSITION_MENU,
  POSITION_VEL_ACC_MENU
};

enum ControlKeys {
  QUIT_KEY = 'q',
  MODE_KEY = 'm',
  ENTER = '\n'
};

// Global flag to indicate if the CTRL+C signal was received
volatile sig_atomic_t g_signalReceived = false;

// Signal handler function for the CTRL+C signal
void signalHandler(int signal) {
  if (signal == SIGINT) {
    g_signalReceived = true;
  }
}

void getMotorID(uint8_t &motorID) {
  mvprintw(0, 0, "Enter Motor ID: ");
  scanw("%d", &motorID);
}

void RenderModes(uint8_t motorID, int modeSelection) {
  mvprintw(0, 0, "AK60 Motor Controller - ID: %d", motorID);
  mvprintw(1, 0, "----------------------");
  mvprintw(2, 0, "Mode Selection");
  mvprintw(3, 0, "----------------------");
  if (modeSelection == ORIGIN_MODE) {
    mvprintw(4, 0, "0. Origin Mode <-");
  } else {
    mvprintw(4, 0, "0. Origin Mode");
  }
  if (modeSelection == DUTY_MODE) {
    mvprintw(5, 0, "1. Duty Mode <-");
  } else {
    mvprintw(5, 0, "1. Duty Mode");
  }
  if (modeSelection == CURRENT_MODE) {
    mvprintw(6, 0, "2. Current Mode <-");
  } else {
    mvprintw(6, 0, "2. Current Mode");
  }
  if (modeSelection == CURRENT_BRAKE_MODE) {
    mvprintw(7, 0, "3. Current Brake Mode <-");
  } else {
    mvprintw(7, 0, "3. Current Brake Mode");
  }
  if (modeSelection == VELOCITY_MODE) {
    mvprintw(8, 0, "4. Velocity Mode <-");
  } else {
    mvprintw(8, 0, "4. Velocity Mode");
  }
  if (modeSelection == POSITION_MODE) {
    mvprintw(9, 0, "5. Position Mode <-");
  } else {
    mvprintw(9, 0, "5. Position Mode");
  }
  if (modeSelection == POSITION_VEL_ACC_MODE) {
    mvprintw(10, 0, "6. Position Velocity Acceleration Mode <-");
  } else {
    mvprintw(10, 0, "6. Position Velocity Acceleration Mode");
  }
  mvprintw(11, 0, "----------------------");
  mvprintw(12, 0, "Press 'q' to quit");
  mvprintw(13, 0, "----------------------");
  mvprintw(14, 0, "Current Mode: %d", modeSelection);
}

void renderMenu(int menuSelection) {
  return;
}

int main() {
  initscr();
  keypad(stdscr, TRUE);

  signal(SIGINT, signalHandler);

  uint8_t motorID = 0;
  getMotorID(motorID);
  clear();
  raw();
  noecho();

  int modeSelection = ORIGIN_MODE;
  int menuSelection = NO_MENU;
  int ch_in;

  while (true) {
    /* Render */
    RenderModes(motorID, modeSelection);


    /* Key input. */
    ch_in = getch();
    switch (ch_in)
    {
    case ControlKeys::QUIT_KEY:
      goto end;
      break;
    case KEY_RIGHT:
      modeSelection = (modeSelection + 1) % 7;
      break;
    case KEY_LEFT:
      modeSelection = (modeSelection - 1) % 7;
      if (modeSelection < 0) {
        modeSelection = 6;
      }
      break;
    case ENTER:
      menuSelection = modeSelection;
      break;
    default:
      break;
    }

    /* Clear*/
    clear();
  }

  end:
  endwin();

  return 0;
}