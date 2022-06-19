// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * THE mouse handling.
 *
 * This file contains all commands that can be assigned to function keys or typed on the command line.
 */

#include "the.h"
#include "proto.h"

/*
 * Following #defines to cater for those platforms that don't have mouse definitions in <curses.h>
 */
#define MY_BUTTON_SHIFT           0010
#define MY_BUTTON_CONTROL         0020
#define MY_BUTTON_ALT             0040

MEVENT ncurses_mouse_event;

/*   3         2         1         0
 * 210987654321098765432109876543210
 *                maaaaabbbbb
 */

/*
 * Button masks
 */
#define MOUSE_MODIFIER_MASK(x)  ((x) & 0xE0000) /* 11100000000000000000 */
#define MOUSE_ACTION_MASK(x)    ((x) & 0x1F000) /* 00011111000000000000 */
#define MOUSE_BUTTON_MASK(x)    ((x) & 0x00F80) /* 00000000111110000000 */
#define MOUSE_WINDOW_MASK(x)    ((x) & 0x0007F) /* 00000000000001111111 */
/*
 * Button modifiers
 */
#define MOUSE_MODIFIER_OFFSET   17      /* was 13 */
#define MOUSE_NORMAL            0
#define MOUSE_SHIFT             ((MY_BUTTON_SHIFT >> 3)   << MOUSE_MODIFIER_OFFSET)
#define MOUSE_CONTROL           ((MY_BUTTON_CONTROL >> 3) << MOUSE_MODIFIER_OFFSET)
#define MOUSE_ALT               ((MY_BUTTON_ALT >> 3)     << MOUSE_MODIFIER_OFFSET)
/*
 * Button actions
 */
#define MOUSE_ACTION_OFFSET     14      /* was 10 */
#define MOUSE_PRESS             (BUTTON_PRESSED << MOUSE_ACTION_OFFSET)
#define MOUSE_RELEASE           (BUTTON_RELEASED << MOUSE_ACTION_OFFSET)
#define MOUSE_DRAG              (BUTTON_MOVED << MOUSE_ACTION_OFFSET)
#define MOUSE_CLICK             (BUTTON_CLICKED << MOUSE_ACTION_OFFSET)
#define MOUSE_DOUBLE_CLICK      (BUTTON_DOUBLE_CLICKED << MOUSE_ACTION_OFFSET)
/*
 * Button numbers
 */
#define MOUSE_BUTTON_OFFSET     7
#define MOUSE_LEFT              (1 << MOUSE_BUTTON_OFFSET)
#define MOUSE_MIDDLE            (2 << MOUSE_BUTTON_OFFSET)
#define MOUSE_RIGHT             (3 << MOUSE_BUTTON_OFFSET)
#define THE_MOUSE_WHEEL_UP      (4 << MOUSE_BUTTON_OFFSET)
#define THE_MOUSE_WHEEL_DOWN    (5 << MOUSE_BUTTON_OFFSET)
#define THE_MOUSE_WHEEL_LEFT    (6 << MOUSE_BUTTON_OFFSET)
#define THE_MOUSE_WHEEL_RIGHT   (7 << MOUSE_BUTTON_OFFSET)

#define MOUSE_INFO_TO_KEY(w,b,ba,bm) ((w)|(b<<MOUSE_BUTTON_OFFSET)|(ba<<MOUSE_ACTION_OFFSET)|((bm>>3)<<MOUSE_MODIFIER_OFFSET))

static char *button_names[] = {
  "-button 0-",
  "LB",            /* left button */
  "MB",            /* middle button */
  "RB",            /* right button */
  "UW",            /* wheel up */
  "DW",            /* wheel down */
  "LW",            /* wheel left */
  "RW",            /* wheel right */
};

static char *button_modifier_names[] = {
  "",
  "S-",            /* shift */
  "C-",            /* control */
  "?",             /* unknown */
  "A-",            /* alt */
};

static char *button_action_names[] = {
  "R",             /* release */
  "P",             /* press */
  "C",             /* clicked */
  "2",             /* double clicked */
  "3",             /* triple clicked */
  "D",             /* dragged */
  "S",             /* scrolled */
};

/*
 * These two variables are saved by each mouse key press or reset to -1 when a normal key it pressed.
 */
static int last_mouse_x_pos = -1;
static int last_mouse_y_pos = -1;

void wmouse_position(WINDOW * win, int *y, int *x) {
  int begy, begx, maxy, maxx;

  /*
   * if the current mouse position is outside the provided window, put -1 in x and y
   */
  if (win == (WINDOW *) NULL) {
    *y = *x = (-1);
    return;
  }
  getbegyx(win, begy, begx);
  getmaxyx(win, maxy, maxx);
  if (begy > ncurses_mouse_event.y || begx > ncurses_mouse_event.x || begy + maxy <= ncurses_mouse_event.y || begx + maxx <= ncurses_mouse_event.x) {
    *x = *y = (-1);
  } else {
    *x = ncurses_mouse_event.x - begx;
    *y = ncurses_mouse_event.y - begy;
  }
  return;
}

short get_mouse_info(int *button, int *button_action, int *button_modifier) {
  short rc = RC_OK;

  getmouse(&ncurses_mouse_event);
  /*
   * Save the current mouse position
   */
  last_mouse_x_pos = ncurses_mouse_event.x;
  last_mouse_y_pos = ncurses_mouse_event.y;

  if (ncurses_mouse_event.bstate & BUTTON1_RELEASED || ncurses_mouse_event.bstate & BUTTON1_PRESSED || ncurses_mouse_event.bstate & BUTTON1_CLICKED || ncurses_mouse_event.bstate & BUTTON1_DOUBLE_CLICKED) {
    *button = 1;
  } else {
    if (ncurses_mouse_event.bstate & BUTTON2_RELEASED || ncurses_mouse_event.bstate & BUTTON2_PRESSED || ncurses_mouse_event.bstate & BUTTON2_CLICKED || ncurses_mouse_event.bstate & BUTTON2_DOUBLE_CLICKED) {
      *button = 2;
    } else {
      if (ncurses_mouse_event.bstate & BUTTON3_RELEASED || ncurses_mouse_event.bstate & BUTTON3_PRESSED || ncurses_mouse_event.bstate & BUTTON3_CLICKED || ncurses_mouse_event.bstate & BUTTON3_DOUBLE_CLICKED) {
        *button = 3;
      } else {
        *button = *button_action = *button_modifier = 0;
        return RC_INVALID_OPERAND;
      }
    }
  }
  if (ncurses_mouse_event.bstate & BUTTON_SHIFT) {
    *button_modifier = MY_BUTTON_SHIFT;
  } else if (ncurses_mouse_event.bstate & BUTTON_CTRL) {
    *button_modifier = MY_BUTTON_CONTROL;
  } else if (ncurses_mouse_event.bstate & BUTTON_ALT) {
    *button_modifier = MY_BUTTON_ALT;
  } else {
    *button_modifier = 0;
  }
  if (ncurses_mouse_event.bstate & BUTTON1_RELEASED || ncurses_mouse_event.bstate & BUTTON2_RELEASED || ncurses_mouse_event.bstate & BUTTON3_RELEASED) {
    *button_action = BUTTON_RELEASED;
  } else {
    if (ncurses_mouse_event.bstate & BUTTON1_PRESSED || ncurses_mouse_event.bstate & BUTTON2_PRESSED || ncurses_mouse_event.bstate & BUTTON3_PRESSED) {
      *button_action = BUTTON_PRESSED;
    } else {
      if (ncurses_mouse_event.bstate & BUTTON1_CLICKED || ncurses_mouse_event.bstate & BUTTON2_CLICKED || ncurses_mouse_event.bstate & BUTTON3_CLICKED) {
        *button_action = BUTTON_CLICKED;
      } else {
        if (ncurses_mouse_event.bstate & BUTTON1_DOUBLE_CLICKED || ncurses_mouse_event.bstate & BUTTON2_DOUBLE_CLICKED || ncurses_mouse_event.bstate & BUTTON3_DOUBLE_CLICKED) {
          *button_action = BUTTON_DOUBLE_CLICKED;
        }
      }
    }
  }
  return (rc);
}

short THEMouse(char* params) {
  int w = 0;
  char scrn = 0;
  short rc = RC_OK;
  int curr_button_action = 0;
  int curr_button_modifier = 0;
  int curr_button = 0;

  which_window_is_mouse_in(&scrn, &w);
  if (w == (-1)) {              /* shouldn't happen! */
    return (RC_OK);
  }
  rc = get_mouse_info(&curr_button, &curr_button_action, &curr_button_modifier);
  if (rc != RC_OK) {
    return (rc);
  }
  rc = execute_mouse_commands(MOUSE_INFO_TO_KEY(w, curr_button, curr_button_action, curr_button_modifier));
  return (rc);
}

void which_window_is_mouse_in(char* scrn, int *w) {
  char i = 0;
  int j = 0;
  int y = 0, x = 0;

  for (i = 0; i < display_screens; i++) {
    for (j = 0; j < VIEW_WINDOWS; j++) {
      if (screen[(byte)i].win[j] != (WINDOW *) NULL) {
        wmouse_position(screen[(byte)i].win[j], &y, &x);
        if (y != (-1) && x != (-1)) {
          *scrn = i;
          *w = j;
          return;
        }
      }
    }
  }
  /*
   * To get here, the mouse is NOT in any of the view windows; is it in the status line ?
   */
  wmouse_position(statarea, &y, &x);
  if (y != (-1) && x != (-1)) {
    *w = WINDOW_STATAREA;
    *scrn = current_screen;
    return;
  }
  /*
   * To get here, the mouse is NOT in any of the view windows; or the status line. Is it in the FILETABS window ?
   */
  wmouse_position(filetabs, &y, &x);
  if (y != (-1) && x != (-1)) {
    *w = WINDOW_FILETABS;
    *scrn = current_screen;
    return;
  }
  /*
   * To get here, the mouse is NOT in any of the view windows; or the status line, or the FILETABS window. Is it in the DIVIDER window ?
   */
  if (display_screens > 1 && !horizontal) {
    wmouse_position(divider, &y, &x);
    if (y != (-1) && x != (-1)) {
      *w = WINDOW_DIVIDER;
      *scrn = current_screen;
      return;
    }
  }
  /*
   * To get here, the mouse is NOT in ANY window. Return an error.
   */
  *w = WINDOW_ALL /* was (-1) */ ;
  return;
}

void reset_saved_mouse_pos(void) {
  last_mouse_x_pos = -1;
  last_mouse_y_pos = -1;
  return;
}

void get_saved_mouse_pos(int *y, int *x) {
  *x = last_mouse_x_pos;
  *y = last_mouse_y_pos;
  return;
}

void initialise_mouse_commands(void) {

  /*
   * Default mouse actions in FILEAREA
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_PRESS | MOUSE_SHIFT, "CURSOR MOUSE#RESET BLOCK#MARK LINE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_PRESS | MOUSE_CONTROL, "CURSOR MOUSE#RESET BLOCK#MARK BOX", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_DRAG | MOUSE_SHIFT, "CURSOR MOUSE#MARK LINE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_DRAG | MOUSE_CONTROL, "CURSOR MOUSE#MARK BOX", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_RIGHT | MOUSE_PRESS | MOUSE_SHIFT, "CURSOR MOUSE#MARK LINE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_RIGHT | MOUSE_PRESS | MOUSE_CONTROL, "CURSOR MOUSE#MARK BOX", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_RIGHT | MOUSE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE#SOS MAKECURR", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_DRAG | MOUSE_SHIFT, "CURSOR MOUSE#MARK LINE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_DRAG | MOUSE_CONTROL, "CURSOR MOUSE#MARK BOX", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_DOUBLE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE#SOS EDIT", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_LEFT | MOUSE_CLICK | MOUSE_ALT, "BACKWARD", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILEAREA | MOUSE_RIGHT | MOUSE_CLICK | MOUSE_ALT, "FORWARD", FALSE, FALSE, 0);
  /*
   * Default mouse actions in PREFIX area
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_PREFIX | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_PREFIX | MOUSE_RIGHT | MOUSE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE#SOS MAKECURR", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_PREFIX | MOUSE_LEFT | MOUSE_DOUBLE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE#SOS EDIT", FALSE, FALSE, 0);
  /*
   * Default mouse actions in COMMAND line
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_COMMAND | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "CURSOR MOUSE", FALSE, FALSE, 0);
  /*
   * Default mouse actions in STATAREA
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_STATAREA | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "STATUS", FALSE, FALSE, 0);
  /*
   * Default mouse actions in FILETABS
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_FILETABS | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "TABFILE", FALSE, FALSE, 0);
  /*
   * Default mouse actions in IDLINE
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_IDLINE | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "XEDIT", FALSE, FALSE, 0);
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_IDLINE | MOUSE_RIGHT | MOUSE_CLICK | MOUSE_NORMAL, "XEDIT -", FALSE, FALSE, 0);
  /*
   * Default mouse actions in DIVIDER
   */
  add_define(&first_mouse_define, &last_mouse_define, WINDOW_DIVIDER | MOUSE_LEFT | MOUSE_CLICK | MOUSE_NORMAL, "SCREEN 1", FALSE, FALSE, 0);

  return;
}

int mouse_info_to_key(int w, int button, int button_action, int button_modifier) {
  return (MOUSE_INFO_TO_KEY(w, button, button_action, button_modifier));
}

char* mouse_key_number_to_name(int key_number, char* key_name, int* shift) {
  register int i = 0;
  int w = 0, b = 0, ba = 0, bm = 0;
  char* win_name = "*** unknown ***";

  w = MOUSE_WINDOW_MASK(key_number);
  b = (MOUSE_BUTTON_MASK(key_number) >> MOUSE_BUTTON_OFFSET);
  ba = (MOUSE_ACTION_MASK(key_number) >> MOUSE_ACTION_OFFSET);
  bm = (MOUSE_MODIFIER_MASK(key_number) >> MOUSE_MODIFIER_OFFSET);
  *shift = bm;
  if (w == WINDOW_ALL) {
    win_name = "*";
  } else {
    for (i = 0; i < ATTR_MAX; i++) {
      if (w == valid_areas[i].area_window && valid_areas[i].actual_window) {
        win_name = valid_areas[i].area;
        break;
      }
    }
  }
  sprintf(key_name, "%s%s%s in %s", button_modifier_names[bm], button_action_names[ba], button_names[b], win_name);
  return (key_name);
}

/* Function:
 *   find the matching mouse key value for the supplied name
 * Parameters:
 *   mnemonic: the key name to be matched
 *   win_name: the window to be matched
 * Returns:
 *   the mouse button, action and modifier or -1 if error
 */
int find_mouse_key_value(char* mnemonic) {
  int key = 0, len = 0;
  int b = 0, ba = 0, bm = 0;
  char tmp_buf[6];

  /*
   * Parse the mnemonic for a valid mouse key definition...
   */
  len = strlen(mnemonic);
  if (len == 3) {
    strcpy(tmp_buf, "N-");
    strcat(tmp_buf, (char *) mnemonic);
  } else {
    if (len == 5) {
      strcpy(tmp_buf, (char *) mnemonic);
    } else {
      display_error(1, mnemonic, FALSE);
      return (-1);
    }
  }
  if (tmp_buf[1] != '-' || (tmp_buf[4] != 'B' && tmp_buf[4] != 'b')) {
    display_error(1, mnemonic, FALSE);
    return (-1);
  }
  /*
   * Validate button modifier
   */
  switch (tmp_buf[0]) {

    case 'N':
    case 'n':
      bm = 0;
      break;

    case 'S':
    case 's':
      bm = MOUSE_SHIFT;
      break;

    case 'C':
    case 'c':
      bm = MOUSE_CONTROL;
      break;

    case 'A':
    case 'a':
      bm = MOUSE_ALT;
      break;

    default:
      display_error(1, mnemonic, FALSE);
      return (-1);
      break;
  }
  /*
   * Validate button action
   */
  switch (tmp_buf[2]) {

    case 'P':
    case 'p':
      ba = MOUSE_PRESS;
      break;

    case 'C':
    case 'c':
      ba = MOUSE_CLICK;
      break;

    case 'R':
    case 'r':
      ba = MOUSE_RELEASE;
      break;

    case '2':
      ba = MOUSE_DOUBLE_CLICK;
      break;

    case 'D':
    case 'd':
      ba = MOUSE_DRAG;
      break;

    default:
      display_error(1, mnemonic, FALSE);
      return (-1);
      break;
  }
  /*
   * Validate button number
   */
  switch (tmp_buf[3]) {
    case 'L':
    case 'l':
      b = MOUSE_LEFT;
      break;

    case 'R':
    case 'r':
      b = MOUSE_RIGHT;
      break;

    case 'M':
    case 'm':
      b = MOUSE_MIDDLE;
      break;

    default:
      display_error(1, mnemonic, FALSE);
      return (-1);
      break;
  }
  key = b | ba | bm;
  return (key);
}

/* Function:
 *   find the matching mouse key value for the supplied name in the specified window.
 * Parameters:
 *   mnemonic: the key name to be matched
 *   win_name: the window to be matched
 * Returns:
 *   the mouse button, action, modifier and window or -1 if error.
 */
int find_mouse_key_value_in_window(char* mnemonic, char* win_name) {
  register short i = 0;
  int w = (-1), key = 0;
  int mb;

  /*
   * Parse the mnemonic for a valid mouse key definition...
   */
  mb = find_mouse_key_value(mnemonic);
  if (mb == (-1)) {
    return mb;
  }
  /*
   * Find a valid window name for win_name...
   */
  for (i = 0; i < ATTR_MAX; i++) {
    if (equal(valid_areas[i].area, win_name, valid_areas[i].area_min_len)) {
      w = valid_areas[i].area_window;
      break;
    }
  }
  if (w == (-1)) {
    display_error(1, win_name, FALSE);
    return (-1);
  }
  key = w | mb;
  return (key);
}
