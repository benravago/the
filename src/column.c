// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */

#include "the.h"
#include "proto.h"

short column_command(char_t *cmd_text, int cmd_type) {
  length_t i = 0;
  line_t true_line = 0L;
  short rc = RC_OK;
  length_t len_params = 0;
  unsigned short y = 0, x = 0;

  /*
   * All column commands under XEDIT compatibility refer to current line.
   * ******* At this stage, revert to THE behaviour at all times *******
   */
  if (compatible_feel == COMPAT_XEDIT) {
    true_line = CURRENT_VIEW->current_line;
  } else {
    true_line = get_true_line(TRUE);
  }
  /*
   * If on TOF or BOF, exit with error.
   */
  if (TOF(true_line) || BOF(true_line)) {
    display_error(36, (char_t *) "", FALSE);
    return (RC_NO_LINES_CHANGED);
  }
  /*
   * If HEX mode is on, convert the hex string...
   */
  if (CURRENT_VIEW->hex) {
    len_params = convert_hex_strings(cmd_text);
    switch (len_params) {

      case -1:                 /* invalid hex value */
        display_error(32, cmd_text, FALSE);
        return (RC_INVALID_OPERAND);
        break;

      case -2:                 /* memory exhausted */
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
        break;

      default:
        break;
    }
  } else {
    len_params = strlen((char *) cmd_text);
  }
  /*
   * If on command line, copy current line into rec
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || compatible_feel == COMPAT_XEDIT) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL);
    x = CURRENT_VIEW->current_column - 1;
  } else {
    if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
      if (cmd_type != COLUMN_CAPPEND) {
        display_error(36, (char_t *) "", FALSE);
        return (RC_NO_LINES_CHANGED);
      }
    }
    if (curses_started) {
      getyx(CURRENT_WINDOW, y, x);
    }
    x = CURRENT_VIEW->verify_col - 1 + x;
  }
  switch (cmd_type) {

    case COLUMN_CAPPEND:
      CURRENT_VIEW->current_column = rec_len + 1;
      for (i = 0; i < len_params; i++) {
        if (rec_len > max_line_length) {
          break;
        }
        rec[rec_len] = *(cmd_text + i);
        rec_len++;
      }
      break;

    case COLUMN_CINSERT:
      if (x > rec_len) {
        rec_len = x;
        for (i = 0; i < len_params; i++) {
          if (rec_len > max_line_length) {
            break;
          }
          rec[rec_len] = *(cmd_text + i);
          rec_len++;
        }
      } else {
        rec = meminsmem(rec, cmd_text, len_params, x, max_line_length, rec_len);
        rec_len = min(max_line_length, rec_len + len_params);
      }
      break;

    case COLUMN_COVERLAY:
      for (i = 0; i < len_params; i++) {
        if (x > max_line_length) {
          break;
        }
        switch (*(cmd_text + i)) {

          case '_':
            rec[x] = ' ';
            break;

          case ' ':
            break;

          default:
            rec[x] = *(cmd_text + i);
            break;
        }
        x++;
      }
      rec_len = max(rec_len, x + 1);
      break;

    case COLUMN_CREPLACE:
      for (i = 0; i < len_params; i++) {
        if (x > max_line_length) {
          break;
        }
        rec[x] = *(cmd_text + i);
        x++;
      }
      rec_len = max(rec_len, x + 1);
      break;
  }
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || compatible_feel == COMPAT_XEDIT) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL, TRUE);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  } else {
    switch (cmd_type) {

      case COLUMN_CAPPEND:
        if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
          CURRENT_VIEW->current_window = WINDOW_FILEAREA;
          if (curses_started) {
            wmove(CURRENT_WINDOW, y, 0);
          }
        }
        rc = execute_move_cursor(current_screen, CURRENT_VIEW, CURRENT_VIEW->current_column - 1);
        break;

      case COLUMN_CINSERT:
        break;
    }
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}

