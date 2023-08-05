// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* CURSOR commands                                          */

/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */

#include "the.h"
#include "proto.h"

short THEcursor_cmdline(char_t curr_screen, VIEW_DETAILS *curr_view, short col) {
  short rc = RC_OK;

  /*
   * If in READV CMDLINE, return without doing anything
   */
  if (in_readv) {
    return (RC_OK);
  }
  /*
   * If CMDLINE is OFF return without doing anything.
   */
  if (SCREEN_WINDOW_COMMAND(curr_screen) == (WINDOW *) NULL) {
    return (rc);
  }
  if (curr_view->current_window != WINDOW_COMMAND) {
    curr_view->previous_window = curr_view->current_window;
    post_process_line(curr_view, curr_view->focus_line, (LINE *) NULL, TRUE);
    curr_view->current_window = WINDOW_COMMAND;
  }
  wmove(SCREEN_WINDOW(curr_screen), 0, col - 1);
  curr_view->cmdline_col = col - 1;
  cmd_verify_col = 1;
  return (rc);
}

short THEcursor_column(void) {
  short rc = RC_OK;
  unsigned short y = 0;

  /*
   * If in READV CMDLINE, return without doing anything
   */
  if (in_readv) {
    return (RC_OK);
  }
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      rc = THEcursor_home(current_screen, CURRENT_VIEW, FALSE);
      break;

    case WINDOW_PREFIX:
      y = getcury(CURRENT_WINDOW);
      CURRENT_VIEW->current_window = WINDOW_FILEAREA;
      wmove(CURRENT_WINDOW, y, 0);
      break;
  }
  rc = execute_move_cursor(current_screen, CURRENT_VIEW, CURRENT_VIEW->current_column - 1);
  return (rc);
}

short THEcursor_down(char_t curr_screen, VIEW_DETAILS *curr_view, short escreen) {
  short rc = RC_OK;
  short x;

  /*
   * If in READV CMDLINE, return without doing anything
   */
  if (in_readv) {
    return (RC_OK);
  }
  switch (curr_view->current_window) {

    case WINDOW_PREFIX:
    case WINDOW_FILEAREA:
      rc = scroll_line(curr_screen, curr_view, DIRECTION_FORWARD, 1L, FALSE, escreen);
      if (rc == RC_OK && escreen == CURSOR_CUA) {
        x = getcurx(SCREEN_WINDOW_FILEAREA(curr_screen));
        if (x + curr_view->verify_col > min(rec_len, curr_view->verify_end)) {
          rc = execute_move_cursor(curr_screen, curr_view, rec_len);
        }
      }
      break;

    case WINDOW_COMMAND:
      /*
       * Cycle forward  through the command list or tab to first line.
       */
      if (CMDARROWSTABCMDx) {
        rc = Sos_topedge((char_t *) "");
      } else {
        rc = Retrieve((char_t *) "+");
      }
      break;

    default:
      display_error(2, (char_t *) "", FALSE);
      break;
  }
  return (rc);
}

short THEcursor_file(bool show_errors, line_t line, length_t col) {
  short rc = RC_OK;
  short y = 0, x = 0;

  /*
   * If in READV CMDLINE, return without doing anything.
   */
  if (in_readv) {
    return (RC_OK);
  }
  /*
   * If line is not in display, error.
   */
  if (!line_in_view(current_screen, line)) {
    if (show_errors) {
      display_error(63, (char_t *) "", FALSE);
    }
    return (RC_INVALID_OPERAND);
  }
  /*
   * If column is not 0 and not in display, error.
   */
  if (col == 0) {
    x = 1;
  } else {
    if (!column_in_view(current_screen, col - 1)) {
      if (show_errors) {
        display_error(63, (char_t *) "", FALSE);
      }
      return (RC_INVALID_OPERAND);
    }
    x = (length_t) ((line_t) col - (line_t) CURRENT_VIEW->verify_col + 1);
  }
  y = get_row_for_focus_line(current_screen, line, CURRENT_VIEW->current_row);
  rc = THEcursor_move(current_screen, CURRENT_VIEW, show_errors, TRUE, (short) (y + 1), x);
  return (rc);
}

short THEcursor_home(char_t curr_screen, VIEW_DETAILS *curr_view, bool save) {
  char_t last_win = 0;
  unsigned short x = 0, y = 0;
  short rc = RC_OK;

  /*
   * If CMDLINE is OFF or in READV CMDLINE, return without doing anything
   */
  if (SCREEN_WINDOW_COMMAND(curr_screen) == (WINDOW *) NULL || in_readv) {
    return (rc);
  }
  last_win = curr_view->previous_window;
  curr_view->previous_window = curr_view->current_window;
  if (curr_view->current_window == WINDOW_COMMAND) {
    if (!line_in_view(curr_screen, curr_view->focus_line)) {
      curr_view->focus_line = curr_view->current_line;
    }
    pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
    if (save) {
      curr_view->current_window = last_win;
    } else {
      curr_view->current_window = WINDOW_FILEAREA;
    }
    getyx(SCREEN_WINDOW(curr_screen), y, x);
    y = get_row_for_focus_line(curr_screen, curr_view->focus_line, curr_view->current_row);
    wmove(SCREEN_WINDOW(curr_screen), y, x);
  } else {
    post_process_line(curr_view, curr_view->focus_line, (LINE *) NULL, TRUE);
    curr_view->current_window = WINDOW_COMMAND;
    wmove(SCREEN_WINDOW(curr_screen), 0, 0);
  }
  build_screen(curr_screen);
  return (rc);
}

short THEcursor_left(short escreen, bool kedit_defaults) {
  unsigned short x = 0, y = 0;
  short rc = RC_OK;

  // If escreen is CURSOR_ESCREEN or CURSOR_CUA, then scrolling of the
  // window will be done if possible.

  /*
   * The following should be a temporary fix for KEDIT compatibility...
   */
  if (CURRENT_VIEW->prefix && kedit_defaults) {
    escreen = CURSOR_SCREEN;
  }
  getyx(CURRENT_WINDOW, y, x);
  /*
   * For all windows, if we are not at left column, move 1 pos to left.
   */
  if (x > 0) {
    wmove(CURRENT_WINDOW, y, x - 1);
    return (RC_OK);
  }
  /*
   * We are at the left edge of the window
   * For all windows, determine if CMDARROWSTABLRx is set for tabbing or
   * scrolling and act accordingly.
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      /*
       * For CUA interface, at the left column of the file, move the cursor
       * to the last character of the previous line; that line
       * which is in scope
       */
      if (escreen == CURSOR_CUA && CURRENT_VIEW->verify_col == 1) {
        rc = scroll_line(current_screen, CURRENT_VIEW, DIRECTION_BACKWARD, 1L, FALSE, escreen);
        rc = Sos_endchar((char_t *) "");
        break;
      }
      if (escreen == CURSOR_SCREEN) {
        /*
         * Move the cursor into the prefix area, to the right-most
         * column, then move the cursor to the last column of the
         * window, either PREFIX (if ON) or FILEAREA
         */
        if (CURRENT_VIEW->prefix) {
          rc = Sos_prefix((char_t *) "");
        }
        rc = Sos_lastcol((char_t *) "");
      } else {
        if (CURRENT_VIEW->verify_col != 1 && CURRENT_VIEW->autoscroll != 0) {
          length_t curr_col = x + CURRENT_VIEW->verify_col - 1;
          length_t num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2 : CURRENT_VIEW->autoscroll;
          num_cols = min(num_cols, CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
          if (num_cols >= CURRENT_VIEW->verify_col) {
            CURRENT_VIEW->verify_col = 1;
          } else {
            CURRENT_VIEW->verify_col = CURRENT_VIEW->verify_col - num_cols;
          }
          build_screen(current_screen);
          display_screen(current_screen);
          wmove(CURRENT_WINDOW, y, curr_col - CURRENT_VIEW->verify_col);
        } else {
          if (compatible_feel == COMPAT_KEDIT || compatible_feel == COMPAT_KEDITW) {
            if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
              rc = Sos_prefix((char_t *) "");
            }
            rc = Sos_lastcol((char_t *) "");
          }
        }
      }
      break;

    case WINDOW_COMMAND:
      if (cmd_verify_col != 1 && CURRENT_VIEW->autoscroll != 0) {
        length_t curr_col = x + cmd_verify_col - 1;
        length_t num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_COMMAND] / 2 : CURRENT_VIEW->autoscroll;
        num_cols = min(num_cols, CURRENT_SCREEN.cols[WINDOW_COMMAND]);
        if (num_cols >= cmd_verify_col) {
          cmd_verify_col = 1;
        } else {
          cmd_verify_col = cmd_verify_col - num_cols;
        }
        display_cmdline(current_screen, CURRENT_VIEW);
        wmove(CURRENT_WINDOW, y, curr_col - cmd_verify_col);
      } else {
        if (escreen == CURSOR_SCREEN) {
          rc = Sos_rightedge((char_t *) "");
        }
      }
      break;

    case WINDOW_PREFIX:
      if ((escreen == CURSOR_ESCREEN && (CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_RIGHT) || escreen == CURSOR_SCREEN) {
        rc = Sos_rightedge((char_t *) "");
      }
      break;

    default:
      break;
  }
  return (rc);
}

short THEcursor_right(short escreen, bool kedit_defaults) {
  unsigned short x = 0, y = 0, tempx = 0;
  col_t right_column = 0;
  short rc = RC_OK;

  /*
   * The following should be a temporary fix for KEDIT compatibility...
   */
  if (CURRENT_VIEW->prefix && kedit_defaults) {
    escreen = CURSOR_SCREEN;
  }
  getyx(CURRENT_WINDOW, y, x);
  right_column = getmaxx(CURRENT_WINDOW) - 1;
  if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    /*
     * Check for going past end of line - max_line_length
     */
    if (CURRENT_VIEW->verify_col + x + 1 > max_line_length) {
      return (RC_OK);
    }
    /*
     * For CUA interface, if after the right-most character of the line,
     * move the cursor to the first column of the next line; that line
     * which is in scope
     */
    if (escreen == CURSOR_CUA && x + CURRENT_VIEW->verify_col > min(rec_len, CURRENT_VIEW->verify_end)) {
      rc = scroll_line(current_screen, CURRENT_VIEW, DIRECTION_FORWARD, 1L, FALSE, escreen);
      rc = Sos_firstcol((char_t *) "");
      return (rc);
    }
  }
  /*
   * For all windows, if we are not at right column, move 1 pos to right.
   */
  if (x < right_column) {
    wmove(CURRENT_WINDOW, y, x + 1);
    return (RC_OK);
  }
  /*
   * For all windows, determine if CMDARROWSTABLRx is set for tabbing or
   * scrolling and act accordingly.
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      if (escreen == CURSOR_SCREEN) {
        if (CURRENT_VIEW->prefix) {
          rc = Sos_prefix((char_t *) "");
        } else {
          wmove(CURRENT_WINDOW, y, 0);  /* this should move down a line too */
        }
      } else {
        tempx = getmaxx(CURRENT_WINDOW);
        if (x == tempx - 1 && CURRENT_VIEW->autoscroll != 0) {
          length_t curr_col = x + CURRENT_VIEW->verify_col - 1;
          length_t num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2 : CURRENT_VIEW->autoscroll;
          num_cols = min(num_cols, CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
          CURRENT_VIEW->verify_col += num_cols;
          build_screen(current_screen);
          display_screen(current_screen);
          wmove(CURRENT_WINDOW, y, curr_col - CURRENT_VIEW->verify_col + 2);
        }
      }
      break;

    case WINDOW_PREFIX:
      rc = Sos_leftedge((char_t *) "");
      break;

    case WINDOW_COMMAND:
      tempx = getmaxx(CURRENT_WINDOW);
      if (x == tempx - 1 && CURRENT_VIEW->autoscroll != 0) {
        length_t curr_col = x + cmd_verify_col - 1;
        length_t num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_COMMAND] / 2 : CURRENT_VIEW->autoscroll;
        num_cols = min(num_cols, CURRENT_SCREEN.cols[WINDOW_COMMAND]);
        cmd_verify_col += num_cols;
        display_cmdline(current_screen, CURRENT_VIEW);
        wmove(CURRENT_WINDOW, y, curr_col - cmd_verify_col + 2);
      }
      break;

    default:
      break;
  }
  return (rc);
}

short THEcursor_up(short escreen) {
  short rc = RC_OK;
  short x;
  char_t *current_command = NULL;

  /*
   * If in READV CMDLINE, return without doing anything
   */
  if (in_readv) {
    return (RC_OK);
  }
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
    case WINDOW_PREFIX:
      rc = scroll_line(current_screen, CURRENT_VIEW, DIRECTION_BACKWARD, 1L, FALSE, escreen);
      if (rc == RC_OK && escreen == CURSOR_CUA) {
        x = getcurx(CURRENT_WINDOW_FILEAREA);
        if (x + CURRENT_VIEW->verify_col > min(rec_len, CURRENT_VIEW->verify_end)) {
          rc = execute_move_cursor(current_screen, CURRENT_VIEW, rec_len);
        }
      }
      break;

    case WINDOW_COMMAND:
      /*
       * Cycle backward through the command list or tab to last line.
       */
      if (CMDARROWSTABCMDx) {
        rc = Sos_bottomedge((char_t *) "");
      } else {
        current_command = get_next_command(DIRECTION_FORWARD, 1);
        wmove(CURRENT_WINDOW_COMMAND, 0, 0);
        my_wclrtoeol(CURRENT_WINDOW_COMMAND);
        if (current_command != (char_t *) NULL) {
          Cmsg(current_command);
        }
      }
      break;

    default:
      display_error(2, (char_t *) "", FALSE);
      rc = RC_INVALID_OPERAND;
      break;
  }
  return (rc);
}

short THEcursor_move(char_t curr_screen, VIEW_DETAILS *curr_view, bool show_errors, bool escreen, short row, short col) {
  register int i = 0;
  short rc = RC_OK;
  unsigned short x = 0, y = 0;
  unsigned short max_row = 0, min_row = 0, max_col = 0;
  short idx = (-1);

  getyx(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
  /*
   * Always post_process_line() for CURRENT_VIEW
   */
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (escreen) {
    /*
     * For CURSOR ESCREEN...
     */
    if (find_last_focus_line(curr_screen, &max_row) != RC_OK) {
      return (rc);
    }
    if (find_first_focus_line(curr_screen, &min_row) != RC_OK) {
      return (rc);
    }
    if (row == 0) {
      row = y;
    } else {
      if (row > max_row) {
        row = max_row;
      } else {
        if (row < min_row) {
          row = min_row;
        } else {
          if (screen[curr_screen].sl[row - 1].main_enterable) {
            row--;
          } else {
            if (show_errors) {
              display_error(63, (char_t *) "", FALSE);
            }
            return (RC_TOF_EOF_REACHED);        /* this is a strange RC :-( */
          }
        }
      }
    }
    max_col = screen[curr_screen].cols[WINDOW_FILEAREA];
    if (col == 0) {
      col = x;
    } else {
      if (col > max_col) {
        col = max_col - 1;
      } else {
        col--;
      }
    }
    switch (curr_view->current_window) {

      case WINDOW_COMMAND:
        rc = THEcursor_home(curr_screen, curr_view, FALSE);
        break;

      case WINDOW_PREFIX:
        curr_view->current_window = WINDOW_FILEAREA;
        break;
    }
    wmove(SCREEN_WINDOW_FILEAREA(curr_screen), row, col);
    curr_view->focus_line = screen[curr_screen].sl[row].line_number;
    pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
  } else {
    /*
     * Convert supplied row/col to 0 based offset...
     */
    if (row == 0) {
      row = screen[curr_screen].start_row[curr_view->current_window] + y;
    } else {
      row--;
    }
    if (col == 0) {
      col = screen[curr_screen].start_col[curr_view->current_window] + x;
    } else {
      col--;
    }
    max_row = screen[curr_screen].screen_rows - 1;
    max_col = screen[curr_screen].screen_cols - 1;
    /*
     * If row/col outside maximum screen size, exit...
     */
    if (row > max_row || col > max_col) {
      if (show_errors) {
        display_error(63, (char_t *) "", FALSE);
      }
      return (RC_TOF_EOF_REACHED);      /* this is a strange RC :-( */
    }
    /*
     * Determine which window the cursor will end up in...
     */
    for (i = 0; i < VIEW_WINDOWS; i++) {
      int top_int_row = screen[curr_screen].start_row[i] - screen[curr_screen].screen_start_row;
      int bot_int_row = screen[curr_screen].start_row[i] - screen[curr_screen].screen_start_row + screen[curr_screen].rows[i] - 1;
      if (row >= top_int_row && row <= bot_int_row && col >= (screen[curr_screen].start_col[i] - screen[curr_screen].screen_start_col) && col <= (screen[curr_screen].start_col[i] + screen[curr_screen].cols[i] - 1 - screen[curr_screen].screen_start_col)) {
        idx = i;
        break;
      }
    }
    row = row - (screen[curr_screen].start_row[idx] - screen[curr_screen].screen_start_row);
    col = col - (screen[curr_screen].start_col[idx] - screen[curr_screen].screen_start_col);
    switch (idx) {

      case WINDOW_FILEAREA:
        row = get_row_for_tof_eof(row, curr_screen);
        if (!screen[curr_screen].sl[row].main_enterable) {
          if (show_errors) {
            display_error(63, (char_t *) "", FALSE);
          }
          return (RC_TOF_EOF_REACHED);  /* this is a strange RC :-( */
        }
        rc = do_Sos_current((char_t *) "", curr_screen, curr_view);
        wmove(SCREEN_WINDOW_FILEAREA(curr_screen), row, col);
        curr_view->focus_line = screen[curr_screen].sl[row].line_number;
        pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
        /*
         * If the colours of FILEAREA and CURSORLINE are different, we need
         * to display the screen.
         */
        if (set_colour(curr_view->file_for_view->attr + ATTR_FILEAREA) != set_colour(curr_view->file_for_view->attr + ATTR_CURSORLINE)) {
          build_screen(curr_screen);
          display_screen(curr_screen);
        }
        break;

      case WINDOW_PREFIX:
        row = get_row_for_tof_eof(row, curr_screen);
        if (!screen[curr_screen].sl[row].prefix_enterable) {
          if (show_errors) {
            display_error(63, (char_t *) "", FALSE);
          }
          return (RC_TOF_EOF_REACHED);  /* this is a strange RC :-( */
        }
        rc = do_Sos_current((char_t *) "", curr_screen, curr_view);
        rc = do_Sos_prefix((char_t *) "", curr_screen, curr_view);
        wmove(SCREEN_WINDOW_PREFIX(curr_screen), row, col);
        curr_view->focus_line = screen[curr_screen].sl[row].line_number;
        pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
        break;

      case WINDOW_COMMAND:
        rc = THEcursor_cmdline(curr_screen, curr_view, (short) (col + 1));
        break;

      default:
        if (show_errors) {
          display_error(63, (char_t *) "", FALSE);
        }
        return (RC_TOF_EOF_REACHED);    /* this is a strange RC :-( */
        break;
    }
  }
  return (rc);
}

short THEcursor_goto(line_t row, length_t col) {
  short rc = RC_OK;

  if (row > CURRENT_FILE->number_lines || row < 0 || col > max_line_length) {
    display_error(63, (char_t *) "", FALSE);
    return (RC_TOF_EOF_REACHED);        /* this is a strange RC :-( */
  }
  if (col == 0) {
    col = CURRENT_VIEW->verify_col;
  }
  if (THEcursor_file(FALSE, row, col) != RC_OK) {
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    /*
     * Make specified line current and display specified column in the
     * middle of the screen if not already displayed.
     */
    CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line = row;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    if (!column_in_view(current_screen, col - 1)) {
      if (col < CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2) {
        CURRENT_VIEW->verify_col = 1;
      } else {
        CURRENT_VIEW->verify_col = col - (CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2);
      }
    }
    build_screen(current_screen);
    display_screen(current_screen);
    THEcursor_file(FALSE, row, col);
  }
  return (rc);
}

#define MOUSE_Y (ncurses_mouse_event.y+1-screen[scrn].screen_start_row)
#define MOUSE_X (ncurses_mouse_event.x+1-screen[scrn].screen_start_col)

short THEcursor_mouse(void) {
  int w = 0;
  char_t scrn = 0;
  short rc = RC_OK;

  /*
   * If in READV CMDLINE, return without doing anything
   */
  if (in_readv) {
    return (RC_OK);
  }
  /*
   * First determine in which window the mouse is...
   */
  which_window_is_mouse_in(&scrn, &w);
  if (w == (-1)) {              /* shouldn't happen! */
    return (RC_INVALID_ENVIRON);
  }
  /*
   * If the mouse is in a different screen to the current one, move there
   */
  if (current_screen != scrn) {
    (void) Nextwindow((char_t *) "");
  }
  /*
   * Move the cursor to the correct screen coordinates...
   */
  rc = THEcursor_move(current_screen, CURRENT_VIEW, TRUE, FALSE, (short) MOUSE_Y, (short) MOUSE_X);
  return (rc);
}

long where_now(void) {
  long rc = 0L;
  unsigned short y = 0;

  y = getcury(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      rc |= WHERE_WINDOW_FILEAREA;
      break;

    case WINDOW_PREFIX:
      if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
        rc |= WHERE_WINDOW_PREFIX_LEFT;
      } else {
        rc |= WHERE_WINDOW_PREFIX_RIGHT;
      }
      break;

    case WINDOW_COMMAND:
      if (CURRENT_VIEW->cmd_line == 'B') {
        rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
      } else {
        rc |= WHERE_WINDOW_CMDLINE_TOP;
      }
      break;
  }
  if (display_screens == 1) {
    rc |= WHERE_SCREEN_ONLY;
  } else {
    if (current_screen == 0) {
      rc |= WHERE_SCREEN_FIRST;
    } else {
      rc |= WHERE_SCREEN_LAST;
    }
  }
  rc |= (long) y;
  return (rc);
}

long what_current_now(void) {
  long rc = 0;

  if (CURRENT_WINDOW_PREFIX != NULL) {
    if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
      rc |= WHERE_WINDOW_PREFIX_LEFT;
    } else {
      rc |= WHERE_WINDOW_PREFIX_RIGHT;
    }
  }
  if (CURRENT_VIEW->cmd_line == 'B') {
    rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
  } else {
    if (CURRENT_VIEW->cmd_line == 'T') {
      rc |= WHERE_WINDOW_CMDLINE_TOP;
    }
  }
  return (rc);
}

long what_other_now(void) {
  long rc = 0L;

  if (display_screens == 1) {
    return (rc);
  }
  if (OTHER_SCREEN.win[WINDOW_PREFIX] != NULL) {
    if ((OTHER_SCREEN.screen_view->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
      rc |= WHERE_WINDOW_PREFIX_LEFT;
    } else {
      rc |= WHERE_WINDOW_PREFIX_RIGHT;
    }
  }
  if (OTHER_SCREEN.screen_view->cmd_line == 'B') {
    rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
  } else {
    if (OTHER_SCREEN.screen_view->cmd_line == 'T') {
      rc |= WHERE_WINDOW_CMDLINE_TOP;
    }
  }
  return (rc);
}

long where_next(long where, long what_current, long what_other) {
  long where_row = 0L, where_window = 0L, where_screen = 0L;
  long what_current_window = 0L;
  long rc = 0L;
  unsigned short current_top_row = 0, current_bottom_row = 0;

  where_row = where & WHERE_ROW_MASK;
  where_window = where & WHERE_WINDOW_MASK;
  where_screen = where & WHERE_SCREEN_MASK;
  what_current_window = what_current & WHERE_WINDOW_MASK;
  find_first_focus_line(current_screen, &current_top_row);
  find_last_focus_line(current_screen, &current_bottom_row);
  switch (where_window) {

    case WHERE_WINDOW_FILEAREA:
      /*
       * In filearea.
       */
      if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT) {
        /*
         * In filearea and there is prefix on right.
         * Result: same row,same screen,go to prefix.
         */
        return (where_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
      }
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In filearea and only screen.
           */
          if (where_row == (long) current_bottom_row) {
            /*
             * In filearea, prefix on left or off, on bottom line.
             */
            if (what_current_window & WHERE_WINDOW_CMDLINE_TOP) {
              /*
               * In filearea,prefix on left or off, on bottom line.
               * Result: row irrelevant,same screen,go to cmdline.
               */
              return (where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
            }
            if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM) {
              /*
               * In filearea,prefix on left or off, on bottom line.
               * Result: row irrelevant,same screen,go to cmdline.
               */
              return (where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
            }
            /* To get here, there is no cmdline. */
            if (what_current_window & WHERE_WINDOW_PREFIX_LEFT) {
              /*
               * In filearea,prefix on left or off, on bottom line.
               * Result: first row,same screen,go to prefix.
               */
              return ((long) current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
            }
          }
          /* To get here, we are not on last row. */
          if (what_current_window & WHERE_WINDOW_PREFIX_LEFT) {
            /*
             * In filearea, prefix on left, not on bottom line.
             * Result: next row,same screen,go to prefix.
             */
            return ((where_row + 1L) | where_screen | WHERE_WINDOW_PREFIX_LEFT);
          }
          /*
           * In filearea, no prefix, not on bottom line.
           * Result: next row,same screen,same window.
           */
          return ((where_row + 1L) | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_PREFIX_LEFT:
      rc = where_row | where_screen | WHERE_WINDOW_FILEAREA;
      break;

    case WHERE_WINDOW_PREFIX_RIGHT:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In right prefix and only screen.
           */
          if (where_row != (long) current_bottom_row) {
            /*
             * In right prefix and not on bottom line.
             * Result: next row,same screen,go to filearea.
             */
            return ((where_row + 1L) | where_screen | WHERE_WINDOW_FILEAREA);
          }
          if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM) {
            /*
             * In right prefix, cmdline on bottom, on bottom line.
             * Result: row irrelevant,same screen,go to cmdline.
             */
            return (where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
          }
          if (what_current_window & WHERE_WINDOW_CMDLINE_TOP) {
            /*
             * In right prefix, cmdline on top, on bottom line.
             * Result: row irrelevant,same screen,go to cmdline.
             */
            return (where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
          }
          /*
           * In right prefix, no cmdline, on bottom line.
           * Result: first row,same screen,go to filearea.
           */
          return ((long) current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_CMDLINE_TOP:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In cmdline, and only screen.
           */
          if (what_current_window & WHERE_WINDOW_PREFIX_LEFT) {
            /*
             * In cmdline, and only screen and prefix on left.
             * Result: first row, same screen, go to prefix.
             */
            return ((long) current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
          }
          /*
           * In cmdline, and prefix on right or none.
           * Result: first row, same screen, go to filearea.
           */
          return ((long) current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_CMDLINE_BOTTOM:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In cmdline, and only screen.
           */
          if (what_current_window & WHERE_WINDOW_PREFIX_LEFT) {
            /*
             * In cmdline, and only screen and prefix on left.
             * Result: first row, same screen, go to prefix.
             */
            return ((long) current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
          }
          /*
           * In cmdline, and prefix on right or none.
           * Result: first row, same screen, go to filearea.
           */
          return ((long) current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;
  }
  return (rc);
}

long where_before(long where, long what_current, long what_other) {
  long where_row = 0L, where_window = 0L, where_screen = 0L;
  long what_current_window = 0L;
  long rc = 0L;
  unsigned short current_top_row = 0, current_bottom_row = 0;

  where_row = where & WHERE_ROW_MASK;
  where_window = where & WHERE_WINDOW_MASK;
  where_screen = where & WHERE_SCREEN_MASK;
  what_current_window = what_current & WHERE_WINDOW_MASK;
  find_first_focus_line(current_screen, &current_top_row);
  find_last_focus_line(current_screen, &current_bottom_row);
  switch (where_window) {

    case WHERE_WINDOW_FILEAREA:
      /*
       * In filearea.
       */
      if (what_current_window & WHERE_WINDOW_PREFIX_LEFT) {
        /*
         * In filearea and there is prefix on left.
         * Result: same row,same screen,go to prefix.
         */
        return (where_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
      }
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In filearea and only screen.
           */
          if (where_row == (long) current_top_row) {
            /*
             * In filearea, prefix on right or off, on top line.
             */
            if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM) {
              /*
               * In filearea,prefix on right or off, on top line.
               * Result: row irrelevant,same screen,go to cmdline.
               */
              return (where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
            }
            if (what_current_window & WHERE_WINDOW_CMDLINE_TOP) {
              /*
               * In filearea,prefix on right or off, on top line.
               * Result: row irrelevant,same screen,go to cmdline.
               */
              return (where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
            }
            /* To get here, there is no cmdline. */
            if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT) {
              /*
               * In filearea,prefix on right or off, on top line.
               * Result: last  row,same screen,go to prefix.
               */
              return ((long) current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
            }
          }
          /* To get here, we are not on top row. */
          if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT) {
            /*
             * In filearea, prefix on right, not on top line.
             * Result: prior row,same screen,go to prefix.
             */
            return ((where_row - 1L) | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
          }
          /*
           * In filearea, no prefix, not on top line.
           * Result: prior row,same screen,same window.
           */
          return ((where_row - 1L) | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_PREFIX_RIGHT:
      rc = where_row | where_screen | WHERE_WINDOW_FILEAREA;
      break;

    case WHERE_WINDOW_PREFIX_LEFT:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In left prefix and only screen.
           */
          if (where_row != (long) current_top_row) {
            /*
             * In left prefix and not on top line.
             * Result: prior row,same screen,go to filearea.
             */
            return ((where_row - 1L) | where_screen | WHERE_WINDOW_FILEAREA);
          }
          if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM) {
            /*
             * In left prefix, cmdline on bottom, on top line.
             * Result: row irrelevant,same screen,go to cmdline.
             */
            return (where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
          }
          if (what_current_window & WHERE_WINDOW_CMDLINE_TOP) {
            /*
             * In left prefix, cmdline on top, on top line.
             * Result: row irrelevant,same screen,go to cmdline.
             */
            return (where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
          }
          /*
           * In left prefix, no cmdline, on top line.
           * Result: last  row,same screen,go to filearea.
           */
          return ((long) current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_CMDLINE_TOP:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In cmdline, and only screen.
           */
          if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT) {
            /*
             * In cmdline, and only screen and prefix on right.
             * Result: last  row, same screen, go to prefix.
             */
            return ((long) current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
          }
          /*
           * In cmdline, and prefix on left  or none.
           * Result: last  row, same screen, go to filearea.
           */
          return ((long) current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;

    case WHERE_WINDOW_CMDLINE_BOTTOM:
      switch (where_screen) {

        case WHERE_SCREEN_FIRST:
        case WHERE_SCREEN_LAST:
          /* the two cases above will be separate in future */
        case WHERE_SCREEN_ONLY:
          /*
           * In cmdline, and only screen.
           */
          if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT) {
            /*
             * In cmdline, and only screen and prefix on right.
             * Result: last  row, same screen, go to prefix.
             */
            return ((long) current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
          }
          /*
           * In cmdline, and prefix on left  or none.
           * Result: last  row, same screen, go to filearea.
           */
          return ((long) current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
          break;
      }
      break;
  }
  return (rc);
}

bool enterable_field(long where) {
  bool rc = TRUE;
  row_t row = 0;
  long where_screen = 0L;
  char_t scrn = 0;

  where_screen = where & WHERE_SCREEN_MASK;
  row = (row_t) (where & WHERE_ROW_MASK);
  scrn = (where_screen == WHERE_SCREEN_LAST) ? 1 : 0;
  switch (where & WHERE_WINDOW_MASK) {

    case WHERE_WINDOW_FILEAREA:
      if (!screen[scrn].sl[row].main_enterable) {
        rc = FALSE;
      }
      break;

    case WHERE_WINDOW_PREFIX_LEFT:
    case WHERE_WINDOW_PREFIX_RIGHT:
      if (!screen[scrn].sl[row].prefix_enterable) {
        rc = FALSE;
      }
      break;

    case WHERE_WINDOW_CMDLINE_TOP:
    case WHERE_WINDOW_CMDLINE_BOTTOM:
      break;
  }
  return (rc);
}

short go_to_new_field(long save_where, long where) {
  short rc = RC_OK;
  long save_where_window = 0L, where_window = 0L;
  row_t where_row = 0;

  save_where_window = save_where & WHERE_WINDOW_MASK;
  where_window = where & WHERE_WINDOW_MASK;
  where_row = (row_t) (where & WHERE_ROW_MASK);
  if (save_where_window == where_window) {
    /*
     * No change to screen or window...
     */
    CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
  } else {
    switch (save_where_window) {

      case WHERE_WINDOW_FILEAREA:
        switch (where_window) {

          case WHERE_WINDOW_PREFIX_LEFT:
          case WHERE_WINDOW_PREFIX_RIGHT:
            CURRENT_VIEW->current_window = WINDOW_PREFIX;
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
            break;

          case WHERE_WINDOW_CMDLINE_TOP:
          case WHERE_WINDOW_CMDLINE_BOTTOM:
            CURRENT_VIEW->previous_window = CURRENT_VIEW->current_window;
            CURRENT_VIEW->current_window = WINDOW_COMMAND;
            where_row = 0;
            break;
        }
        break;

      case WHERE_WINDOW_PREFIX_LEFT:
      case WHERE_WINDOW_PREFIX_RIGHT:
        switch (where_window) {

          case WHERE_WINDOW_FILEAREA:
            CURRENT_VIEW->current_window = WINDOW_FILEAREA;
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
            break;

          case WHERE_WINDOW_CMDLINE_TOP:
          case WHERE_WINDOW_CMDLINE_BOTTOM:
            CURRENT_VIEW->previous_window = CURRENT_VIEW->current_window;
            CURRENT_VIEW->current_window = WINDOW_COMMAND;
            where_row = 0;
            break;
        }
        break;

      case WHERE_WINDOW_CMDLINE_TOP:
      case WHERE_WINDOW_CMDLINE_BOTTOM:
        switch (where_window) {

          case WHERE_WINDOW_PREFIX_LEFT:
          case WHERE_WINDOW_PREFIX_RIGHT:
            CURRENT_VIEW->current_window = WINDOW_PREFIX;
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
            break;

          case WHERE_WINDOW_FILEAREA:
            CURRENT_VIEW->current_window = WINDOW_FILEAREA;
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
            break;
        }
        break;
    }
  }
  wmove(CURRENT_WINDOW, where_row, 0);
  return (rc);
}

void get_cursor_position(line_t *screen_line, length_t *screen_column, line_t *file_line, length_t *file_column) {
  unsigned short y = 0, x = 0;
  unsigned short begy = 0, begx = 0;

  if (curses_started) {
    getyx(CURRENT_WINDOW, y, x);
    getbegyx(CURRENT_WINDOW, begy, begx);
    *screen_line = (line_t) (y + begy + 1L);
    *screen_column = (length_t) (x + begx + 1L);
  } else {
    *screen_line = *screen_column = (-1L);
  }
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      *file_line = CURRENT_VIEW->focus_line;
      *file_column = CURRENT_VIEW->verify_col + x;
      break;

    case WINDOW_PREFIX:
      *file_line = CURRENT_VIEW->focus_line;
      *file_column = (-1L);
      break;

    default:                   /* command line */
      *file_line = *file_column = (-1L);
      break;
  }
  return;
}

short advance_focus_line(line_t num_lines) {
  unsigned short y = 0, x = 0;
  LINE *curr = NULL;
  line_t actual_lines = num_lines;
  short direction = DIRECTION_FORWARD, rc = RC_OK;

  if (num_lines < 0L) {
    actual_lines = -num_lines;
    direction = DIRECTION_BACKWARD;
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->focus_line, CURRENT_FILE->number_lines);
  while (actual_lines > 0) {
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    if (curr == NULL) {
      break;
    }
    CURRENT_VIEW->focus_line += (line_t) direction;
    if (CURRENT_VIEW->scope_all || IN_SCOPE(CURRENT_VIEW, curr)) {
      actual_lines--;
    }
  }
  if (!line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
    CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  if (curses_started) {
    getyx(CURRENT_WINDOW, y, x);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    wmove(CURRENT_WINDOW, y, x);
  }
  if (FOCUS_TOF || FOCUS_BOF) {
    rc = RC_TOF_EOF_REACHED;
  }
  return rc;
}

short advance_current_line(line_t num_lines) {
  LINE *curr = NULL;
  line_t actual_lines = num_lines;
  short direction = DIRECTION_FORWARD;
  short y = 0, x = 0, rc = RC_OK;

  if (num_lines < 0L) {
    actual_lines = -num_lines;
    direction = DIRECTION_BACKWARD;
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->current_line, CURRENT_FILE->number_lines);
  while (actual_lines > 0) {
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    if (curr == NULL) {
      break;
    }
    CURRENT_VIEW->current_line += (line_t) direction;
    if (CURRENT_VIEW->scope_all || IN_SCOPE(CURRENT_VIEW, curr)) {
      actual_lines--;
    }
  }
  build_screen(current_screen);
  if (!line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
    if (compatible_feel == COMPAT_XEDIT) {
      THEcursor_cmdline(current_screen, CURRENT_VIEW, 1);
    }
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
      getyx(CURRENT_WINDOW, y, x);
    }
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    /* THEcursor_move(TRUE,FALSE,y+1,x+1); */
    THEcursor_move(current_screen, CURRENT_VIEW, TRUE, TRUE, (short) (y + 1), (short) (x + 1));
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  }
  return rc;
}

short advance_current_or_focus_line(line_t num_lines) {
  short rc = RC_OK;

  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || compatible_feel == COMPAT_XEDIT) {
    rc = advance_current_line(num_lines);
  } else {
    rc = advance_focus_line(num_lines);
  }
  return (rc);
}

void resolve_current_and_focus_lines(char_t curr_screen, VIEW_DETAILS *view, line_t true_line, line_t num_lines, short direction, bool respect_stay, bool sos) {
  short y = 0, x = 0;
  short save_compatible_feel = compatible_feel;

  /*
   * If no lines to move, don't do anything...
   */
  if (num_lines == 0) {
    build_screen(curr_screen);
    display_screen(curr_screen);
    return;
  }
  /*
   * Set the internal compatibility mode to THE for sos = TRUE.
   */
  if (sos) {
    save_compatible_feel = COMPAT_THE;
  }
  /*
   * If STAY is ON, and we are respecting it, don't do anything...
   */
  if (view->stay && (respect_stay || compatible_feel != COMPAT_XEDIT)) {
    build_screen(curr_screen);
    display_screen(curr_screen);
    return;
  }
  /*
   * If we are on the command line, all actions are the same irrespective
   * of the compatibility mode in place.
   */
  if (view->current_window == WINDOW_COMMAND) {
    view->current_line = true_line + num_lines - (line_t) direction;
    build_screen(curr_screen);
    display_screen(curr_screen);
    return;
  }
  /*
   * From here down is applicable to the cursor being in the FILEAREA or
   * PREFIX...
   */
  switch (save_compatible_feel) {

    case COMPAT_THE:
    case COMPAT_KEDIT:
    case COMPAT_KEDITW:
      view->focus_line = true_line + num_lines - (line_t) direction;
      build_screen(curr_screen);
      if (!line_in_view(curr_screen, view->focus_line)) {
        view->current_line = view->focus_line;
      }
      pre_process_line(view, view->focus_line, (LINE *) NULL);
      build_screen(curr_screen);
      display_screen(curr_screen);
      if (curses_started) {
        getyx(SCREEN_WINDOW(curr_screen), y, x);
        y = get_row_for_focus_line(curr_screen, view->focus_line, view->current_row);
        wmove(SCREEN_WINDOW(curr_screen), y, x);
      }
      break;

    case COMPAT_XEDIT:
      view->current_line = true_line + num_lines - (line_t) direction;
      pre_process_line(view, view->focus_line, (LINE *) NULL);
      build_screen(curr_screen);
      if (!line_in_view(curr_screen, view->focus_line)) {
        THEcursor_cmdline(curr_screen, view, 1);
      } else {
        if (curses_started) {
          if (view->current_window == WINDOW_FILEAREA) {
            getyx(SCREEN_WINDOW(curr_screen), y, x);
          }
          y = get_row_for_focus_line(curr_screen, view->focus_line, view->current_row);
          THEcursor_move(curr_screen, view, TRUE, FALSE, (short) (y + 1), (short) (x + 1));
        }
      }
      display_screen(curr_screen);
      break;
  }
  return;
}

