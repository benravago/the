// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

static short selective_change(TARGET *target, uchar *old_str, long len_old_str, uchar *new_str, long len_new_str, long true_line, long last_true_line, long start_col) {
  register short i = 0;
  short y = 0, x = 0, rc = RC_OK;
  int key = 0;
  bool changed = FALSE;
  bool line_displayed = FALSE;
  TARGET save_target;

  getyx(CURRENT_WINDOW_FILEAREA, y, x);
  /*
   * move cursor to old string a la cmatch
   * display message
   * accept key - C next, - N change, - Q to quit
   */
  CURRENT_VIEW->focus_line = true_line;
  /*
   * Check if the true_line is in the currently displayed window.
   * If not, then change the current_line to the true_line.
   */
  line_displayed = FALSE;
  for (i = 0; i < CURRENT_SCREEN.rows[WINDOW_FILEAREA]; i++) {
    if (CURRENT_SCREEN.sl[i].line_number == true_line && CURRENT_SCREEN.sl[i].line_type == LINE_LINE) {
      line_displayed = TRUE;
      y = i;
      break;
    }
  }
  if (!line_displayed) {
    CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
    y = CURRENT_VIEW->current_row;
  }
  if (start_col >= CURRENT_VIEW->verify_col - 1 && start_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA] + (CURRENT_VIEW->verify_col - 1)) - 1) {
    x = start_col - (CURRENT_VIEW->verify_col - 1);
  } else {
    x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
    CURRENT_VIEW->verify_col = max(1, start_col - (short) x);
    x = (start_col - (CURRENT_VIEW->verify_col - 1));
  }
  key = 0;
  changed = FALSE;
  /*
   * Save the current thighlight_target for restoring later...
   */
  if (CURRENT_VIEW->thighlight_on) {
    save_target = CURRENT_VIEW->thighlight_target;
    CURRENT_VIEW->thighlight_target = *target;
    CURRENT_VIEW->thighlight_active = TRUE;
    CURRENT_VIEW->thighlight_target.true_line = true_line;
    CURRENT_VIEW->thighlight_target.rt[0].found = TRUE;
    CURRENT_VIEW->thighlight_target.rt[0].start = start_col;
  }
  while (key == 0) {
    build_screen(current_screen);
    /*
     * Set the applicable fields of the target with the values from the string to be changed
     */
    if (CURRENT_VIEW->thighlight_on) {
      CURRENT_VIEW->thighlight_target.rt[0].length = (changed) ? len_new_str : len_old_str;
    }
    display_screen(current_screen);
    if (changed) {
      display_prompt((uchar *) "Press 'N' for next,'C' to undo 'Q' to quit");
    } else {
      display_prompt((uchar *) "Press 'N' for next,'C' to change 'Q' to quit");
    }
    wmove(CURRENT_WINDOW_FILEAREA, y, x);
    wrefresh(CURRENT_WINDOW_FILEAREA);
    key = my_getch(CURRENT_WINDOW_FILEAREA);
    clear_msgline(-1);
    switch (key) {
      case 'N':
      case 'n':
        if (changed) {
          rc = RC_OK;
        } else {
          rc = SKIP;
        }
        break;
      case 'C':
      case 'c':
        if (changed) {
          memdeln(rec, start_col, rec_len, len_new_str);
          rec_len -= len_new_str;
          meminsmem(rec, old_str, len_old_str, start_col, max_line_length, rec_len);
          rec_len += len_old_str;
        } else {
          memdeln(rec, start_col, rec_len, len_old_str);
          rec_len -= len_old_str;
          meminsmem(rec, new_str, len_new_str, start_col, max_line_length, rec_len);
          rec_len += len_new_str;
        }
        changed = (changed) ? FALSE : TRUE;
        key = 0;
        break;
      case 'Q':
      case 'q':
        if (changed) {
          rc = QUITOK;
        } else {
          rc = QUIT;
        }
        break;
      default:
        key = 0;
        break;
    }
  }
  /*
   * Restore the current thighlight_target.
   */
  if (CURRENT_VIEW->thighlight_on) {
    CURRENT_VIEW->thighlight_target = save_target;
    CURRENT_VIEW->thighlight_active = FALSE;
    display_screen(current_screen);
  }
  return (rc);
}

short execute_change_command(uchar *in_params, bool selective) {
  long num_lines = 0L, long_n = 0L, long_m = 0L;
  LINE *curr = NULL;
  uchar *old_str = NULL, *new_str = NULL;
  short rc = 0, selective_rc = RC_OK;
  short direction = DIRECTION_FORWARD;
  long number_changes = 0L, number_of_changes = 0L, number_of_occ = 0L;
  long start_col = 0, real_start = 0, real_end = 0, loc = 0;
  long true_line = 0L, last_true_line = 0L, number_lines = 0L;
  long num_actual_lines = 0L, abs_num_lines = 0L, i = 0L;
  long num_file_lines = 0L;
  long len_old_str = 0, len_new_str = 0;
  TARGET target;
  uchar message[100];
  bool lines_based_on_scope = FALSE;
  uchar *save_params = NULL;
  uchar *params;
  short save_target_type = TARGET_RELATIVE;
  long str_length = 0;

  /*
   * If no arguments have been supplied, pass the last change command to be executed.
   * If no last change command, return error 39.
   */
  if (blank_field(in_params)) {
    params = (selective) ? lastop[LASTOP_SCHANGE].value : lastop[LASTOP_CHANGE].value;
    if (blank_field(params)) {
      display_error(39, (uchar *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
  } else {
    params = in_params;
  }
  /*
   * Save the parameters for later...
   */
  if ((save_params = (uchar *) strdup((char*)params)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Validate the parameters that have been supplied.
   * Up to 4 parameters may be supplied.
   * The first is the string to change and its new value, the second is the target,
   * the third is the number of times to change the value on one line
   * and lastly is which occurrence to change first.
   */
  initialise_target(&target);
  rc = split_change_params(params, &old_str, &new_str, &target, &long_n, &long_m);
  if (rc != RC_OK) {
    free_target(&target);
    return (rc);
  }
  num_lines = target.num_lines;
  true_line = target.true_line;
  if (target.rt == NULL) {
    lines_based_on_scope = TRUE;
  } else {
    lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
    save_target_type = target.rt[0].target_type;
  }
  /*
   * For ISPF we ignore lines_based_on_scope.
   */
  if (compatible_feel == COMPAT_ISPF && target.rt == NULL) {
    lines_based_on_scope = FALSE;
  }
  /*
   * Check for any hex strings in both old_str and new_str.
   */
  if (CURRENT_VIEW->hex) {
    len_old_str = convert_hex_strings(old_str);
    switch (len_old_str) {
      case -1:                 /* invalid hex value */
        free_target(&target);
        display_error(32, old_str, FALSE);
        free(save_params);
        return (RC_INVALID_OPERAND);
        break;
      case -2:                 /* memory exhausted */
        free_target(&target);
        display_error(30, (uchar *) "", FALSE);
        free(save_params);
        return (RC_OUT_OF_MEMORY);
        break;
      default:
        break;
    }
    len_new_str = convert_hex_strings(new_str);
    switch (len_new_str) {
      case -1:                 /* invalid hex value */
        free_target(&target);
        display_error(32, new_str, FALSE);
        free(save_params);
        return (RC_INVALID_OPERAND);
        break;
      case -2:                 /* memory exhausted */
        free_target(&target);
        display_error(30, (uchar *) "", FALSE);
        free(save_params);
        return (RC_OUT_OF_MEMORY);
        break;
      default:
        break;
    }
  } else {
    len_old_str = strlen((char *) old_str);
    len_new_str = strlen((char *) new_str);
  }
  /*
   * Save the last change command...
   */
  if (selective) {
    rc = save_lastop(LASTOP_SCHANGE, save_params);
  } else {
    rc = save_lastop(LASTOP_CHANGE, save_params);
  }
  free(save_params);
  if (rc != RC_OK) {
    free_target(&target);
    display_error(30, (uchar *) "", FALSE);
    return rc;
  }
  /*
   * If the number of lines is zero, don't make any changes.
   * Exit with no rows changed.
   */
  if (num_lines == 0L) {
    free_target(&target);
    display_error(36, (uchar *) "", FALSE);
    return (RC_NO_LINES_CHANGED);
  }
  if (num_lines < 0) {
    direction = DIRECTION_BACKWARD;
    abs_num_lines = -num_lines;
  } else {
    direction = DIRECTION_FORWARD;
    abs_num_lines = num_lines;
  }

  if (true_line != CURRENT_VIEW->focus_line) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  }
  last_true_line = true_line;
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines) {
        break;
      }
    } else {
      /*
       * For ISPF implement a change with no target as find first target, change it and finish.
       */
      if (compatible_feel == COMPAT_ISPF && target.rt == NULL) {
        if (number_of_changes > 0) {
          break;
        }
      } else {
        if (abs_num_lines == i) {
          break;
        }
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line, curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        pre_process_line(CURRENT_VIEW, true_line, curr);
        loc = 0;
        number_of_changes = number_of_occ = 0L;
        while (loc != (-1)) {
          if (save_target_type == TARGET_BLOCK_CURRENT) {
            if (MARK_VIEW->mark_type == M_STREAM || MARK_VIEW->mark_type == M_CUA) {
              real_end = rec_len + len_old_str;
              real_start = start_col;
              if (true_line == MARK_VIEW->mark_start_line) {
                real_start = max(start_col, MARK_VIEW->mark_start_col - 1);
              }
              if (true_line == MARK_VIEW->mark_end_line) {
                real_end = min(rec_len + len_old_str, MARK_VIEW->mark_end_col - 1);
              }
            } else {
              real_end = min(rec_len + len_old_str, MARK_VIEW->mark_end_col - 1);
              real_start = max(start_col, MARK_VIEW->mark_start_col - 1);
            }
          } else {
            real_end = min(rec_len + len_old_str, CURRENT_VIEW->zone_end - 1);
            real_start = max(start_col, CURRENT_VIEW->zone_start - 1);
          }
          if (rec_len < real_start && blank_field(old_str)) {
            loc = 0;
            rec_len = real_start + 1;
          } else {
            loc = memfind(rec + real_start, old_str, real_end - real_start + 1, len_old_str, (bool) ((CURRENT_VIEW->case_change == CASE_IGNORE) ? TRUE : FALSE), CURRENT_VIEW->arbchar_status, CURRENT_VIEW->arbchar_single, CURRENT_VIEW->arbchar_multiple, &str_length);
          }
          if (loc != (-1)) {
            start_col = loc + real_start;
            if (number_of_changes <= long_n - 1 && number_of_occ >= long_m - 1) {
              /* the following block is done for change or confirm of sch */
              if (!selective) {
                memdeln(rec, start_col, rec_len, len_old_str);
                rec_len = (long) max((long) start_col, (long) rec_len - (long) len_old_str);
                meminsmem(rec, new_str, len_new_str, start_col, max_line_length, rec_len);
                rec_len += len_new_str;
                if (rec_len > max_line_length) {
                  rec_len = max_line_length;
                  loc = (-1);
                }
                start_col += len_new_str;
                number_changes++;
                number_of_changes++;
              } else {
                /* selective */
                selective_rc = selective_change(&target, old_str, len_old_str, new_str, len_new_str, true_line, last_true_line, start_col);
                last_true_line = true_line;
                switch (selective_rc) {
                  case QUITOK:
                  case RC_OK:
                    start_col += len_new_str;
                    number_changes++;
                    number_of_changes++;
                    if (rec_len > max_line_length) {
                      rec_len = max_line_length;
                      loc = (-1);
                    }
                    break;
                  case SKIP:
                    start_col += len_old_str;
                    break;
                  case QUIT:
                    break;
                }
                if (selective_rc == QUIT || selective_rc == QUITOK) {
                  break;
                }
              }
              number_of_occ++;
            } else {
              start_col += len_old_str;
              number_of_occ++;
            }
            if (number_of_changes > long_n - 1) { /* || number_of_occ > long_n-1 */
              loc = (-1);
            }
          }
        }                       /* end while */
        if (number_of_changes != 0L) {  /* changes made */
          post_process_line(CURRENT_VIEW, true_line, curr, FALSE);
          number_lines++;
        }
        num_actual_lines++;
        break;
    }
    if (selective_rc == QUIT || selective_rc == QUITOK) {
      break;
    }
    start_col = 0;
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    true_line += (long) (direction);
    num_file_lines += (long) (direction);
    if (curr == NULL) {
      break;
    }
  }
  free_target(&target);
  /*
   * If no changes were made, display error message and return.
   */
  if (number_changes == 0L) {
    display_error(36, (uchar *) "", FALSE);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    return (RC_NO_LINES_CHANGED);
  }
  /*
   * Increment the alteration count here, once irrespective of the number of lines changed.
   */
  increment_alt(CURRENT_FILE);
  /*
   * If STAY is OFF, change the current and focus lines by the number of lines calculated from the target.
   */
  if (selective) {
    if (!CURRENT_VIEW->stay) {  /* stay is off */
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
    }
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    build_screen(current_screen);
    display_screen(current_screen);
  } else {
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, last_true_line, num_file_lines, direction, TRUE, FALSE);
  }
  sprintf((char *) message, "%ld occurrence(s) changed on %ld line(s)", number_changes, number_lines);
  display_error(0, message, TRUE);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return rc;
}

short insert_new_line(uchar curr_screen, VIEW_DETAILS *curr_view, uchar *line, long len, long num_lines, long true_line, bool start_left_col, bool make_current, bool inc_alt, uchar select, bool move_cursor, bool sos_command) {
  long i;
  LINE *curr = NULL, *save_curr = NULL;
  unsigned short x = 0, y = 0;
  long new_col = 0;
  bool on_bottom_of_file = FALSE, on_bottom_of_screen = FALSE;
  short number_focus_rows = 0;
  bool leave_cursor = FALSE;
  long new_focus_line = 0L, new_current_line = 0L;

  if (!curr_view->scope_all) {
    true_line = find_last_not_in_scope(curr_view, NULL, true_line, DIRECTION_FORWARD);
  }
  /*
   * If we are on the 'Bottom of File' line
   * reduce the true_line by 1 so that the new line is added before the bottom line.
   */
  if (true_line == (curr_view->file_for_view->number_lines + 1L)) {
    true_line--;
  }
  /*
   * Find the current LINE pointer for the true_line.
   * This is the line after which the line(s) are to be added.
   */
  curr = lll_find(curr_view->file_for_view->first_line, curr_view->file_for_view->last_line, true_line, curr_view->file_for_view->number_lines);
  /*
   * Insert into the linked list the number of lines specified.
   * All lines will contain a blank line and a length of zero.
   */
  save_curr = curr;
  for (i = 0; i < num_lines; i++) {
    if ((curr = add_LINE(curr_view->file_for_view->first_line, curr, line, len, select, TRUE)) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
  }
  /*
   * Fix the positioning of the marked block (if there is one and it is in the current view)
   * and any pending prefix commands.
   */
  adjust_marked_lines(TRUE, true_line, num_lines);
  adjust_pending_prefix(curr_view, TRUE, true_line, num_lines);
  /*
   * Increment the number of lines counter for the current file and the
   * number of alterations, only if requested to do so.
   */
  if (inc_alt) {
    increment_alt(curr_view->file_for_view);
  }
  curr_view->file_for_view->number_lines += num_lines;
  /*
   * Sort out focus and current line.
   */
  if (move_cursor) {
    switch (curr_view->current_window) {
      case WINDOW_COMMAND:
        curr_view->focus_line = true_line + 1L;
        if (make_current) {
          curr_view->current_line = true_line + 1L;
        }
        pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
        break;
      case WINDOW_FILEAREA:
      case WINDOW_PREFIX:
        build_screen(curr_screen);
        getyx(SCREEN_WINDOW(curr_screen), y, x);
        calculate_scroll_values(curr_screen, curr_view, &number_focus_rows, &new_focus_line, &new_current_line, &on_bottom_of_screen, &on_bottom_of_file, &leave_cursor, DIRECTION_FORWARD);
        new_col = x;
        if (curr_view->current_window == WINDOW_FILEAREA) {
          if (!start_left_col) {
            if (curr_view->newline_aligned) {
              new_col = memne(save_curr->line, ' ', save_curr->length);
              if (new_col == (-1)) {
                new_col = 0;
              }
              /*
               * Special case when right margin is > than screen width...
               */
              if (curr_view->verify_start != curr_view->verify_col) {
                /*
                 * If the new column position will be on the same page...
                 */
                if (curr_view->verify_col < new_col && curr_view->verify_col + screen[curr_screen].screen_cols > new_col) {
                  new_col = (new_col - curr_view->verify_col) + 1;
                } else {
                  x = screen[curr_screen].cols[WINDOW_FILEAREA] / 2;
                  curr_view->verify_col = max(1, new_col - (short) x + 2);
                  new_col = (curr_view->verify_col == 1) ? new_col : x - 1;
                }
              }
            } else {
              new_col = 0;
              curr_view->verify_col = 1;
            }
          }
        }
        /*
         * Move the cursor to where it should be and display the page.
         */
        if (on_bottom_of_screen) {
          curr_view->current_line = new_current_line;
          curr_view->focus_line = new_focus_line;
          wmove(SCREEN_WINDOW(curr_screen), y - ((leave_cursor) ? 0 : 1), new_col);
        } else {
          /*
           * We are in the middle of the window, so just move the cursor down 1 line.
           */
          wmove(SCREEN_WINDOW(curr_screen), y + number_focus_rows, new_col);
          curr_view->focus_line = new_focus_line;
          if (compatible_feel == COMPAT_XEDIT && !sos_command) {
            curr_view->current_line = new_current_line;
          }
        }
        break;
    }
  }
  pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
  build_screen(curr_screen);
  display_screen(curr_screen);
  return (RC_OK);
}

#define SHELL "SHELL"

short execute_os_command(uchar *cmd, bool quiet, bool pause) {
  short rc = 0;

  if (!quiet && curses_started) {
    attrset(A_NORMAL);
    clear();
    wmove(stdscr, 1, 0);
    wrefresh(stdscr);           /* clear screen */
    suspend_curses();
  }
  if (allocate_temp_space(strlen((char *) cmd), TEMP_TEMP_CMD) != RC_OK) {
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  if (strcmp((char *) cmd, "") == 0) {
    sprintf((char *) temp_cmd, "\"%s\"", getenv(SHELL));
    /*
     * If no command to execute then do not ask for "any key"
     */
    pause = 0;
  } else {
    strcpy((char *) temp_cmd, (char *) cmd);
  }
  if (strcmp((char *) temp_cmd, "") == 0) {     /* no SHELL env variable set */
    printf("No SHELL environment variable set - using /bin/sh\n");
    fflush(stdout);
    strcpy((char *) temp_cmd, "/bin/sh");
  }
  if (quiet) {
    strcat((char *) temp_cmd, " > /dev/null");
  }
  rc = system((char *) temp_cmd);
  if (pause) {
    printf("\n\n%s", HIT_ANY_KEY);
    fflush(stdout);
  }
  if (!quiet && curses_started) {
    if (pause) {
      (void) my_getch(stdscr);
    }
    resume_curses();
    restore_THE();
  }
  if (curses_started) {
    draw_cursor(TRUE);
  }
  return (rc);
}

short execute_makecurr(uchar curr_screen, VIEW_DETAILS *curr_view, long line) {
  unsigned short y = 0, x = 0;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr_view->current_line = line;
  if (curr_view->current_window == WINDOW_PREFIX) {
    getyx(SCREEN_WINDOW(curr_screen), y, x);
  } else {
    getyx(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
  }
  build_screen(curr_screen);
  display_screen(curr_screen);
  y = get_row_for_focus_line(curr_screen, curr_view->focus_line, curr_view->current_row);
  if (curr_view->current_window == WINDOW_PREFIX) {
    wmove(SCREEN_WINDOW(curr_screen), y, x);
  } else {
    wmove(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
  }
  return (RC_OK);
}

short execute_shift_command(uchar curr_screen, VIEW_DETAILS *curr_view, bool shift_left, long num_cols, long true_line, long num_lines, bool lines_based_on_scope, long target_type, bool sos, bool zone_shift) {
  LINE *curr = NULL;
  long abs_num_lines = (num_lines < 0L ? -num_lines : num_lines);
  long num_file_lines = 0L, i = 0L;
  long num_actual_lines = 0L;
  long left_col = 0L, right_col = 0L;
  long j = 0;
  long actual_cols = 0;
  short rc = RC_OK;
  short direction = (num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  bool adjust_alt = FALSE;

  /*
   * Always post_process_line() the line in the CURRENT_VIEW
   */
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr = lll_find(curr_view->file_for_view->first_line, curr_view->file_for_view->last_line, true_line, curr_view->file_for_view->number_lines);
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines) {
        break;
      }
    } else {
      if (abs_num_lines == num_file_lines) {
        break;
      }
    }
    rc = processable_line(curr_view, true_line + (long) (i * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        memset(trec, ' ', max_line_length);
        memcpy(trec, curr->line, curr->length);
        trec_len = curr->length;
        if (target_type == TARGET_BLOCK_CURRENT) {
          if (MARK_VIEW->mark_type == M_LINE) {
            left_col = curr_view->zone_start - 1;
            right_col = curr_view->zone_end - 1;
          } else {
            left_col = MARK_VIEW->mark_start_col - 1;
            right_col = MARK_VIEW->mark_end_col - 1;
          }
        } else {
          left_col = curr_view->zone_start - 1;
          right_col = curr_view->zone_end - 1;
        }
        if (shift_left) {
          actual_cols = min(num_cols, max(0, trec_len - left_col));
          memdeln(trec, left_col, trec_len, actual_cols);
          trec_len -= actual_cols;
          if (zone_shift) {
            /*
             * Fill up the right most positions of the zone
             * with blanks.
             */
            for (j = 0; j < actual_cols; j++) {
              meminschr(trec, ' ', right_col, max_line_length, trec_len++);
            }
          }
        } else {
          if (zone_shift) {
            /*
             * Remove the right most positions of the zone.
             */
            actual_cols = min(num_cols, max(0, 1 + trec_len - right_col));
            memdeln(trec, 1 + right_col - actual_cols, trec_len, actual_cols);
            trec_len -= actual_cols;
          }
          for (j = 0; j < num_cols; j++) {
            meminschr(trec, ' ', left_col, max_line_length, trec_len++);
          }
          if (trec_len > max_line_length) {
            trec_len = max_line_length;
            display_error(0, (uchar *) "Truncated", FALSE);
          }
          actual_cols = num_cols;
        }
        /*
         * Set a flag to cause alteration counts to be incremented.
         */
        if (actual_cols != 0) {
          adjust_alt = TRUE;
          /*
           * Add the old line contents to the line recovery list.
           */
          add_to_recovery_list(curr->line, curr->length);
          /*
           * Realloc the dynamic memory for the line if the line is now longer.
           */
          if (trec_len > curr->length) {
            curr->line = (uchar *) realloc((void *) curr->line, (trec_len + 1) * sizeof(uchar));
            if (curr->line == NULL) {
              display_error(30, (uchar *) "", FALSE);
              return (RC_OUT_OF_MEMORY);
            }
          }
          /*
           * Copy the contents of trec into the line.
           */
          memcpy(curr->line, trec, trec_len);
          curr->length = trec_len;
          *(curr->line + trec_len) = '\0';
          curr->flags.changed_flag = TRUE;
        }
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    num_file_lines += (long) direction;
    if (curr == NULL) {
      break;
    }
  }
  /*
   * Increment the alteration counters once if any line has changed...
   */
  if (adjust_alt) {
    increment_alt(curr_view->file_for_view);
  }
  /*
   * Display the new screen...
   */
  pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
  resolve_current_and_focus_lines(curr_screen, curr_view, true_line, num_file_lines, direction, TRUE, sos);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return rc;
}

short execute_set_lineflag(unsigned int new_flag, unsigned int changed_flag, unsigned int tag_flag, long true_line, long num_lines, bool lines_based_on_scope, long target_type) {
  LINE *curr = NULL;
  long abs_num_lines = (num_lines < 0L ? -num_lines : num_lines);
  long num_file_lines = 0L, i = 0L;
  long num_actual_lines = 0L;
  short rc = RC_OK;
  short direction = (num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  bool adjust_alt = FALSE;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines) {
        break;
      }
    } else {
      if (abs_num_lines == num_file_lines) {
        break;
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line + (long) (i * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        if (new_flag < 2) {
          curr->flags.new_flag = new_flag;
        }
        if (changed_flag < 2) {
          curr->flags.changed_flag = changed_flag;
        }
        if (tag_flag < 2) {
          curr->flags.tag_flag = tag_flag;
        }
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    num_file_lines += (long) direction;
    if (curr == NULL) {
      break;
    }
  }
  /*
   * Increment the alteration counters once if any line has changed...
   */
  if (adjust_alt) {
    increment_alt(CURRENT_FILE);
  }
  /*
   * Display the new screen...
   */
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, true_line, num_file_lines, direction, TRUE, FALSE);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return rc;
}

/*
 * Returns TRUE if a line was changed, FALSE otherwise.
 * This function MUST preceed execute_change_case().
 */
static bool change_case(uchar *str, long start, long end, uchar which_case) {
  long i;
  bool altered = FALSE;

  for (i = start; i < end + 1; i++) {
    switch (which_case) {
      case CASE_UPPER:
        if (islower(*(str + i))) {
          *(str + i) = toupper(*(str + i));
          altered = TRUE;
        }
        break;
      case CASE_LOWER:
        if (isupper(*(str + i))) {
          *(str + i) = tolower(*(str + i));
          altered = TRUE;
        }
        break;
    }
  }
  return (altered);
}

short do_actual_change_case(long true_line, long num_lines, uchar which_case, bool lines_based_on_scope, short direction, long start_col, long end_col) {
  bool adjust_alt = FALSE;
  LINE *curr = NULL;
  long i, num_actual_lines = 0L, num_file_lines = 0L;
  short rc = RC_OK;

  /*
   * Find the current LINE pointer for the true_line.
   * This is the first line to change.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * Change the case for the target lines and columns...
   */
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == num_lines) {
        break;
      }
    } else {
      if (num_lines == i) {
        break;
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line + (long) (i * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        add_to_recovery_list(curr->line, curr->length);
        if (MARK_VIEW && (MARK_VIEW->mark_type == M_STREAM || MARK_VIEW->mark_type == M_CUA)) {
          int mystart = 0, myend = curr->length - 1;

          if (true_line + i == MARK_VIEW->mark_start_line) {
            mystart = start_col;
          }
          if (true_line + i == MARK_VIEW->mark_end_line) {
            myend = end_col;
          }
          rc = change_case(curr->line, mystart, min(curr->length - 1, myend), which_case);
        } else {
          rc = change_case(curr->line, start_col, min(curr->length - 1, end_col), which_case);
        }
        if (rc) {
          adjust_alt = TRUE;
          curr->flags.changed_flag = TRUE;
        }
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    num_file_lines += (long) direction;
    if (curr == NULL) {
      break;
    }
  }
  /*
   * Increment the alteration counts if any lines changed...
   */
  if (adjust_alt) {
    increment_alt(CURRENT_FILE);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, true_line, num_file_lines, direction, TRUE, FALSE);
  return rc;
}

short execute_change_case(uchar *params, uchar which_case) {
  long num_lines = 0L, true_line = 0L;
  short direction = 0;
  long start_col = 0, end_col = 0;
  short rc = RC_OK;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  bool lines_based_on_scope = TRUE;

  /*
   * Validate the parameters that have been supplied.
   * Valid values are: a target or "block".
   * If no parameter is supplied, 1 is assumed.
   */
  if (strcmp("", (char *) params) == 0) {
    params = (uchar *) "+1";
  }
  initialise_target(&target);
  if ((rc = validate_target(params, &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Determine in which direction we are working.
   */
  if (target.num_lines < 0L) {
    direction = DIRECTION_BACKWARD;
    num_lines = target.num_lines * (-1L);
  } else {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
  }
  true_line = target.true_line;
  /*
   * If the target is BLOCK set the left and right margins to be the margins of the BOX BLOCK, otherwise use ZONE settings.
   */
  start_col = CURRENT_VIEW->zone_start - 1;
  end_col = CURRENT_VIEW->zone_end - 1;
  if (target.rt[0].target_type == TARGET_BLOCK_CURRENT) {
    num_lines = MARK_VIEW->mark_end_line - MARK_VIEW->mark_start_line + 1L;
    true_line = MARK_VIEW->mark_start_line;
    direction = DIRECTION_FORWARD;
    lines_based_on_scope = FALSE;
    if (MARK_VIEW->mark_type != M_LINE) {
      start_col = MARK_VIEW->mark_start_col - 1;
      end_col = MARK_VIEW->mark_end_col - 1;
    }
  }
  /*
   * Change the case of the lines...
   */
  rc = do_actual_change_case(true_line, num_lines, which_case, lines_based_on_scope, direction, start_col, end_col);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return (rc);
}

short rearrange_line_blocks(uchar command, uchar source, long start_line, long end_line, long dest_line, long num_occ, VIEW_DETAILS *src_view, VIEW_DETAILS *dst_view, bool lines_based_on_scope, long *lines_affected) {
  long j = 0, k = 0;
  short rc = RC_OK;
  static unsigned short y = 0, x = 0;
  bool dst_inside_src = FALSE, lines_added = FALSE, reset_block = FALSE;
  bool dest_in_block = FALSE;
  short direction = 0;
  long num_lines = 0L, off = 0L, adjust_line = dest_line, num_actual_lines = 0L;
  long i = 0L, num_pseudo_lines = 0L;
  LINE *curr_src = NULL, *curr_dst = NULL;
  LINE *save_curr_src = NULL, *save_curr_dst = NULL;
  FILE_DETAILS *src_file = NULL, *dst_file = NULL;

  src_file = src_view->file_for_view;
  dst_file = dst_view->file_for_view;
  if (source == SOURCE_BLOCK) {
    reset_block = FALSE;
  } else {
    reset_block = TRUE;
  }
  /*
   * This block of commands is for copying lines...
   */
  switch (command) {
    case COMMAND_COPY:
    case COMMAND_OVERLAY_COPY:
    case COMMAND_MOVE_COPY_SAME:
    case COMMAND_MOVE_COPY_DIFF:
    case COMMAND_DUPLICATE:
      lines_added = TRUE;
      switch (source) {
        case SOURCE_BLOCK:
        case SOURCE_BLOCK_RESET:
          if (src_view == dst_view && dest_line >= start_line && dest_line < end_line) {
            dest_in_block = TRUE;
          }
          break;
        case SOURCE_PREFIX:
          if (dest_line >= start_line && dest_line < end_line) {
            dest_in_block = TRUE;
          }
          break;
        default:
          break;
      }
      /*
       * If the destination line is within the marked block
       * then we have to handle the processing of the src_curr pointer differently.
       */
      if (dest_in_block) {
        dst_inside_src = TRUE;
        off = dest_line - start_line;
      } else {
        dst_inside_src = FALSE;
      }
      if (start_line > end_line) {
        direction = DIRECTION_BACKWARD;
        num_lines = start_line - end_line + 1L;
      } else {
        direction = DIRECTION_FORWARD;
        num_lines = end_line - start_line + 1L;
      }
      save_curr_src = lll_find(src_file->first_line, src_file->last_line, start_line, src_file->number_lines);
      save_curr_dst = lll_find(dst_file->first_line, dst_file->last_line, dest_line, dst_file->number_lines);
      for (k = 0; k < num_occ; k++) {
        curr_src = save_curr_src;
        curr_dst = save_curr_dst;
        for (i = 0L, num_actual_lines = 0L;; i++) {
          if (lines_based_on_scope) {
            if (num_actual_lines == num_lines) {
              break;
            }
          } else {
            if (num_lines == i) {
              break;
            }
          }
          rc = processable_line(src_view, start_line + (i * direction), curr_src);
          switch (rc) {
            case LINE_SHADOW:
              break;
            case LINE_TOF:
            case LINE_EOF:
              num_actual_lines++;
              num_pseudo_lines++;
              break;
            default:
              if ((curr_dst = add_LINE(dst_file->first_line, curr_dst, curr_src->line, curr_src->length, dst_view->display_low, TRUE)) == NULL) { /* curr_src->select == NULL */
                display_error(30, (uchar *) "", FALSE);
                return (RC_OUT_OF_MEMORY);
              }
              /*
               * If moving lines within the same file, move any line
               * name with the line also.
               */
              if (command == COMMAND_MOVE_COPY_SAME) {
                if (curr_src->first_name != (THELIST *) NULL) {
                  curr_dst->first_name = curr_src->first_name;
                  curr_src->first_name = (THELIST *) NULL;
                }
              }
              if (direction == DIRECTION_BACKWARD) {
                curr_dst = save_curr_dst;
              }
              num_actual_lines++;
              break;
          }
          if (dst_inside_src && i == off) {
            for (j = 0; j < off + 1; j++) {
              curr_src = curr_src->next;
            }
          }
          if (direction == DIRECTION_FORWARD) {
            curr_src = curr_src->next;
          } else {
            curr_src = curr_src->prev;
          }
        }
      }
      dst_file->number_lines += (num_actual_lines - num_pseudo_lines) * num_occ;
      *lines_affected = (num_actual_lines - num_pseudo_lines) * num_occ;
      break;
    default:
      break;
  }
  /*
   * This block of commands is for deleting lines...
   */
  switch (command) {
    case COMMAND_OVERLAY_DELETE:
    case COMMAND_DELETE:
    case COMMAND_MOVE_DELETE_SAME:
    case COMMAND_MOVE_DELETE_DIFF:
      lines_added = FALSE;
      if (start_line > end_line) {
        direction = DIRECTION_BACKWARD;
        num_lines = start_line - end_line + 1L;
      } else {
        direction = DIRECTION_FORWARD;
        num_lines = end_line - start_line + 1L;
      }
      curr_dst = lll_find(dst_file->first_line, dst_file->last_line, start_line, dst_file->number_lines);
      for (i = 0L, num_actual_lines = 0L;; i++) {
        if (lines_based_on_scope) {
          if (num_actual_lines == num_lines) {
            break;
          }
        } else {
          if (num_lines == i) {
            break;
          }
        }
        rc = processable_line(dst_view, start_line + (i * direction), curr_dst);
        switch (rc) {
          case LINE_TOF:
          case LINE_EOF:
            num_actual_lines++; /* this is meant to fall through */
            num_pseudo_lines++;
          case LINE_SHADOW:
            if (direction == DIRECTION_FORWARD) {
              curr_dst = curr_dst->next;
            } else {
              curr_dst = curr_dst->prev;
            }
            break;
          default:
            if (command != COMMAND_MOVE_DELETE_SAME) {
              add_to_recovery_list(curr_dst->line, curr_dst->length);
            }
            curr_dst = delete_LINE(&dst_file->first_line, &dst_file->last_line, curr_dst, direction, TRUE);
            num_actual_lines++;
        }
        if (curr_dst == NULL) {
          break;
        }
      }
      dst_file->number_lines -= (num_actual_lines - num_pseudo_lines) * num_occ;
      break;
    default:
      break;
  }
  /*
   * Increment alteration count for all but COMMAND_MOVE_COPY_SAME...
   */
  if (command != COMMAND_MOVE_COPY_SAME && command != COMMAND_OVERLAY_COPY && (num_actual_lines - num_pseudo_lines) != 0) {
    increment_alt(dst_file);
  }
  /*
   * This block of commands is for sorting out cursor position...
   */
  if (curses_started) {
    getyx(CURRENT_WINDOW, y, x);
  }
  switch (command) {
    case COMMAND_COPY:
    case COMMAND_OVERLAY_COPY:
    case COMMAND_MOVE_COPY_SAME:
    case COMMAND_MOVE_COPY_DIFF:
    case COMMAND_DUPLICATE:
      if (source == SOURCE_COMMAND && CURRENT_VIEW->current_window == WINDOW_COMMAND && CURRENT_VIEW->stay) {
        break;
      }
      if (command == COMMAND_DUPLICATE) {
        dst_view->focus_line = dest_line + 1L;
        if (dst_view == CURRENT_SCREEN.screen_view) {
          unsigned short last_focus_row = 0;

          find_last_focus_line(current_screen, &last_focus_row);
          if (dest_line >= CURRENT_SCREEN.sl[last_focus_row].line_number) {
            dst_view->current_line = dst_view->focus_line;
            y = dst_view->current_row;
          } else {
            y = get_row_for_focus_line(current_screen, dst_view->focus_line, dst_view->current_row);
          }
        }
      } else {
        if (IN_VIEW(dst_view, dest_line)) {
          dst_view->focus_line = dest_line + 1L;
        }
        if (dst_view->current_window != WINDOW_COMMAND && dst_view == CURRENT_SCREEN.screen_view) {
          unsigned short last_focus_row = 0;

          find_last_focus_line(current_screen, &last_focus_row);
          if (y == last_focus_row) {
            dst_view->current_line = dst_view->focus_line;
            y = dst_view->current_row;
          } else {
            y = get_row_for_focus_line(current_screen, dst_view->focus_line, dst_view->current_row);
          }
        }
      }
      break;
    case COMMAND_DELETE:
    case COMMAND_OVERLAY_DELETE:
    case COMMAND_MOVE_DELETE_SAME:
    case COMMAND_MOVE_DELETE_DIFF:
      if (dst_view->focus_line >= start_line && dst_view->focus_line <= end_line) {
        if (IN_VIEW(dst_view, dest_line)) {
          if (dst_view->current_line > dst_file->number_lines + 1L) {
            /*
             * This is better than before, but the cursor still ends up NOT on the focus line ?????
             */
            dst_view->current_line = dst_file->number_lines + 1L;
            dst_view->focus_line = dest_line;
          } else {
            dst_view->focus_line = dest_line;
          }
        } else {
          if (dest_line > dst_file->number_lines) {
            dst_view->focus_line = dst_view->current_line = dst_file->number_lines;
          } else {
            dst_view->focus_line = dst_view->current_line = dest_line;
          }
        }
      } else {
        dest_line = (dst_view->focus_line < start_line ? dst_view->focus_line : dst_view->focus_line - num_lines);
        if (IN_VIEW(dst_view, dest_line)) {
          if (dst_view->current_line > dst_file->number_lines + 1L) {
            dst_view->current_line -= (num_actual_lines - num_pseudo_lines);
          }
          dst_view->focus_line = dest_line;
        } else {
          if (dest_line > dst_file->number_lines) {
            dst_view->focus_line = dst_view->current_line = dst_file->number_lines;
          } else {
            dst_view->focus_line = dst_view->current_line = dest_line;
          }
        }
      }
      if (dst_file->number_lines == 0L) {
        dst_view->focus_line = dst_view->current_line = 0L;
      }
      if (dst_view->current_window != WINDOW_COMMAND && dst_view == CURRENT_SCREEN.screen_view) {
        build_screen(current_screen);
        y = get_row_for_focus_line(current_screen, dst_view->focus_line, dst_view->current_row);
      }
      /*
       * This is set here so that the adjust_pending_prefix command will work
       */
      if (direction == DIRECTION_BACKWARD) {
        adjust_line = end_line;
      } else {
        adjust_line = start_line;
      }
      dst_view->current_line = find_next_in_scope(dst_view, NULL, dst_view->current_line, DIRECTION_FORWARD);
      break;
    default:
      break;
  }
  /*
   * This block of commands is for adjusting prefix and block lines...
   */
  switch (source) {
    case SOURCE_BLOCK:
    case SOURCE_BLOCK_RESET:
      adjust_pending_prefix(dst_view, lines_added, adjust_line, (num_actual_lines - num_pseudo_lines) * num_occ);
      if (command == COMMAND_MOVE_DELETE_SAME) {
        adjust_marked_lines(lines_added, adjust_line, (num_actual_lines - num_pseudo_lines) * num_occ);
      } else {
        switch (command) {
          case COMMAND_MOVE_DELETE_DIFF:
            src_view->marked_line = src_view->marked_col = FALSE;
            break;
          case COMMAND_OVERLAY_DELETE:
            break;
          default:
            if (command == COMMAND_COPY && src_view != dst_view) {
              src_view->marked_line = src_view->marked_col = FALSE;
            }
            /*
             * The following does a 'reset block' in the current view.
             */
            if (reset_block) {
              dst_view->marked_line = dst_view->marked_col = FALSE;
              MARK_VIEW = (VIEW_DETAILS *) NULL;
            } else {
              if (command == COMMAND_OVERLAY_DELETE) {
                dst_view->mark_start_line = dest_line;
              } else {
                dst_view->mark_start_line = dest_line + 1L;
              }
              dst_view->mark_end_line = dest_line + (num_actual_lines - num_pseudo_lines);
              dst_view->marked_col = FALSE;
              dst_view->mark_start_col = src_view->mark_start_col;
              dst_view->mark_end_col = src_view->mark_end_col;
              dst_view->mark_type = M_LINE;
              dst_view->focus_line = dst_view->mark_start_line;
              MARK_VIEW = dst_view;
            }
            break;
        }
      }
      break;
    case SOURCE_PREFIX:
    case SOURCE_COMMAND:
      adjust_marked_lines(lines_added, adjust_line, (num_actual_lines - num_pseudo_lines) * num_occ);
      adjust_pending_prefix(dst_view, lines_added, adjust_line, (num_actual_lines - num_pseudo_lines) * num_occ);
      break;
  }
  if (command != COMMAND_MOVE_DELETE_DIFF) {
    pre_process_line(CURRENT_VIEW, dst_view->focus_line, (LINE *) NULL);
  }
  if ((source == SOURCE_BLOCK || source == SOURCE_BLOCK_RESET) && display_screens > 1) {
    build_screen((uchar) (other_screen));
    display_screen((uchar) (other_screen));
  }
  if (command != COMMAND_MOVE_DELETE_DIFF && command != COMMAND_MOVE_COPY_SAME && command != COMMAND_OVERLAY_COPY && command != COMMAND_OVERLAY_DELETE) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  if (dst_view->current_window != WINDOW_COMMAND && dst_view == CURRENT_SCREEN.screen_view) {
    if (curses_started) {
      wmove(CURRENT_WINDOW, y, x);
    }
  }
  return (RC_OK);
}

short execute_set_point(uchar curr_screen, VIEW_DETAILS *curr_view, uchar *name, long true_line, bool point_on) {
  LINE *curr = NULL;
  long dummy = 0L;
  uchar *this_name;
  THELIST *curr_name;

  /*
   * Find a line that already has the same name.
   * If one exists, remove the name from that line.
   * This is done whether we are adding a name or deleting it.
   */
  if ((curr = find_named_line(name, &dummy, FALSE)) != (LINE *) NULL) {
    /*
     * We found a line with the specified name; remove it from the list of names for this line
     * Find the name in the list and remove the name...
     */
    curr_name = find_line_name(curr, name);
    if (curr_name) {
      ll_del(&curr->first_name, NULL, curr_name, DIRECTION_FORWARD, free);
    }
  }
  if (point_on) {
    /*
     * Doesn't matter if the name didn't exist; we are adding a name.
     * Find the focus line...
     */
    curr = lll_find(curr_view->file_for_view->first_line, curr_view->file_for_view->last_line, true_line, curr_view->file_for_view->number_lines);
    /*
     * Allocate space for the name and add it to the start of the linked list
     */
    if ((this_name = (uchar *) malloc(strlen((char *) name) + 1)) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) this_name, (char *) name);
    curr->first_name = ll_add(curr->first_name, NULL, sizeof(THELIST));
    curr->first_name->data = (void *) this_name;
  }
  if (!point_on && curr == NULL) {
    /*
     * Error if the name didn't exist; we are trying to delete it.
     */
    display_error(60, name, FALSE);
    return (RC_INVALID_OPERAND);
  }
  return (RC_OK);
}

short execute_wrap_word(long col) {
  long i = 0;
  long col_break = 0, cursor_offset = 0;
  LINE *curr = NULL, *next_line = NULL;
  bool bnewline = FALSE, cursor_wrap = FALSE;
  uchar *buf = NULL, *word_to_wrap = NULL;
  long next_line_start = 0, length_word = 0, last_col = 0;
  short rc = RC_OK;

  /*
   * This function is called when the length of the focus line exceeds the right margin.
   * If the cursor is positioned in the last word of the line, the cursor moves with that word to the next line.
   * If the combined length of the word to be wrapped and a space and the line following the focus line exceeds the right margin,
   * a new line is inserted with the word being wrapped, otherwise the word to be wrapped is prepended to the following line.
   *
   * -
   * Find the current LINE pointer for the focus_line.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->focus_line, CURRENT_FILE->number_lines);
  /*
   * Determine where to start splitting the line in relation to end of line...
   */
  col_break = memreveq(rec, ' ', rec_len);
  /*
   * If there is no word break, don't attempt any wrap.
   */
  if (col_break == (-1)) {
    return (RC_OK);
  }
  /*
   * Actual column to break on is 1 character to right of last space.
   */
  col_break++;
  /*
   * Make a null terminated string out of current line so we can grab the word to be wrapped.
   */
  rec[rec_len] = '\0';
  /*
   * Point to word to wrap and determine its length.
   */
  length_word = rec_len - col_break;
  word_to_wrap = (uchar *) rec + col_break;
  /*
   * If the position of the cursor is before the word to wrap leave the cursor where it is.
   */
  if (col >= col_break) {
    cursor_wrap = TRUE;
    cursor_offset = col - col_break - 1;
  } else {
    cursor_wrap = FALSE;
  }
  /*
   * Now we have to work out if a new line is to added or we prepend to the following line...
   */
  if (curr->next->next == NULL) { /* next line bottom of file */
    bnewline = TRUE;
  } else {
    next_line = curr->next;
    if (!IN_SCOPE(CURRENT_VIEW, next_line)) {
      bnewline = TRUE;
    } else {
      next_line_start = memne(next_line->line, ' ', next_line->length);
      if (next_line_start != CURRENT_VIEW->margin_left - 1) {   /* next line doesn't start in left margin */
        bnewline = TRUE;
      } else {
        if (next_line->length + length_word + 1 > CURRENT_VIEW->margin_right) {
          bnewline = TRUE;
        }
      }
    }
  }
  /*
   * Save the word to be wrapped...
   */
  buf = (uchar *) malloc(length_word + CURRENT_VIEW->margin_left);
  if (buf == NULL) {
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  memcpy(buf, word_to_wrap, length_word);
  /*
   * Remove the word to be wrapped from the focus line buffer; rec...
   */
  for (i = col_break; i < rec_len + 1; i++) {
    rec[i] = ' ';
  }
  last_col = memrevne(rec, ' ', max_line_length);
  rec_len = (last_col == (-1)) ? 0 : last_col + 1;
  /*
   * We now should know if a new line is to added or not.
   */
  if (bnewline) {
    for (i = 0; i < CURRENT_VIEW->margin_left - 1; i++) {
      (void) meminschr(buf, ' ', 0, max_line_length, i + length_word);
    }
    curr = add_LINE(CURRENT_FILE->first_line, curr, buf, length_word + CURRENT_VIEW->margin_left - 1, curr->select, TRUE);
    CURRENT_FILE->number_lines++;
  } else {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line + 1L, (LINE *) NULL);
    (void) meminschr(rec, ' ', CURRENT_VIEW->margin_left - 1, max_line_length, rec_len++);
    (void) meminsmem(rec, buf, length_word, CURRENT_VIEW->margin_left - 1, max_line_length, rec_len);
    rec_len += length_word;
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line + 1L, (LINE *) NULL, TRUE);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  free(buf);
  /*
   * We now should know if the cursor is to wrap or stay where it is.
   */
  if (cursor_wrap) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    build_screen(current_screen);
    THEcursor_down(current_screen, CURRENT_VIEW, TRUE);
    rc = Sos_firstchar((uchar *) "");
    for (i = 0; i < cursor_offset + 1; i++) {
      rc = THEcursor_right(TRUE, FALSE);
    }
  } else {
    if (INSERTMODEx) {
      rc = THEcursor_right(TRUE, FALSE);
    }
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}

short execute_split_join(short action, bool aligned, bool cursorarg) {
  long i = 0;
  long num_cols = 0, num_blanks_focus = 0, num_blanks_next = 0;
  unsigned short x = 0, y = 0;
  LINE *curr = NULL;
  long true_line = 0L;
  long col = 0;

  /*
   * Determine line and column to use.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || cursorarg == FALSE) {
    col = CURRENT_VIEW->current_column - 1;
    true_line = CURRENT_VIEW->current_line;
  } else {
    if (curses_started) {
      getyx(CURRENT_WINDOW_FILEAREA, y, x);
      col = (x + CURRENT_VIEW->verify_col - 1);
      true_line = CURRENT_VIEW->focus_line;
    } else {
      col = 0;
      true_line = CURRENT_VIEW->current_line;
    }
  }
  /*
   * Reject the command if true_line is top or bottom of file.
   */
  if (VIEW_TOF(CURRENT_VIEW, true_line) || VIEW_BOF(CURRENT_VIEW, true_line)) {
    display_error(38, (uchar *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  /*
   * If this function called from SPLTJOIN(), then determine if SPLIT
   * or JOIN is required.
   */
  if (action == SPLTJOIN_SPLTJOIN) {
    action = (col >= rec_len) ? SPLTJOIN_JOIN : SPLTJOIN_SPLIT;
  }
  /*
   * Find the current LINE pointer for the true line.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  getyx(CURRENT_WINDOW, y, x);
  switch (action) {
    case SPLTJOIN_SPLIT:
      /*
       * Copy any changes in the focus line to the linked list.
       */
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
      memset(rec, ' ', max_line_length);
      if (col < curr->length) {
        memcpy(rec, curr->line + col, curr->length - col);
        rec_len = curr->length - col;
      } else {
        rec_len = 0;
      }
      curr->flags.changed_flag = TRUE;
      /*
       * Calculate the number of leading blanks on the current line
       * so that the new line can have this many blanks prepended to align properly.
       */
      if (aligned) {
        num_cols = memne(curr->line, ' ', curr->length);
        if (num_cols == (-1)) {
          num_cols = 0;
        }
        for (i = 0; i < num_cols; i++) {
          meminschr(rec, ' ', 0, max_line_length, rec_len++);
        }
        rec_len = min(rec_len, max_line_length);
      }
      add_LINE(CURRENT_FILE->first_line, curr, (rec), rec_len, curr->select, TRUE);
      CURRENT_FILE->number_lines++;
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND || cursorarg == FALSE) {
        if (curr->length > col) {
          curr->length = col;
          *(curr->line + (col)) = '\0';
          increment_alt(CURRENT_FILE);
        }
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      } else {
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
        Sos_delend((uchar *) "");
        post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
      }
      break;
    case SPLTJOIN_JOIN:
      if (curr->next->next == NULL) {
        /*
         * Trying to join with the bottom of file line.
         */
        return (RC_INVALID_ENVIRON);
      }
      /*
       * Calculate the number of leading blanks for the focus line and also for the line to be joined.
       * To align the join properly, we have to remove up the number of leading blanks in the focus line
       * from the beginning of the line to be joined.
       */
      if (aligned) {
        num_blanks_focus = memne(curr->line, ' ', curr->length);
        if (num_blanks_focus == (-1)) {
          num_blanks_focus = 0;
        }
        num_blanks_next = memne(curr->next->line, ' ', curr->length);
        if (num_blanks_next == (-1)) {
          num_blanks_next = 0;
        }
        num_cols = min(num_blanks_focus, num_blanks_next);
      } else {
        num_cols = 0;
      }
      /*
       * If the join would result in exceeding line length (or future TRUNC column)
       */
      if (col + curr->next->length - num_cols > max_line_length) {
        display_error(154, (uchar *) "", FALSE);
        return (RC_NO_LINES_CHANGED);
      }
      /*
       * Copy any changes in the focus line to the linked list.
       */
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);

      if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL);
      }
      meminsmem(rec, curr->next->line + num_cols, curr->next->length - num_cols, col, max_line_length, col);
      rec_len = col + curr->next->length - num_cols;
      post_process_line(CURRENT_VIEW, true_line, (LINE *) NULL, TRUE);
      curr = delete_LINE(&CURRENT_FILE->first_line, &CURRENT_FILE->last_line, curr->next, DIRECTION_BACKWARD, TRUE);
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      }
      /*
       * If on the bottom line, use the previous line.
       */
      if (CURRENT_BOF) {
        CURRENT_VIEW->current_line--;
        y++;
      }
      /*
       * Decrement the number of lines counter for the current file and move the cursor to the appropriate line.
       */
      CURRENT_FILE->number_lines--;
      if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
        wmove(CURRENT_WINDOW, y, x);
      }
      break;
  }
  /*
   * Determine new current line
   */
  if (action == SPLTJOIN_SPLIT && compatible_feel == COMPAT_XEDIT && CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    CURRENT_VIEW->current_line++;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
      build_screen(current_screen);
      if (!line_in_view(current_screen, CURRENT_VIEW->focus_line) && compatible_feel == COMPAT_XEDIT) {
        THEcursor_cmdline(current_screen, CURRENT_VIEW, 1);
      } else {
        y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
        wmove(CURRENT_WINDOW, y, x);
      }
    }
  }
  /*
   * Fix the positioning of the marked block (if there is one and it is in the current view)
   * and any pending prefix commands.
   */
  adjust_marked_lines((bool) ((action == SPLTJOIN_SPLIT) ? TRUE : FALSE), true_line, 1L);
  adjust_pending_prefix(CURRENT_VIEW, (bool) ((action == SPLTJOIN_SPLIT) ? TRUE : FALSE), true_line, 1L);
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}

short execute_put(uchar *params, bool putdel) {
  long num_lines = 0L, true_line = 0L, num_file_lines = 0L;
  bool append = FALSE;
  uchar *filename = NULL;
  short rc = RC_OK;
  long start_col = 0, end_col = max_line_length;
  bool lines_based_on_scope = TRUE;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL | TARGET_SPARE;
  short direction = DIRECTION_FORWARD;

  /*
   * If there are no arguments, default to "1"...
   */
  if (strcmp("", (char *) params) == 0) {
    params = (uchar *) "1";
  }
  /*
   * Validate first argument as a target...
   */
  initialise_target(&target);
  if ((rc = validate_target(params, &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  /*
   * If there is no second argument, no filename supplied...
   */
  if (target.spare == (-1)) {
    append = FALSE;
    filename = tempfilename;
    fprintf(stderr, "1:%s\n", filename);
  } else {
    /*
     * Remove leading and trailing spaces, then remove any leading and trailing double quotes
     */
    target.rt[target.spare].string = strstrip(strstrip(target.rt[target.spare].string, STRIP_BOTH, ' '), STRIP_BOTH, '"');
    if ((rc = splitpath(target.rt[target.spare].string)) != RC_OK) {
      display_error(10, target.rt[target.spare].string, FALSE);
      free_target(&target);
      return (rc);
    }
    strcpy((char *) temp_cmd, (char *) sp_path);
    strcat((char *) temp_cmd, (char *) sp_fname);
    filename = temp_cmd;
    append = TRUE;
  }
  true_line = target.true_line;
  num_lines = target.num_lines;
  /*
   * Determine in which direction we are working.
   */
  direction = (target.num_lines < 0L) ? DIRECTION_BACKWARD : DIRECTION_FORWARD;
  /*
   * If the marked block is a BOX block, set up the left and right column values.
   */
  if (target.rt[0].target_type == TARGET_BLOCK_CURRENT) {
    lines_based_on_scope = FALSE;
    if (MARK_VIEW->mark_type == M_BOX || MARK_VIEW->mark_type == M_STREAM || MARK_VIEW->mark_type == M_WORD || MARK_VIEW->mark_type == M_CUA) {
      start_col = MARK_VIEW->mark_start_col - 1;
      end_col = MARK_VIEW->mark_end_col - 1;
      // end_line = MARK_VIEW->mark_end_line;
    }
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if ((rc = save_file(CURRENT_FILE, filename, TRUE, num_lines, true_line, &num_file_lines, append, start_col, end_col, FALSE, lines_based_on_scope, FALSE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  /*
   * If we are executing a putd command, delete the target...
   */
  if (putdel) {
    rc = DeleteLine(target.string);
  } else {
    /*
     * If STAY is OFF, change the current and focus lines by the number of lines calculated from the target.
     * This is only applicable for PUT and NOT for PUTDEL.
     */
    resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, true_line, num_file_lines, direction, FALSE, FALSE);
  }
  free_target(&target);
  return (rc);
}

#define MAC_PARAMS  3

short execute_macro(uchar *params, bool error_on_not_found, short *macrorc) {
  short rc = RC_OK;
  short errnum = 0;
  FILE *fp = NULL;
  uchar *word[MAC_PARAMS + 1];
  uchar strip[MAC_PARAMS];
  uchar quoted[MAC_PARAMS];
  unsigned short num_params = 0;
  uchar *macroname = NULL;
  bool save_in_macro = in_macro;
  bool allow_interactive = FALSE;
  uchar *tmpfilename = NULL;
  uchar *tmpargs = NULL;

  /*
   * Validate the parameters. At least 1 must be present, the filename.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_NONE;
  quoted[0] = '"';
  quoted[1] = '\0';
  num_params = quoted_param_split(params, word, MAC_PARAMS - 1, WORD_DELIMS, TEMP_PARAM, strip, TRUE, quoted);
  if (num_params == 0) {
    display_error(3, (uchar *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Check if first parameter is ?...
   */
  if (strcmp((char *) word[0], "?") == 0) {
    strip[0] = STRIP_BOTH;
    strip[1] = STRIP_BOTH;
    strip[2] = STRIP_NONE;
    quoted[0] = '\0';
    quoted[1] = '"';
    quoted[2] = '\0';
    num_params = quoted_param_split(params, word, MAC_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, TRUE, quoted);
    if (num_params == 1) {
      display_error(3, (uchar *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    tmpfilename = word[1];
    tmpargs = word[2];
    allow_interactive = TRUE;
  } else {
    tmpfilename = word[0];
    tmpargs = word[1];
    allow_interactive = FALSE;
  }
  /*
   * Allocate some space for macroname...
   */
  if ((macroname = (uchar *) malloc((MAX_FILE_NAME + 1) * sizeof(uchar))) == NULL) {
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Find the fully qualified file name for the supplied macro name.
   */
  rc = get_valid_macro_file_name(tmpfilename, macroname, macro_suffix, &errnum);
  /*
   * Validate the return code...
   */
  switch (rc) {
    case RC_OK:
      /*
       * If RC_OK, continue to process the macro...
       */
      break;
    case RC_FILE_NOT_FOUND:
      /*
       * If RC_FILE_NOT_FOUND and IMPOS is not on, display an error and exit.
       * If IMPOS is on, just return without displaying an error.
       */
      if (error_on_not_found) {
        display_error(errnum, tmpfilename, FALSE);
      }
      free(macroname);
      return (rc);
      break;
    default:
      /*
       * All other cases, display error and return.
       */
      display_error(errnum, tmpfilename, FALSE);
      free(macroname);
      return (rc);
  }
  /*
   * Set in_macro = TRUE to stop multiple show_page()s being performed.
   */
  if (!allow_interactive) {
    in_macro = TRUE;
  } else {
    in_macro = FALSE;
  }
  /*
   * Save the values of the cursor position...
   */
  get_cursor_position(&original_screen_line, &original_screen_column, &original_file_line, &original_file_column);
  /*
   * If REXX is supported, process the macro as a REXX macro...
   */
  if (rexx_support) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    rc = execute_macro_file(macroname, tmpargs, macrorc, allow_interactive);
    if (rc != RC_OK) {
      display_error(54, (uchar *) "", FALSE);
      rc = RC_SYSTEM_ERROR;
    }
    free(macroname);
  } else {
    /*
     * ...otherwise, process the file as a non-REXX macro file...
     */
    if ((fp = fopen((char *) macroname, "r")) == NULL) {
      rc = RC_ACCESS_DENIED;
      display_error(8, macroname, FALSE);
    }
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    if (rc == RC_OK) {
      rc = execute_command_file(fp);
      fclose(fp);
    }
    free(macroname);
    if (rc == RC_SYSTEM_ERROR) {
      display_error(53, (uchar *) "", FALSE);
    }
    if (rc == RC_NOREXX_ERROR) {
      display_error(52, (uchar *) "", FALSE);
    }
  }
  /*
   * Set in_macro = FALSE to indicate we are out of the macro
   * and do a show_page() now as long as there are still file(s) in the ring.
   */
  in_macro = save_in_macro;
  if (number_of_files > 0) {
    if (display_screens > 1) {
      build_screen((uchar) (other_screen));
      display_screen((uchar) (other_screen));
    }
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}

short write_macro(uchar *defn) {
  int len, i;

  if (record_fp) {
    len = strlen((char *) defn);
    if (len) {
      fputc('"', record_fp);
      for (i = 0; i < len; i++) {
        if (defn[i] == '"') {
          fputc('"', record_fp);
        }
        fputc(defn[i], record_fp);
      }
      fprintf(record_fp, "\"\n");
      fflush(record_fp);
    }
  }
  return 0;
}

short execute_set_on_off(uchar *inparams, bool *flag, bool error_display) {
  short rc = RC_OK;
  int len = 0;
  uchar *params = NULL;

  /*
   * Make a copy of the arguments so we can split them up
   */
  if ((params = (uchar *) strdup((char*)inparams)) == NULL) {
    if (error_display) {
      display_error(30, (uchar *) "", FALSE);
    }
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Strip the leading and trailing spaces from parameters...
   */
  params = strstrip(params, STRIP_BOTH, ' ');
  /*
   * Validate the parameter. It must be ON or OFF.
   */
  len = strlen((char *) params);

  if ((len < 2) || (toupper(params[0]) != 'O')) {
    if (error_display) {
      display_error(1, (uchar *) inparams, FALSE);
    }
    rc = RC_INVALID_OPERAND;
  } else {
    if ((len == 3) && (toupper(params[1]) == 'F') && (toupper(params[2]) == 'F')) {
      *flag = FALSE;
    } else {
      if ((len == 2) && (toupper(params[1]) == 'N')) {
        *flag = TRUE;
      } else {
        if (error_display) {
          display_error(1, (uchar *) inparams, FALSE);
        }
        rc = RC_INVALID_OPERAND;
      }
    }
  }
  free(params);
  return (rc);
}

short execute_set_row_position(uchar *inparams, short *base, short *off) {
  short rc = RC_OK;
  uchar *params = NULL, *save_param_ptr = NULL;

  /*
   * Strip the leading and trailing spaces from parameters...
   */
  if ((params = save_param_ptr = (uchar *) strdup((char*)inparams)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  params = strstrip(params, STRIP_BOTH, ' ');
  /*
   * Parse the position parameter...
   */
  if (*params == 'M' || *params == 'm') {
    *base = POSITION_MIDDLE;
    params++;
    if (blank_field(params)) {
      *off = 0;
    } else {
      if ((*params != '-' && *params != '+') || ((*off = atoi((char *) params)) == 0)) {
        display_error(1, inparams, FALSE);
        free(save_param_ptr);
        return (RC_INVALID_OPERAND);
      }
    }
  } else {
    if ((*off = atoi((char *) params)) == 0) {
      display_error(1, inparams, FALSE);
      free(save_param_ptr);
      return (RC_INVALID_OPERAND);
    }
    *base = (*off > 0) ? POSITION_TOP : POSITION_BOTTOM;
  }
  free(save_param_ptr);
  return (rc);
}

short processable_line(VIEW_DETAILS *view, long true_line, LINE *curr) {

  if (VIEW_TOF(view, true_line)) {
    return (LINE_TOF);
  }
  if (VIEW_BOF(view, true_line)) {
    return (LINE_EOF);
  }
  if (view->scope_all || IN_SCOPE(view, curr)) {
    return (LINE_LINE);
  }
  return (LINE_SHADOW);
}

short execute_expand_compress(uchar *params, bool expand, bool inc_alt, bool use_tabs, bool add_to_recovery) {
  long i = 0L, num_actual_lines = 0L;
  long num_lines = 0L, true_line = 0L, num_file_lines = 0L;
  short direction = 0, rc = RC_OK;
  LINE *curr = NULL;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  bool lines_based_on_scope = FALSE;
  bool adjust_alt = FALSE;

  /*
   * Validate the parameters that have been supplied.
   * If no parameter is supplied, 1 is assumed.
   */
  true_line = get_true_line(TRUE);
  if (strcmp("", (char *) params) == 0) {
    params = (uchar *) "1";
  }
  initialise_target(&target);
  if ((rc = validate_target(params, &target, target_type, true_line, TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  /*
   * If a BOX BLOCK target requested, return an error...
   */
  if (target.rt[0].target_type == TARGET_BLOCK_CURRENT) {
    switch (MARK_VIEW->mark_type) {
      case M_BOX:
      case M_COLUMN:
      case M_WORD:
        display_error(48, (uchar *) "", FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      case M_STREAM:
      case M_CUA:
        display_error(49, (uchar *) "", FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      default:
        break;
    }
    if (MARK_VIEW->mark_type != M_LINE) {
      free_target(&target);
      return (rc);
    }
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Determine in which direction we are working.
   */
  if (target.num_lines < 0L) {
    direction = DIRECTION_BACKWARD;
    num_lines = -target.num_lines;
  } else {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
  }
  true_line = target.true_line;
  lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
  free_target(&target);
  /*
   * Find the current LINE pointer for the true_line.
   * This is the first line to change.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * Convert all tabs in the current line to spaces.
   */
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == num_lines) {
        break;
      }
    } else {
      if (num_lines == i) {
        break;
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line + (long) (i * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        rc = tabs_convert(curr, expand, use_tabs, add_to_recovery);
        if (rc == RC_FILE_CHANGED) {
          adjust_alt = TRUE;
        } else {
          if (rc != RC_OK) {
            return (rc);
          }
        }
        num_actual_lines++;
    }
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    num_file_lines += (long) direction;
    if (curr == NULL) {
      break;
    }
  }
  /*
   * Increment the number of alterations count if required...
   */
  if (inc_alt && adjust_alt) {
    increment_alt(CURRENT_FILE);
  }
  /*
   * If STAY is OFF, change the current and focus lines by the number of lines calculated from the target.
   */
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, true_line, num_file_lines, direction, TRUE, FALSE);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return (rc);
}

short execute_select(uchar *params, bool relative, short off) {
  long i = 0L, num_actual_lines = 0L;
  long num_lines = 0L, true_line = 0L;
  short direction = 0, rc = RC_OK;
  LINE *curr = NULL;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  bool lines_based_on_scope = FALSE;

  /*
   * Validate the parameters that have been supplied.
   * If no parameter is supplied, 1 is assumed.
   */
  true_line = get_true_line(TRUE);
  initialise_target(&target);
  if ((rc = validate_target(params, &target, target_type, true_line, TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Determine in which direction we are working.
   */
  if (target.num_lines < 0L) {
    direction = DIRECTION_BACKWARD;
    num_lines = -target.num_lines;
  } else {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
  }
  true_line = target.true_line;
  lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
  /*
   * Find the current LINE pointer for the true_line.
   * This is the first line to change.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * Convert all tabs in the current line to spaces.
   */
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == num_lines) {
        break;
      }
    } else {
      if (num_lines == i) {
        break;
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line + (long) (i * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        if (relative) {
          if (((short) curr->select + off) > MAX_SELECT_LEVEL) {
            curr->select = MAX_SELECT_LEVEL;
          } else {
            if (((short) curr->select + off) < 0) {
              curr->select = 0;
            } else {
              curr->select += off;
            }
          }
        } else {
          curr->select = off;
        }
        num_actual_lines++;
    }
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    if (curr == NULL) {
      break;
    }
  }
  free_target(&target);
  if (CURRENT_TOF || CURRENT_BOF) {
    rc = RC_TOF_EOF_REACHED;
  } else {
    rc = RC_OK;
  }
  return (rc);
}

short execute_move_cursor(uchar curr_screen, VIEW_DETAILS *curr_view, long col) {
  short y = 0, x = 0;
  ushort new_screen_col = 0;
  long new_verify_col = 0;

  switch (curr_view->current_window) {
    case WINDOW_FILEAREA:
      getyx(SCREEN_WINDOW(curr_screen), y, x);
      calculate_new_column(curr_screen, curr_view, x, curr_view->verify_col, col, &new_screen_col, &new_verify_col);
      if (curr_view->verify_col != new_verify_col) {
        curr_view->verify_col = new_verify_col;
        build_screen(curr_screen);
        display_screen(curr_screen);
      }
      wmove(SCREEN_WINDOW(curr_screen), y, new_screen_col);
      break;
    case WINDOW_COMMAND:
      getyx(SCREEN_WINDOW(curr_screen), y, x);
      calculate_new_column(curr_screen, curr_view, x, cmd_verify_col, col, &new_screen_col, &new_verify_col);
      if (cmd_verify_col != new_verify_col) {
        cmd_verify_col = new_verify_col;
        display_cmdline(curr_screen, curr_view);
      }
      wmove(SCREEN_WINDOW(curr_screen), y, new_screen_col);
      break;
    default:                   /* PREFIX */
      /*
       * Don't do anything for PREFIX window...
       */
      break;
  }
  /*
   * Don't have any code here, as PREFIX window is supposed to do nothing
   */
  return (RC_OK);
}

short execute_find_command(uchar *str, long target_type) {
  short rc = RC_OK;
  long save_zone_start = CURRENT_VIEW->zone_start;
  long save_zone_end = CURRENT_VIEW->zone_end;
  bool save_arbchar_status = CURRENT_VIEW->arbchar_status;
  bool save_hex = CURRENT_VIEW->hex;
  bool negative = FALSE, wrapped = FALSE;
  uchar save_arbchar_single = CURRENT_VIEW->arbchar_single;
  TARGET target;
  long true_line = 0L;
  long save_focus_line = CURRENT_VIEW->focus_line;
  long save_current_line = CURRENT_VIEW->current_line;

  if (strcmp((char *) str, "") == 0) {  /* no argument supplied */
    if (lastop[LASTOP_FIND].value == NULL || strcmp((char *) lastop[LASTOP_FIND].value, "") == 0) {
      display_error(39, (uchar *) "", FALSE);
      return (RC_INVALID_OPERAND);
    } else {
      str = lastop[LASTOP_FIND].value;
    }
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  initialise_target(&target);
  /*
   * Force the ZONE settings to start at 1 and end at parameter length.
   * Ensure ARBCHAR and HEX will be ignored in the search...
   */
  CURRENT_VIEW->zone_start = 1;
  CURRENT_VIEW->zone_end = strlen((char *) str);
  CURRENT_VIEW->arbchar_status = TRUE;
  CURRENT_VIEW->arbchar_single = find_unique_char(str);
  CURRENT_VIEW->hex = FALSE;
  rc = validate_target(str, &target, target_type, get_true_line(TRUE), TRUE, FALSE);
  if (rc == RC_TARGET_NOT_FOUND && CURRENT_VIEW->wrap) {
    wrapped = TRUE;
    negative = target.rt[0].negative;
    free_target(&target);
    initialise_target(&target);
    true_line = (negative ? CURRENT_FILE->number_lines + 1 : 0L);
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    rc = validate_target(str, &target, target_type, true_line, TRUE, FALSE);
  }
  /*
   * Put ZONE, ARBCHAR and HEX back the way they were...
   */
  CURRENT_VIEW->zone_start = save_zone_start;
  CURRENT_VIEW->zone_end = save_zone_end;
  CURRENT_VIEW->arbchar_status = save_arbchar_status;
  CURRENT_VIEW->arbchar_single = save_arbchar_single;
  CURRENT_VIEW->hex = save_hex;
  /*
   * Save the last target...
   */
  if (save_lastop(LASTOP_FIND, str) != RC_OK) {
    display_error(30, (uchar *) "", FALSE);
    return rc;
  }
  /*
   * Check for target not found...
   */
  if (rc == RC_TARGET_NOT_FOUND) {
    if (wrapped) {
      /*
       * Put the current and focus lines back the way they were before wrapping.
       */
      CURRENT_VIEW->focus_line = save_focus_line;
      CURRENT_VIEW->current_line = save_current_line;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
    display_error(34, (uchar *) "", FALSE);
    free_target(&target);
    return (rc);
  }
  /*
   * Target found, so advance the cursor...
   */
  if (wrapped) {
    CURRENT_VIEW->focus_line = save_focus_line;
    CURRENT_VIEW->current_line = save_current_line;
    build_screen(current_screen);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND || compatible_feel == COMPAT_XEDIT) {
      CURRENT_VIEW->current_line = true_line;
    } else {
      CURRENT_VIEW->focus_line = true_line;
    }
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  rc = advance_current_or_focus_line(target.num_lines);
  free_target(&target);
  if (wrapped) {
    display_error(0, (uchar *) "Wrapped...", FALSE);
  }
  return (rc);
}

short execute_modify_command(uchar *str) {
  register short i = 0;
  short itemno = 0;
  uchar item_type = 0;

  if ((itemno = find_query_item(str, strlen((char *) str), &item_type)) == (-1) || !(item_type & QUERY_MODIFY)) {
    display_error(1, str, FALSE);
    return (RC_INVALID_OPERAND);
  }
  itemno = get_item_values(1, itemno, (uchar *) "", QUERY_MODIFY, 0L, NULL, 0L);
  strcpy((char *) temp_cmd, "set");
  for (i = 0; i < itemno + 1; i++) {
    strcat((char *) temp_cmd, " ");
    strcat((char *) temp_cmd, (char *) item_values[i].value);
  }
  return (RC_OK);
}

long calculate_rec_len(short action, uchar *rec, long current_rec_len, long start_col, long num_cols, short trailing) {
  long new_rec_len = 0;
  long end_col = start_col + num_cols - 1;

  /*
   * start_col is 1 based; ie first column is column 1
   */
  if (num_cols <= 0) {
    new_rec_len = current_rec_len;
  } else if (trailing == TRAILING_OFF) {
    new_rec_len = memrevne(rec, ' ', max_line_length);
    if (new_rec_len == (-1)) {
      new_rec_len = 0;
    } else {
      new_rec_len = new_rec_len + 1;
    }
  } else {
    switch (action) {
      case ADJUST_DELETE:
        if (start_col < current_rec_len) {
          new_rec_len = (long) (long) current_rec_len - (min((long) current_rec_len - (long) start_col, num_cols));
        }
        break;
      case ADJUST_INSERT:
        if (start_col > current_rec_len) {
          new_rec_len = min(end_col, max_line_length);
        } else {
          new_rec_len = min(current_rec_len + num_cols, max_line_length);
        }
        break;
      case ADJUST_OVERWRITE:
        if (end_col > current_rec_len) {
          new_rec_len = min(end_col, max_line_length);
        } else {
          new_rec_len = min(current_rec_len, max_line_length);
        }
        break;
    }
  }
  return (new_rec_len);
}

static short set_editv(uchar *var, uchar *val, bool editv_file, bool rexx_var) {
  short rc = RC_OK;
  LINE *curr = NULL;
  int len_var = 0, len_val = 0;
  uchar *value = NULL;
  LINE *first = NULL;

  first = (editv_file) ? CURRENT_FILE->editv : editv;
  var = make_upper(var);
  if (rexx_var) {
    rc = get_rexx_variable(var, &value, &len_val);
    if (rc != RC_OK) {
      return (rc);
    }
  } else {
    if (val == NULL) {
      value = (uchar *) "";
    } else {
      value = val;
    }
    len_val = strlen((char *) value);
  }
  len_var = strlen((char *) var);
  curr = lll_locate(first, var);
  if (curr) {                   /* found an existing variable */
    if (len_val > curr->length) {
      curr->line = (uchar *) realloc(curr->line, (len_val + 1) * sizeof(uchar));
      if (curr->line == NULL) {
        if (rexx_var && value) {
          free(value);
        }
        display_error(30, (uchar *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
    }
    if (len_val == 0) {         /* need to delete the entry */
      if (editv_file) {
        lll_del(&(CURRENT_FILE->editv), NULL, curr, DIRECTION_FORWARD);
      } else {
        lll_del(&editv, NULL, curr, DIRECTION_FORWARD);
      }
    } else {
      strcpy((char *) curr->line, (char *) value);
    }
  } else {
    curr = lll_add(first, NULL, sizeof(LINE));
    if (curr == NULL) {
      if (rexx_var && value) {
        free(value);
      }
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    /*
     * As the current variable is always inserted as the start of the LL
     * we must set the first pointer to the current variable each time.
     */
    if (editv_file) {
      CURRENT_FILE->editv = curr;
    } else {
      editv = curr;
    }
    curr->line = (uchar *) malloc((len_val + 1) * sizeof(uchar));
    if (curr->line == NULL) {
      if (rexx_var && value) {
        free(value);
      }
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) curr->line, (char *) value);
    curr->length = len_val;
    curr->name = (uchar *) malloc((len_var + 1) * sizeof(uchar));
    if (curr->name == NULL) {
      if (rexx_var && value) {
        free(value);
      }
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) curr->name, (char *) var);
  }
  if (rexx_var && value) {
    free(value);
  }
  return (rc);
}

#define EDITV_PARAMS  2

short execute_editv(short editv_type, bool editv_file, uchar *params) {
  uchar *word[EDITV_PARAMS + 1];
  uchar strip[EDITV_PARAMS];
  uchar *p = NULL, *str = NULL;
  unsigned short num_params = 0;
  LINE *curr = NULL, *first = NULL;
  int key = 0, lineno = 0, i, len_str, len_name, rem, x;
  short rc = RC_OK;

  first = (editv_file) ? CURRENT_FILE->editv : editv;
  switch (editv_type) {
    case EDITV_SETL:
      strip[0] = STRIP_BOTH;
      strip[1] = STRIP_NONE;
      num_params = param_split(params, word, EDITV_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
      if (num_params == 0) {
        display_error(3, (uchar *) "", FALSE);
        return (RC_INVALID_OPERAND);
      }
      rc = set_editv(word[0], word[1], editv_file, FALSE);
      break;
    case EDITV_SET:
      p = (uchar *) strtok((char *) params, " ");
      while (p != NULL) {
        str = (uchar *) strtok(NULL, " ");
        rc = set_editv(p, str, editv_file, FALSE);
        if (str == NULL) {
          break;
        }
        p = (uchar *) strtok(NULL, " ");
      }
      break;
    case EDITV_PUT:
      p = (uchar *) strtok((char *) params, " ");
      while (p != NULL) {
        rc = set_editv(p, NULL, editv_file, TRUE);
        p = (uchar *) strtok(NULL, " ");
      }
      break;
    case EDITV_GET:
      p = (uchar *) strtok((char *) params, " ");
      while (p != NULL) {
        p = make_upper(p);
        curr = lll_locate(first, p);
        if (curr && curr->line) {
          str = curr->line;
        } else {
          str = (uchar *) "";
        }
        if (set_rexx_variable(p, str, strlen((char *) str), -1) != RC_OK) {
          break;
        }
        p = (uchar *) strtok(NULL, " ");
      }
      break;
    case EDITV_GETSTEM:
      len_name = strlen((char *) params);
      if (params[len_name - 1] != '.') {
        /* check if ends in "." */
        display_error(149, params, FALSE);
        return (RC_INVALID_OPERAND);
      }
      curr = first;
      while (curr) {
        if (curr->name && strlen((char *) curr->name) > len_name && memcmpi(curr->name, params, len_name) == 0) {
          if (curr->line) {
            str = curr->line;
          } else {
            str = (uchar *) "";
          }
          rc = set_rexx_variable(curr->name, str, strlen((char *) str), -1);
          if (rc != RC_OK) {
            break;
          }
        }
        curr = curr->next;
      }
      break;
    case EDITV_LIST:
      wclear(stdscr);
      if (blank_field(params)) {
        curr = first;
        while (curr != NULL) {
          if (curr->line) {
            str = curr->line;
          } else {
            str = (uchar *) "";
          }
          attrset(A_BOLD);
          mvaddstr(lineno, 0, (char *) curr->name);
          attrset(A_NORMAL);
          /*
           * Calculate maximum length of string to display so we don't wrap.
           */
          if (curr) {
            len_name = strlen((char *) curr->name);
          } else {
            len_name = 0;
          }
          len_str = strlen((char *) str);
          rem = terminal_cols - len_name - 1;
          /*
           * Display the value, wrapping if necessary
           */
          move(lineno, 1 + len_name);
          for (x = 0, i = 0; i < len_str; i++) {
            if (x == rem) {
              x = 1;
              lineno++;
              move(lineno, 1 + len_name);
            } else {
              x++;
            }
            addch(*(str + i));
          }
          lineno++;
          curr = curr->next;
        }
      } else {
        p = (uchar *) strtok((char *) params, " ");
        while (p != NULL) {
          p = make_upper(p);
          curr = lll_locate(first, p);
          if (curr && curr->line) {
            str = curr->line;
          } else {
            str = (uchar *) "";
          }
          attrset(A_BOLD);
          mvaddstr(lineno, 0, (char *) p);
          attrset(A_NORMAL);
          /*
           * Calculate maximum length of string to display so we don't wrap.
           */
          if (curr) {
            len_name = strlen((char *) curr->name);
          } else {
            len_name = 0;
          }
          len_str = strlen((char *) str);
          rem = terminal_cols - len_name - 1;
          /*
           * Display the value, wrapping if necessary
           */
          move(lineno, 1 + len_name);
          for (x = 0, i = 0; i < len_str; i++) {
            if (x == rem) {
              x = 1;
              lineno++;
              move(lineno, 1 + len_name);
            } else {
              x++;
            }
            addch(*(str + i));
          }
          lineno++;
          p = (uchar *) strtok(NULL, " ");
        }
      }
      mvaddstr(terminal_lines - 2, 0, HIT_ANY_KEY);
      refresh();
      for (;;) {
        key = my_getch(stdscr);
        if (key == KEY_MOUSE) {
          continue;
        }
        if (is_termresized()) {
          continue;
        }
        break;
      }
      Redraw((uchar *) "");
      break;
  }
  return (rc);
}

#define STATE_START        0
#define STATE_EDITFIELD    1
#define STATE_TITLE        2
#define STATE_BUTTON       3
#define STATE_DEFBUTTON    4
#define STATE_ICON         5

short prepare_dialog(uchar *params, bool alert, uchar *stemname) {
  short rc = RC_OK;
  int len_params = strlen((char *) params);
  uchar delimiter;
  int len_prompt, len_title, len_initial, i, j, k, state = STATE_START;
  short button = BUTTON_OK;
  short default_button = 1;
  uchar *prompt = NULL, *initial = NULL, *title = NULL;
  bool editfield = FALSE;
  bool found;
  int button_len[4];
  int button_num[4];
  char *button_text[4];
  int icon_len[4];
  char *icon_text[4];
  char *strdefbutton = "defbutton";
  char *streditfield = "editfield";
  char num[20];
  short icon;

  /*
   * Can only run from a Rexx macro...
   */
  if (!in_macro || !rexx_support) {
    display_error(53, (uchar *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  /*
   * Defining these constants this way is done because
   * non-ansi compilers can't handle definitions like int a[2] = {0,1};
   */
  button_len[0] = 2;
  button_text[0] = "ok";
  button_num[0] = 1;
  button_len[1] = 8;
  button_text[1] = "okcancel";
  button_num[1] = 2;
  button_len[2] = 5;
  button_text[2] = "yesno";
  button_num[2] = 2;
  button_len[3] = 11;
  button_text[3] = "yesnocancel";
  button_num[3] = 3;

  icon_len[0] = 15;
  icon_text[0] = "iconexclamation";
  icon_len[1] = 15;
  icon_text[1] = "iconinformation";
  icon_len[2] = 12;
  icon_text[2] = "iconquestion";
  icon_len[3] = 8;
  icon_text[3] = "iconstop";
  /*
   * got to be something to display...
   */
  if (len_params < 2) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Set the default icon and buttons in case these are not supplied
   */
  if (alert) {
    icon = ICON_STOP;
    button = BUTTON_OK;
  } else {
    icon = ICON_NONE;
    button = BUTTON_OKCANCEL;
  }
  /*
   * The first character of the string is the delimiter for the prompt
   */
  delimiter = *(params);
  params++;                     /* throw away first delimiter */
  for (i = 0; i < len_params; i++) {
    if (*(params + i) == delimiter) {
      break;
    }
  }
  /*
   * Did we get a delimiter ?
   */
  if (i == len_params) {        /* no, then use defaults */
    prompt = (uchar *) malloc(len_params * sizeof(uchar));
    if (prompt == NULL) {
      display_error(30, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    strcpy((char *) prompt, (char *) params);
  } else {                      /* yes, we may have other options... */
    len_prompt = i;
    prompt = (uchar *) malloc(1 + (len_prompt * sizeof(uchar)));
    if (prompt == NULL) {
      display_error(30, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    memcpy(prompt, params, len_prompt);
    prompt[len_prompt] = '\0';
    params += len_prompt + 1;
    strtrunc(params);
    len_params = strlen((char *) params);
    for (i = 0; i < len_params;) {
      if (len_params == 0) {
        break;
      }
      switch (state) {
        case STATE_START:
          if (params[i] == 'E' || params[i] == 'e') {
            state = STATE_EDITFIELD;
            break;
          }
          if (params[i] == 'T' || params[i] == 't') {
            state = STATE_TITLE;
            break;
          }
          if (params[i] == 'O' || params[i] == 'o' || params[i] == 'Y' || params[i] == 'y') {
            state = STATE_BUTTON;
            break;
          }
          if (params[i] == 'D' || params[i] == 'd') {
            state = STATE_DEFBUTTON;
            break;
          }
          if (params[i] == 'I' || params[i] == 'i') {
            state = STATE_ICON;
            break;
          }
          if (params[i] == ' ') {
            i++;
            break;
          }
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        case STATE_EDITFIELD:
          if (len_params < 4) {
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          for (i = 0; i < len_params; i++) {
            if (*(params + i) == ' ') {
              break;
            }
            if (isupper(*(params + i))) {
              *(params + i) = tolower(*(params + i));
            }
            if (*(params + i) != streditfield[i]) {
              rc = RC_INVALID_OPERAND;
              break;
            }
          }
          if (rc == RC_INVALID_OPERAND) {
            display_error(1, params, FALSE);
            break;
          }
          params += i;
          strtrunc(params);
          len_params = strlen((char *) params);
          editfield = TRUE;
          delimiter = *(params);
          if (delimiter == 'T' || delimiter == 't' || delimiter == 'Y' || delimiter == 'y' || delimiter == 'O' || delimiter == 'o' || delimiter == 'D' || delimiter == 'd' || delimiter == 'I' || delimiter == 'i' || delimiter == '\0') {
            state = STATE_START;
            i = 0;
            break;
          }
          params++;
          for (i = 0; i < len_params; i++) {
            if (*(params + i) == delimiter) {
              break;
            }
          }
          if (i == len_params) {        /* no, then error */
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          len_initial = i;
          initial = (uchar *) malloc(1 + (len_initial * sizeof(uchar)));
          if (initial == NULL) {
            display_error(30, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          memcpy(initial, params, len_initial);
          initial[len_initial] = '\0';
          params += (len_initial + 1);  /* add 1 for delimiter */
          strtrunc(params);
          len_params = strlen((char *) params);
          state = STATE_START;
          i = 0;
          break;
        case STATE_TITLE:
          if (len_params < 6 || memcmpi(params, (uchar *) "title ", 6) != 0) {
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          params += 6;
          strtrunc(params);
          delimiter = *(params);
          params++;
          len_params = strlen((char *) params);
          for (i = 0; i < len_params; i++) {
            if (*(params + i) == delimiter) {
              break;
            }
          }
          if (i == len_params) {        /* no, then error */
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          len_title = i + 2;    /* allow for leading and trailing space */
          title = (uchar *) malloc(1 + (len_title * sizeof(uchar)));
          if (title == NULL) {
            display_error(30, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          memset(title, ' ', len_title);
          memcpy(title + 1, params, len_title - 2);
          title[len_title] = '\0';
          params += (len_title - 1);    /* remove 2 spaces and add 1 for delimiter */
          strtrunc(params);
          len_params = strlen((char *) params);
          i = 0;
          state = STATE_START;
          break;
        case STATE_ICON:
          if (len_params >= 5 && memcmpi(params, (uchar *) "icon", 4) == 0) {
            /*
             * Find the first space, or end of string;
             * set the space to nul, and see if the "string" matches one of the valid icon strings.
             */
            for (j = 0; j < len_params; j++) {
              if (*params + j == ' ') {
                *(params + j) = '\0';
                break;
              }
            }
            found = FALSE;
            for (k = 0; k < 4; k++) {
              if (equal((uchar *) icon_text[k], params, 5)) {
                icon = k;
                found = TRUE;
                break;
              }
            }
            /*
             * Reset the space we changed
             */
            *(params + j) = ' ';
            if (found == FALSE) {
              display_error(1, params, FALSE);
              rc = RC_INVALID_OPERAND;
              break;
            }
          } else {
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          params += icon_len[icon];
          strtrunc(params);
          len_params = strlen((char *) params);
          state = STATE_START;
          i = 0;
          break;
        case STATE_BUTTON:
          button = -1;
          for (i = 0; i < 4; i++) {
            if (len_params >= button_len[i] && memcmpi(params, (uchar *) button_text[i], button_len[i]) == 0 && (len_params == button_len[i] || *(params + button_len[i]) == ' ')) {
              button = i;
              break;
            }
          }
          if (button == (-1)) {
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          params += button_len[i];
          strtrunc(params);
          len_params = strlen((char *) params);
          state = STATE_START;
          i = 0;
          break;
        case STATE_DEFBUTTON:
          if (len_params < 6) {
            display_error(1, params, FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
          for (i = 0; i < len_params; i++) {
            if (*(params + i) == ' ') {
              break;
            }
            if (isupper(*(params + i))) {
              *(params + i) = tolower(*(params + i));
            }
            if (*(params + i) != strdefbutton[i]) {
              rc = RC_INVALID_OPERAND;
              break;
            }
          }
          if (rc == RC_INVALID_OPERAND) {
            display_error(1, params, FALSE);
            break;
          }
          params += i;
          strtrunc(params);
          len_params = strlen((char *) params);
          for (i = 0; i < len_params; i++) {
            if (*(params + i) == ' ') {
              break;
            }
            if (!isdigit(*(params + i))) {
              rc = RC_INVALID_OPERAND;
              break;
            }
            num[i] = *(params + i);
          }
          if (rc == RC_INVALID_OPERAND) {
            display_error(1, params, FALSE);
            break;
          }
          num[i] = '\0';
          default_button = atoi(num);
          params += i;
          strtrunc(params);
          len_params = strlen((char *) params);

          state = STATE_START;
          i = 0;
          break;
        default:
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
      }
      if (rc != RC_OK) {
        break;
      }
    }
  }
  if (default_button < 1) {
    display_error(5, (uchar *) num, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  if (default_button > button_num[button]) {
    display_error(6, (uchar *) num, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  if (rc == RC_OK) {
    rc = execute_dialog(prompt, title, initial, editfield, button, (short) (default_button - 1), stemname, icon, alert);
  }
  if (prompt) {
    free(prompt);
  }
  if (initial) {
    free(initial);
  }
  if (title) {
    free(title);
  }
  return (rc);
}

short execute_dialog(uchar *prompt, uchar *title, uchar *initial, bool editfield, short button, short default_button, uchar *stemname, short icon, bool alert) {
  short rc = RC_OK;
  int key, num_buttons = 0, i;
  short title_length = 0, initial_length = 0, max_width, cursor_pos = 0;
  short prompt_length = 0, prompt_max_length = 0, prompt_lines = 0;
  char *prompt_line[MAXIMUM_DIALOG_LINES + 2];
  WINDOW *dialog_win = NULL;
  WINDOW *save_command_window = NULL;
  uchar *save_cmd_rec = NULL;
  uchar *editfield_buf = NULL;
  long save_cmd_rec_len;
  short dw_lines, dw_cols, dw_y, dw_x;
  short editfield_col = -1;
  char *button_text[3];
  short button_len[3];
  short button_col[3];
  short max_button_len = 0;
  short item_selected = -1;
  uchar button_buf[15];
  bool in_editfield;
  long save_max_line_length = 0;
  uchar save_current_window = CURRENT_VIEW->current_window;

  /*
   * Split the prompt up into multiple lines if applicable
   */
  if (prompt) {
    prompt_line[prompt_lines] = (char *) prompt;
    prompt_length = strlen((char *) prompt);
    for (i = 0; i < prompt_length && prompt_lines < MAXIMUM_DIALOG_LINES; i++) {
      if (prompt[i] == 10) {
        prompt_lines++;
        prompt[i] = '\0';
        if (strlen(prompt_line[prompt_lines - 1]) > prompt_max_length) {
          prompt_max_length = strlen(prompt_line[prompt_lines - 1]);
        }
        prompt_line[prompt_lines] = (char *) prompt + i + 1;
      }
    }
    prompt_lines++;
    if (strlen(prompt_line[prompt_lines - 1]) > prompt_max_length) {
      prompt_max_length = strlen(prompt_line[prompt_lines - 1]);
    }
  }
  /*
   * work out dimensions of dialog box based on length of prompt, title, buttons, initial value and width of screen
   */
  max_width = terminal_cols - 2;
  if (title) {
    title_length = strlen((char *) title);
  }
  if (initial) {
    initial_length = strlen((char *) initial);
  }
  if (button == BUTTON_OK) {
    max_button_len = 8;
  } else if (button == BUTTON_YESNO) {
    max_button_len = 16;
  } else if (button == BUTTON_YESNOCANCEL) {
    max_button_len = 25;
  } else if (button == BUTTON_OKCANCEL) {
    max_button_len = 18;
  }
  dw_cols = max_button_len;
  if (prompt_max_length + 2 > dw_cols) {
    dw_cols = prompt_max_length + 2;
  }
  if (title_length + 3 > dw_cols) {
    dw_cols = title_length + 2;
  }
  if (initial_length + 2 > dw_cols) {
    dw_cols = initial_length + 2;
  }
  dw_cols += 3;
  if (dw_cols > max_width) {
    dw_cols = max_width;
  }
  dw_lines = 6 + prompt_lines + ((editfield) ? 2 : 0);
  dw_x = (terminal_cols - dw_cols) / 2;
  dw_y = (terminal_lines - dw_lines) / 2;
  memset(button_len, 0, sizeof(button_len));
  memset(button_col, 0, sizeof(button_col));
  memset(button_text, 0, sizeof(button_text));
  switch (button) {
    case BUTTON_OK:
      num_buttons = 1;
      button_text[0] = " OK ";
      button_len[0] = 4;
      button_col[0] = (dw_cols - 4) / 2;
      break;
    case BUTTON_YESNO:
      num_buttons = 2;
      button_text[0] = " YES ";
      button_len[0] = 5;
      button_text[1] = " NO ";
      button_len[1] = 4;
      button_col[0] = (dw_cols / 4) - (button_len[0] / 2);
      button_col[1] = (dw_cols / 2) + ((dw_cols / 4) - (button_len[1] / 2));
      break;
    case BUTTON_OKCANCEL:
      num_buttons = 2;
      button_text[0] = " OK ";
      button_len[0] = 4;
      button_text[1] = " CANCEL ";
      button_len[1] = 8;
      button_col[0] = (dw_cols / 4) - (button_len[0] / 2);
      button_col[1] = (dw_cols / 2) + ((dw_cols / 4) - (button_len[1] / 2));
      break;
    case BUTTON_YESNOCANCEL:
      num_buttons = 3;
      button_text[0] = " YES ";
      button_len[0] = 5;
      button_text[1] = " NO ";
      button_len[1] = 4;
      button_text[2] = " CANCEL ";
      button_len[2] = 8;
      button_col[0] = (dw_cols / 6) - (button_len[0] / 2);
      button_col[1] = (dw_cols / 3) + ((dw_cols / 6) - (button_len[1] / 2));
      button_col[2] = (2 * (dw_cols / 3)) + ((dw_cols / 6) - (button_len[2] / 2));
      break;
  }
  /*
   * Create the dialog window
   */
  dialog_win = newwin(dw_lines, dw_cols, dw_y, dw_x);
  if (dialog_win == NULL) {
    CURRENT_VIEW->current_window = save_current_window;
    THERefresh((uchar *) "");
    restore_THE();
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  keypad(dialog_win, TRUE);
  if (editfield) {
    editfield_buf = (uchar *) malloc(dw_cols + 1);
    if (editfield_buf == NULL) {
      CURRENT_VIEW->current_window = save_current_window;
      delwin(dialog_win);
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    if (initial) {
      strcpy((char *) editfield_buf, (char *) initial);
    } else {
      strcpy((char *) editfield_buf, "");
    }
    /*
     * Save the CMDLINE contents
     */
    save_cmd_rec_len = cmd_rec_len;
    save_cmd_rec = (uchar *) malloc(save_cmd_rec_len + 1);
    if (save_cmd_rec == NULL) {
      CURRENT_VIEW->current_window = save_current_window;
      delwin(dialog_win);
      free(editfield_buf);
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    memcpy((char *) save_cmd_rec, (char *) cmd_rec, cmd_rec_len);
    save_cmd_rec[cmd_rec_len] = '\0';
    /*
     * Save the CMDLINE window and create a new one in our dialog window
     */
    save_command_window = CURRENT_WINDOW_COMMAND;
    CURRENT_WINDOW_COMMAND = derwin(dialog_win, 1, dw_cols - 4, 3 + prompt_lines, 2);
    if (CURRENT_WINDOW_COMMAND == (WINDOW *) NULL) {
      CURRENT_VIEW->current_window = save_current_window;
      delwin(CURRENT_WINDOW_COMMAND);
      CURRENT_WINDOW_COMMAND = save_command_window;
      delwin(dialog_win);
      free(save_cmd_rec);
      free(editfield_buf);
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    wattrset(CURRENT_WINDOW_COMMAND, set_colour(CURRENT_FILE->attr + ATTR_DIA_EDITFIELD));
  }
  wbkgd(dialog_win, set_colour(CURRENT_FILE->attr + ((alert) ? ATTR_ALERT : ATTR_DIALOG)));
  wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_DIA_BORDER));
  box(dialog_win, 0, 0);
  wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ((alert) ? ATTR_ALERT : ATTR_DIALOG)));
  /*
   * Add the prompt line(s) to the window
   */
  for (i = 0; i < prompt_lines; i++) {
    wmove(dialog_win, 2 + i, 2);
    waddstr(dialog_win, (char *) prompt_line[i]);
  }
  /*
   * Add the title to the window
   */
  if (title) {
    wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_DIA_BORDER));
    wmove(dialog_win, 0, 1);
    waddstr(dialog_win, (char *) title);
  }
  /*
   * Prepare the editfield if we have one
   */
  if (editfield) {
    /*
     * Now we have passed any checks that result in a premature return change the contents of the cmd_rec
     */
    in_editfield = TRUE;
    draw_cursor(TRUE);
    save_max_line_length = max_line_length;
    max_line_length = dw_cols - 2;
    default_button = -1;
  } else {
    wmove(dialog_win, dw_lines - 3, cursor_pos);
    in_editfield = FALSE;
    draw_cursor(FALSE);
  }
  inDIALOG = TRUE;
  for (;;) {
    /*
     * Draw the buttons...
     */
    for (i = 0; i < num_buttons; i++) {
      if (default_button == i) {
        wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_DIA_ABUTTON));
        cursor_pos = button_col[i];
      } else {
        wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_DIA_BUTTON));
      }
      wmove(dialog_win, dw_lines - 3, button_col[i]);
      waddstr(dialog_win, button_text[i]);
    }
    if (in_editfield) {
      /*
       * Go into the pseudo command line and process it until an exit key is pressed.
       * On exit make the first button active.
       */
      draw_cursor(TRUE);
      rc = readv_cmdline(editfield_buf, dialog_win, editfield_col);
      memcpy((char *) editfield_buf, (char *) cmd_rec, cmd_rec_len);
      editfield_buf[cmd_rec_len] = '\0';
      in_editfield = FALSE;
      draw_cursor(FALSE);
      default_button = 0;
      editfield_col = -1;
      if (rc == RC_READV_TERM_MOUSE) {
        /*
         * we terminated the readv_cmdline() with the press of a mouse button outside the CMDLINE.
         * make it look like the mouse was just clicked so we can check if the click happened on a button
         */
        key = KEY_MOUSE;
      } else {
        continue;
      }
    } else {
      /*
       * Display the dialog window and pseudo command line and get a key.
       */
      wrefresh(dialog_win);
      if (editfield) {
        wrefresh(CURRENT_WINDOW_COMMAND);
      }
      key = my_getch(CURRENT_WINDOW_COMMAND);
    }
    if (key == KEY_MOUSE) {
      int b, ba, bm, y, x;

      if (get_mouse_info(&b, &ba, &bm) != RC_OK) {
        continue;
      }
      if (b != 1 || ba == BUTTON_PRESSED) {
        continue;
      }
      wmouse_position(dialog_win, &y, &x);
      if (y == -1 && x == -1) {
        /*
         * Button 1 clicked or released outside of window; ignore it
         */
        continue;
      }
      /*
       * Check that the mouse is clicked on a button
       */
      if (y == dw_lines - 3) {
        bool found = FALSE;

        for (i = 0; i < num_buttons; i++) {
          if (x >= button_col[i] && x <= (button_col[i] + button_len[i])) {
            found = TRUE;
            break;
          }
        }
        if (!found) {
          continue;
        }
      } else if (y == dw_lines - 5 && editfield && x > 1 && x < dw_cols - 2) {
        /*
         * Clicked somewhere on the editfield
         */
        in_editfield = TRUE;
        default_button = -1;
        editfield_col = x - 2;
        continue;
      } else {
        /*
         * Clicked somewhere other than the button line
         */
        continue;
      }
      /*
       * Got a valid button. Check if its a click or press
       */
      if (ba == BUTTON_CLICKED || ba == BUTTON_RELEASED) {
        /*
         * Got a valid line. Redisplay it in highlighted mode.
         */
        item_selected = i;
        touchwin(dialog_win);
        wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_DIA_ABUTTON));
        wmove(dialog_win, dw_lines - 3, button_col[i]);
        waddstr(dialog_win, button_text[i]);
        wrefresh(dialog_win);
        break;
      }
      continue;
    } else {
      if (key == 9) {
        if (++default_button == num_buttons) {
          if (editfield) {
            in_editfield = TRUE;
          } else {
            default_button = 0;
          }
        }
      } else if (key == 'q') {
        item_selected = default_button;
        break;
      } else if (key == 10 || key == 13) {
        item_selected = default_button;
        break;
      }
    }
  }
  delwin(dialog_win);
  draw_cursor(TRUE);
  /*
   * Set DIALOG.2 to the button pressed
   */
  strcpy((char *) button_buf, (char *) button_text[item_selected]);
  strtrunc(button_buf);
  set_rexx_variable(stemname, button_buf, strlen((char *) button_buf), 2);
  /*
   * Set DIALOG.1 to value of editfield2
   */
  if (editfield) {
    set_rexx_variable(stemname, editfield_buf, strlen((char *) editfield_buf), 1);
  } else {
    set_rexx_variable(stemname, (uchar *) "", 0, 1);
  }
  /*
   * Set DIALOG.0 to 2
   */
  set_rexx_variable(stemname, (uchar *) "2", 1, 0);
  /*
   * If we had an editfield, restore the CMDLINE window and contents (if there
   * was a CMDLINE window)
   */
  inDIALOG = FALSE;
  CURRENT_VIEW->current_window = save_current_window;
  if (editfield) {
    delwin(CURRENT_WINDOW_COMMAND);
    CURRENT_WINDOW_COMMAND = save_command_window;
    max_line_length = save_max_line_length;
    memset(cmd_rec, ' ', max_line_length);
    cmd_rec_len = 0;
    if (CURRENT_WINDOW_COMMAND) {
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
      CURRENT_VIEW->cmdline_col = -1;
      if (save_cmd_rec[0] == '&') {
        Cmsg(save_cmd_rec);
      }
    }
    free(save_cmd_rec);
    free(editfield_buf);
  }
  THERefresh((uchar *) "");
  restore_THE();
  return (rc);
}

int get_non_separator_line(int current_line, int num_args, uchar **args, int direction) {
  int i;
  int offset_lines = 0;

  if (direction > 0) {
    for (i = current_line; i < num_args; i++) {
      if (args[i][0] != '-') {
        offset_lines = i - current_line;
        break;
      }
    }
  } else {
    for (i = current_line; i > -1; i--) {
      if (args[i][0] != '-') {
        offset_lines = current_line - i;
        break;
      }
    }
  }
  return offset_lines;
}

#define STATE_POPUP_START            0
#define STATE_POPUP_ESCAPE           1
#define STATE_POPUP_INITIAL          2
#define STATE_POPUP_LOCATION_TEXT    3
#define STATE_POPUP_LOCATION_CENTRE  4
#define STATE_POPUP_LOCATION_MOUSE   5
#define STATE_POPUP_CONTENT          6
#define STATE_POPUP_LOCATION_ABOVE   7
#define STATE_POPUP_LOCATION_BELOW   8
#define STATE_POPUP_KEYS             9

short prepare_popup(uchar *params) {
  int len_params = strlen((char *) params);
  uchar delimiter = ' ';
  int i, j, state = STATE_START, other_options = 0;
  char str_initial[20];
  short rc = RC_OK, num_items = 0, len;
  short height = 0, width = 0, pad_height = 0, pad_width = 0;
  int x = -1, y = -1, args_allocated;
  unsigned short begy, begx;
  uchar **args = NULL;
  bool invalid_item = FALSE, trailing_delimiter;
  int initial = 0;
  char key_name[30];
  char location = 'C';
  char *keyname_start[MAXIMUM_POPUP_KEYS];
  int keyname_index = 0;

  /*
   * Must run from a Rexx macro...
   */
  if (!in_macro || !rexx_support) {
    display_error(53, (uchar *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  strtrunc(params);
  len_params = strlen((char *) params);
  for (i = 0; i < len_params;) {
    if (len_params == 0) {
      break;
    }
    switch (state) {
      case STATE_START:
        if (params[i] == 'E' || params[i] == 'e') {
          state = STATE_POPUP_ESCAPE;
          break;
        }
        if (params[i] == 'C' || params[i] == 'c') {
          state = STATE_POPUP_LOCATION_CENTRE;
          break;
        }
        if (params[i] == 'M' || params[i] == 'm') {
          state = STATE_POPUP_LOCATION_MOUSE;
          break;
        }
        if (params[i] == 'T' || params[i] == 't') {
          state = STATE_POPUP_LOCATION_TEXT;
          break;
        }
        if (params[i] == 'I' || params[i] == 'i') {
          state = STATE_POPUP_INITIAL;
          break;
        }
        if (params[i] == 'B' || params[i] == 'b') {
          state = STATE_POPUP_LOCATION_BELOW;
          break;
        }
        if (params[i] == 'A' || params[i] == 'a') {
          state = STATE_POPUP_LOCATION_ABOVE;
          break;
        }
        if (params[i] == 'K' || params[i] == 'k') {
          state = STATE_POPUP_KEYS;
          break;
        }
        if (params[i] == ' ') {
          i++;
          break;
        }
        /*
         * Must be the start of the popup contents...
         */
        delimiter = *(params);
        params++;               /* throw away first delimiter */
        state = STATE_POPUP_CONTENT;
        break;
      case STATE_POPUP_ESCAPE:
        if (len_params < 6 || memcmpi(params, (uchar *) "escape", 6) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        params += 6;
        strtrunc(params);
        len_params = strlen((char *) params);
        for (j = 0; j < len_params; j++) {
          if (*(params + j) == ' ') {
            break;
          }
          key_name[j] = *(params + j);
        }
        key_name[j] = '\0';
        popup_escape_key = find_key_name((uchar *) key_name);
        if (popup_escape_key == -1) {
          display_error(13, (uchar *) key_name, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        params += j;
        strtrunc(params);
        len_params = strlen((char *) params);
        state = STATE_START;
        i = 0;
        break;
      case STATE_POPUP_KEYS:
        if (len_params < 4 || memcmpi(params, (uchar *) "keys", 4) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        params += 4;
        strtrunc(params);
        len_params = strlen((char *) params);
        keyname_start[keyname_index++] = key_name;
        for (j = 0; j < len_params; j++) {
          if (*(params + j) == ' ') {
            break;
          }
          if (*(params + j) == ',') {
            if (keyname_index >= 20) {
              display_error(2, (uchar *) "Maximum of 20 KEYS allowed.", FALSE);
              rc = RC_INVALID_OPERAND;
              break;
            }
            keyname_start[keyname_index++] = key_name + j + 1;
            key_name[j] = '\0';
          } else {
            key_name[j] = *(params + j);
          }
        }
        if (rc == RC_INVALID_OPERAND) {
          break;
        }
        key_name[j] = '\0';
        params += j;
        /*
         * we have the list of keynames, now split them up into individual keys
         */
        for (j = 0; j < keyname_index; j++) {
          popup_escape_keys[j] = find_key_name((uchar *) keyname_start[j]);
          if (popup_escape_keys[j] == -1) {
            display_error(13, (uchar *) keyname_start[j], FALSE);
            rc = RC_INVALID_OPERAND;
            break;
          }
        }
        if (rc == RC_INVALID_OPERAND) {
          break;
        }
        strtrunc(params);
        len_params = strlen((char *) params);
        state = STATE_START;
        i = 0;
        break;
      case STATE_POPUP_LOCATION_CENTRE:
        if (len_params < 6 || (memcmpi(params, (uchar *) "centre", 6) != 0 && memcmpi(params, (uchar *) "center", 6) != 0)) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        location = 'C';
        other_options++;
        params += 6;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_LOCATION_TEXT:
        if (len_params < 4 || memcmpi(params, (uchar *) "text", 4) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        location = 'T';
        other_options++;
        params += 4;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_LOCATION_ABOVE:
        if (len_params < 4 || memcmpi(params, (uchar *) "above", 5) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        location = 'A';
        other_options++;
        params += 5;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_LOCATION_BELOW:
        if (len_params < 4 || memcmpi(params, (uchar *) "below", 5) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        location = 'B';
        other_options++;
        params += 5;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_LOCATION_MOUSE:
        if (len_params < 5 || memcmpi(params, (uchar *) "mouse", 5) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        location = 'M';
        other_options++;
        params += 5;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_INITIAL:
        if (len_params < 7 || memcmpi(params, (uchar *) "initial", 7) != 0) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        params += 7;
        strtrunc(params);
        len_params = strlen((char *) params);
        for (j = 0; j < len_params; j++) {
          if (*(params + j) == ' ') {
            break;
          }
          str_initial[j] = *(params + j);
        }
        str_initial[j] = '\0';
        if (!valid_positive_integer((uchar *) str_initial)) {
          display_error(4, (uchar *) str_initial, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        initial = atoi((char *) str_initial);
        if (initial == 0) {
          display_error(5, (uchar *) str_initial, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        other_options++;
        params += j;
        strtrunc(params);
        len_params = strlen((char *) params);
        i = 0;
        state = STATE_START;
        break;
      case STATE_POPUP_CONTENT:
        len_params = strlen((char *) params);
        /*
         * Check that we have at least one menu item...
         */
        if (len_params == 0) {
          invalid_item = TRUE;
        } else {
          if (len_params == 1 && (*(params) == delimiter)) {
            invalid_item = TRUE;
          }
        }
        if (invalid_item) {
          display_error(1, params, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        }
        /*
         * Check for trailing delimiter...
         */
        if (*(params + len_params - 1) == delimiter) {
          trailing_delimiter = TRUE;
        } else {
          trailing_delimiter = FALSE;
        }
        /*
         * Allocate the first 100 pointers...
         */
        args_allocated = 100;
        args = (uchar **) malloc(sizeof(uchar *) * args_allocated);
        if (args == NULL) {
          display_error(30, (uchar *) "", FALSE);
          rc = RC_OUT_OF_MEMORY;
          break;
        }
        /*
         * Replace all delimiters with nul character to give us separate strings.
         */
        args[0] = params;
        for (num_items = 1, j = 0; j < len_params; j++) {
          if (*(params + j) == delimiter) {
            *(params + j) = '\0';
            args[num_items++] = params + j + 1;
            if (num_items > args_allocated) {
              args_allocated += 100;
              args = (uchar **) realloc(args, sizeof(uchar *) * args_allocated);
              if (args == NULL) {
                display_error(30, (uchar *) "", FALSE);
                rc = RC_OUT_OF_MEMORY;
                break;
              }
            }
          }
        }
        if (rc != RC_OK) {
          break;
        }
        /*
         * Allow for no trailing delimiter...
         */
        if (trailing_delimiter) {
          num_items--;
        }
        /*
         * Calculate number of lines to display...
         */
        height = num_items + 2; /* extra 2 to allow for border */
        pad_height = height;
        if (height > terminal_lines) {
          height = terminal_lines - 2;
        }
        /*
         * Work out the maximum width of the menu items...
         */
        for (j = 0; j < num_items; j++) {
          len = strlen((char *) args[j]);
          if (len > width) {
            width = len;
          }
        }
        width += 4;
        pad_width = width;
        if (width > terminal_cols) {
          width = terminal_cols - 2;
        }
        other_options++;
        len_params = 0;
        break;
      default:
        display_error(1, params, FALSE);
        rc = RC_INVALID_OPERAND;
        break;
    }
    if (rc != RC_OK) {
      break;
    }
  }
  /*
   * If we have an error from the above, or if ESCAPE is the only option specified.
   */
  if (rc == RC_OK && other_options != 0) {
    /*
     * If the location specified is MOUSE and this wasn't called from a mouse key,
     * or no mouse support is available, return an error.
     */
    switch (location) {
      case 'M':
        get_saved_mouse_pos(&y, &x);
        break;
      case 'C':
        x = (terminal_cols - width) / 2;
        y = (terminal_lines - height) / 2;
        break;
      case 'T':
        /*
         * Get the current window text position and get the global offset from the window start coordinates.
         */
        getyx(CURRENT_WINDOW, y, x);
        getbegyx(CURRENT_WINDOW, begy, begx);
        y = (y + begy);
        x = (x + begx);
        break;
      case 'B':
        /*
         * Get the current window text position and get the global offset from the window start coordinates.
         */
        getyx(CURRENT_WINDOW, y, x);
        getbegyx(CURRENT_WINDOW, begy, begx);
        y = 1 + (y + begy);
        x = (x + begx);
        if (height + y > terminal_lines) {
          height = terminal_lines - y - 1;
        }
        if (height < 3) {
          display_error(0, (uchar *) "No room to display POPUP window", FALSE);
          return (RC_INVALID_OPERAND);
        }
        break;
      case 'A':
        /*
         * Get the current window text position and get the global offset from the window start coordinates.
         */
        getyx(CURRENT_WINDOW, y, x);
        getbegyx(CURRENT_WINDOW, begy, begx);
        y = (y + begy) - 1;
        x = (x + begx);
        if (height > y) {
          height = y;
        }
        y = y - height + 1;
        if (height < 3) {
          display_error(0, (uchar *) "No room to display POPUP window", FALSE);
          return (RC_INVALID_OPERAND);
        }
        break;
      default:
        break;
    }
    /*
     * If the location specified is MOUSE and this wasn't called from a mouse key,
     * or no mouse support is available, return an error.
     */
    if (x == -1) {
      free(args);
      display_error(0, (uchar *) "No mouse support", FALSE);
      return (RC_INVALID_ENVIRON);
    }
    /*
     * Ensure that any supplied initial value fits within the number of lines in the content
     */
    if (initial > num_items) {
      free(args);
      display_error(6, (uchar *) str_initial, FALSE);
      return (RC_INVALID_OPERAND);
    }
    /*
     * Make the window fit by adjusting the top left corner if it would run over the right edge.
     */
    if (x + width > terminal_cols) {
      x = terminal_cols - width;
    }
    if (y + height > terminal_lines) {
      y = terminal_lines - height;
    }
    rc = execute_popup(y, x, height, width, pad_height, pad_width, initial, num_items, args, keyname_index);
    /*
     * Free up the memory used by args
     */
    free(args);
  }
  return (rc);
}

short execute_popup(int y, int x, int height, int width, int pad_height, int pad_width, int initial, int num_args, uchar **args, int keyname_index) {
  short rc = RC_OK;
  int key, i, j, screenx = x, screeny = y;
  WINDOW *dialog_win = NULL;
  WINDOW *pad;
  short item_selected = -1, highlighted_line;
  char buf[20];                 /* enough for a number */
  bool time_to_quit;
  int x_offset = 0, y_offset = 0;
  int x_overlap, y_overlap;
  int offset_lines = 0, scroll_lines;
  int escape_key_index = 0;

  /*
   * Determine where to display the initial line
   */
  y_overlap = pad_height - height;
  x_overlap = pad_width - width;
  if (initial == 0) {
    highlighted_line = initial;
    if (args[highlighted_line][0] == '-') {
      offset_lines = get_non_separator_line(highlighted_line, num_args, args, 1);
    }
    highlighted_line += offset_lines;
  } else {
    highlighted_line = initial - 1;
    if (args[highlighted_line][0] == '-') {
      display_error(0, (uchar *) "Specified initial line is a separator", FALSE);
      return (RC_INVALID_OPERAND);
    }
    /*
     * If the initial line is not on the first screen, work out how to centre it
     */
    if (y_overlap) {
      if (highlighted_line + 2 >= height) {
        y_offset = highlighted_line - height / 2;
        if (y_offset < 0) {
          y_offset = 0;
        } else if (y_offset + height > num_args) {
          y_offset = num_args - height;
        }
      }
    }
  }
  /*
   * Create the popup menu window
   */
  dialog_win = newwin(height, width, y, x);
  if (dialog_win == NULL) {
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  keypad(dialog_win, TRUE);
  pad = newpad(pad_height, pad_width);
  if (pad == NULL) {
    delwin(dialog_win);
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  wbkgd(pad, set_colour(CURRENT_FILE->attr + ATTR_POPUP));
  draw_cursor(FALSE);
  {
    wattrset(dialog_win, set_colour(CURRENT_FILE->attr + ATTR_POP_BORDER));
    box(dialog_win, 0, 0);
    if (height != pad_height) {
      wmove(dialog_win, 0, width - 1);
      waddch(dialog_win, ' ');
      wmove(dialog_win, 1, width - 1);
      waddch(dialog_win, A_ALTCHARSET | ACS_UARROW);
      wmove(dialog_win, height - 2, width - 1);
      waddch(dialog_win, A_ALTCHARSET | ACS_DARROW);
    }
    if (width != pad_width) {
      wmove(dialog_win, height - 1, 0);
      waddch(dialog_win, ' ');
      wmove(dialog_win, height - 1, 1);
      waddch(dialog_win, A_ALTCHARSET | ACS_LARROW);
      wmove(dialog_win, height - 1, width - 2);
      waddch(dialog_win, A_ALTCHARSET | ACS_RARROW);
      wmove(dialog_win, height - 1, width - 1);
      waddch(dialog_win, ' ');
    }
    wnoutrefresh(dialog_win);
  }
  for (;;) {
    for (i = 0; i < num_args; i++) {
      if ((args[i][0]) == '-') {
        wattrset(pad, set_colour(CURRENT_FILE->attr + ATTR_POP_DIVIDER));
        wmove(pad, i, 0);
        whline(pad, 0, pad_width - 2);
      } else {
        if (i == highlighted_line) {
          wattrset(pad, set_colour(CURRENT_FILE->attr + ATTR_POP_CURLINE));
        } else {
          wattrset(pad, set_colour(CURRENT_FILE->attr + ATTR_POPUP));
        }
        wmove(pad, i, 1);
        waddstr(pad, (char *) args[i]);
        wmove(pad, i, 1 + strlen((char *) args[i]));
        for (j = 1 + strlen((char *) args[i]); j < pad_width - 3; j++) {
          waddch(pad, ' ');
        }
      }
    }
    touchwin(pad);
    prefresh(pad, y_offset, x_offset, screeny + 1, screenx + 1, screeny + height - 2, screenx + width - 2);
    key = wgetch(stdscr);
    if (key == KEY_MOUSE) {
      int b, ba, bm, y, x;

      if (get_mouse_info(&b, &ba, &bm) != RC_OK) {
        return (RC_OK);
      }
      if (b != 1 || ba == BUTTON_PRESSED) {
        continue;
      }
      wmouse_position(dialog_win, &y, &x);
      if (y == -1 && x == -1) {
        /*
         * Button 1 clicked or released outside of window; exit
         */
        break;
      }
      /*
       * Check that the mouse is clicked on a valid item
       */
      if (y > 0 && y + 1 < height && x > 0 && x + 1 < width) {
        i = y - 1;
      } else {
        /*
         * Clicked on border
         */
        continue;
      }
      if ((args[i][0]) == '-') {
        /*
         * Clicked on a line
         */
        continue;
      }
      /*
       * Got a valid line. Check if its a click or press
       */
      if (ba == BUTTON_CLICKED || ba == BUTTON_RELEASED) {
        /*
         * Got a valid line. Redisplay it in highlighted mode.
         */
        item_selected = i;
        wattrset(pad, set_colour(CURRENT_FILE->attr + ATTR_POPUP));
        wmove(pad, i, 1);
        waddstr(pad, (char *) args[i]);
        touchwin(pad);
        break;
      }
      continue;
    } else {
      time_to_quit = FALSE;
      switch (key) {
        case 9:
        case KEY_DOWN:
          /* increment highlighted line and check we haven't gone past the end of the list */
          if (++highlighted_line >= num_args) {
            highlighted_line = num_args - 1;
            break;
          }
          offset_lines = 1;
          scroll_lines = 0;
          /* if the new highlighted line is a separator, find the next non-separator */
          if (args[highlighted_line][0] == '-') {
            offset_lines = get_non_separator_line(highlighted_line, num_args, args, 1);
            if (offset_lines == 0) {
              /*
               * we couldn't find a non-separator line before the end of the list,
               * so leave the current line as the highlighted line
               */
              highlighted_line--;
            } else {
              /*
               * we found a non-separator line before the end of the list,
               * so advance the highlighted line by offset_lines and scroll the same number of lines
               */
              highlighted_line += offset_lines;
              scroll_lines = offset_lines;
            }
          }
          if (y_overlap) {
            /*
             * The number of lines in the list is > the window size...
             */
            if (highlighted_line + 2 >= y_offset + height && y_offset < y_overlap) {
              /*
               * The new highlighted line is now below the last displayed line.
               * Change the offset into the list by the number of rows we scrolled in the above code.
               */
              y_offset += 1 + scroll_lines;
            }
          }
          break;
        case KEY_NPAGE:
          /* advance a page full if we have plenty of lines remaining */
          highlighted_line += (height - 2);
          y_offset += (height - 2);
          if (highlighted_line + (height - 2) >= num_args) {
            highlighted_line = num_args - 1;
            y_offset = num_args - (height - 2);
          }
          offset_lines = 1;
          scroll_lines = 0;
          /* if the new highlighted line is a separator, find the next non-separator */
          if (args[highlighted_line][0] == '-') {
            offset_lines = get_non_separator_line(highlighted_line, num_args, args, 1);
            if (offset_lines == 0) {
              /*
               * we couldn't find a non-separator line before the end of the list,
               * so leave the current line as the highlighted line.
               * This won't work need something other than --
               */
              highlighted_line--;
            } else {
              /*
               * we found a non-separator line before the end of the list,
               * so advance the highlighted line by offset_lines and scroll the same number of lines
               */
              highlighted_line += offset_lines;
              scroll_lines = offset_lines;
            }
          }
          break;
        case KEY_UP:
          if (--highlighted_line < 0) {
            highlighted_line = 0;
            break;
          }
          offset_lines = 1;
          scroll_lines = 0;
          if (args[highlighted_line][0] == '-') {
            offset_lines = get_non_separator_line(highlighted_line, num_args, args, -1);
            if (offset_lines == 0) {
              highlighted_line++;
            } else {
              highlighted_line -= offset_lines;
              scroll_lines = offset_lines;
            }
          }
          if (y_overlap) {
            if (highlighted_line + 1 <= y_offset && y_offset) {
              y_offset -= 1 + scroll_lines;
            }
          }
          break;
        case KEY_PPAGE:
          /* retreat a page full if we have plenty of lines remaining */
          highlighted_line -= (height - 2);
          y_offset -= (height - 2);
          if (highlighted_line < 0) {
            highlighted_line = 0;
            y_offset = 0;
          }
          offset_lines = 1;
          scroll_lines = 0;
          /* if the new highlighted line is a separator, find the next non-separator */
          if (args[highlighted_line][0] == '-') {
            offset_lines = get_non_separator_line(highlighted_line, num_args, args, 1);
            if (offset_lines == 0) {
              /*
               * we couldn't find a non-separator line before the end of the list,
               * so leave the current line as the highlighted line.
               * This won't work need something other than --
               */
              highlighted_line--;
            } else {
              /*
               * we found a non-separator line before the end of the list,
               * so advance the highlighted line by offset_lines and scroll the same number of lines
               */
              highlighted_line += offset_lines;
              scroll_lines = offset_lines;
            }
          }
          if (y_offset < 0) {
            y_offset = 0;
          }
          break;
        case KEY_RIGHT:
          if (x_overlap) {
            if (x_offset < x_overlap) {
              x_offset++;
            }
          }
          break;
        case KEY_LEFT:
          if (x_overlap) {
            if (x_offset) {
              x_offset--;
            }
          }
          break;
        case 10:
        case 13:
          item_selected = highlighted_line;
          time_to_quit = TRUE;
          break;
        case 'q':
          time_to_quit = TRUE;
          break;
        default:
          if (key == popup_escape_key) {
            time_to_quit = TRUE;
          } else {
            /*
             * What about other keys?
             */
            for (j = 0; j < keyname_index; j++) {
              if (key == popup_escape_keys[j]) {
                item_selected = highlighted_line;
                time_to_quit = TRUE;
                escape_key_index = j + 1;
                break;
              }
            }
          }
          break;
      }
      if (time_to_quit) {
        break;
      }
    }
  }
  delwin(pad);
  delwin(dialog_win);
  draw_cursor(TRUE);
  /*
   * Set the Rexx variables POPUP.0, POPUP.1 and POPUP.2 depending on whether an item was selected and which item.
   */
  if (item_selected == -1) {
    set_rexx_variable((uchar *) "POPUP", (uchar *) "", 0, 1);
    set_rexx_variable((uchar *) "POPUP", (uchar *) "0", 1, 2);
  } else {
    set_rexx_variable((uchar *) "POPUP", args[item_selected], strlen((char *) args[item_selected]), 1);
    i = sprintf(buf, "%d", item_selected + 1);
    set_rexx_variable((uchar *) "POPUP", (uchar *) buf, i, 2);
  }
  i = sprintf(buf, "%d", highlighted_line + 1);
  set_rexx_variable((uchar *) "POPUP", (uchar *) buf, i, 3);
  /*
   * Set the index to the escape key
   */
  i = sprintf(buf, "%d", escape_key_index);
  set_rexx_variable((uchar *) "POPUP", (uchar *) buf, i, 4);
  /*
   * Set the 0th value to the number of values in popup. array
   */
  set_rexx_variable((uchar *) "POPUP", (uchar *) "4", 1, 0);
  THERefresh((uchar *) "");
  restore_THE();
  return (rc);
}

short execute_preserve(VIEW_DETAILS *src_vd, PRESERVED_VIEW_DETAILS **preserved_view_details, FILE_DETAILS *src_fd, PRESERVED_FILE_DETAILS **preserved_file_details) {
  short rc = RC_OK;

  /*
   * If we already have preserved settings, don't allocate more memory, just use what's there...
   */
  if (*preserved_view_details == NULL) {
    /*
     * Allocate memory for preserved VIEW and FILE details
     */
    if ((*preserved_view_details = (PRESERVED_VIEW_DETAILS *) malloc(sizeof(PRESERVED_VIEW_DETAILS))) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    if ((*preserved_file_details = (PRESERVED_FILE_DETAILS *) malloc(sizeof(PRESERVED_FILE_DETAILS))) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      free(*preserved_view_details);
      *preserved_view_details = NULL;
      return (RC_OUT_OF_MEMORY);
    }
    if (((*preserved_file_details)->attr = (COLOUR_ATTR *) malloc(ATTR_MAX * sizeof(COLOUR_ATTR))) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      free(*preserved_view_details);
      *preserved_view_details = NULL;
      free(*preserved_file_details);
      *preserved_file_details = NULL;
      return (RC_OUT_OF_MEMORY);
    }
    if (((*preserved_file_details)->ecolour = (COLOUR_ATTR *) malloc(ECOLOUR_MAX * sizeof(COLOUR_ATTR))) == NULL) {
      display_error(30, (uchar *) "", FALSE);
      free((*preserved_file_details)->attr);
      (*preserved_file_details)->attr = NULL;
      free(*preserved_view_details);
      *preserved_view_details = NULL;
      free(*preserved_file_details);
      *preserved_file_details = NULL;
      return (RC_OUT_OF_MEMORY);
    }
  }
  /*
   * Save the VIEW details...
   */
  (*preserved_view_details)->arbchar_status = src_vd->arbchar_status;
  (*preserved_view_details)->arbchar_single = src_vd->arbchar_single;
  (*preserved_view_details)->arbchar_multiple = src_vd->arbchar_multiple;
  (*preserved_view_details)->arrow_on = src_vd->arrow_on;
  (*preserved_view_details)->case_enter = src_vd->case_enter;
  (*preserved_view_details)->case_enter_cmdline = src_vd->case_enter_cmdline;
  (*preserved_view_details)->case_enter_prefix = src_vd->case_enter_prefix;
  (*preserved_view_details)->case_locate = src_vd->case_locate;
  (*preserved_view_details)->case_change = src_vd->case_change;
  (*preserved_view_details)->case_sort = src_vd->case_sort;
  (*preserved_view_details)->cmd_line = src_vd->cmd_line;
  (*preserved_view_details)->current_row = src_vd->current_row;
  (*preserved_view_details)->current_base = src_vd->current_base;
  (*preserved_view_details)->current_off = src_vd->current_off;
  (*preserved_view_details)->display_low = src_vd->display_low;
  (*preserved_view_details)->display_high = src_vd->display_high;
  (*preserved_view_details)->hex = src_vd->hex;
  (*preserved_view_details)->hexshow_on = src_vd->hexshow_on;
  (*preserved_view_details)->hexshow_base = src_vd->hexshow_base;
  (*preserved_view_details)->hexshow_off = src_vd->hexshow_off;
  (*preserved_view_details)->highlight = src_vd->highlight;
  (*preserved_view_details)->highlight_high = src_vd->highlight_high;
  (*preserved_view_details)->highlight_low = src_vd->highlight_low;
  (*preserved_view_details)->id_line = src_vd->id_line;
  (*preserved_view_details)->imp_macro = src_vd->imp_macro;
  (*preserved_view_details)->imp_os = src_vd->imp_os;
  (*preserved_view_details)->inputmode = src_vd->inputmode;
  (*preserved_view_details)->linend_status = src_vd->linend_status;
  (*preserved_view_details)->linend_value = src_vd->linend_value;
  (*preserved_view_details)->macro = src_vd->macro;
  (*preserved_view_details)->margin_left = src_vd->margin_left;
  (*preserved_view_details)->margin_right = src_vd->margin_right;
  (*preserved_view_details)->margin_indent = src_vd->margin_indent;
  (*preserved_view_details)->margin_indent_offset_status = src_vd->margin_indent_offset_status;
  (*preserved_view_details)->msgline_base = src_vd->msgline_base;
  (*preserved_view_details)->msgline_off = src_vd->msgline_off;
  (*preserved_view_details)->msgline_rows = src_vd->msgline_rows;
  (*preserved_view_details)->msgmode_status = src_vd->msgmode_status;
  (*preserved_view_details)->newline_aligned = src_vd->newline_aligned;
  (*preserved_view_details)->number = src_vd->number;
  (*preserved_view_details)->position_status = src_vd->position_status;
  (*preserved_view_details)->prefix = src_vd->prefix;
  (*preserved_view_details)->prefix_width = src_vd->prefix_width;
  (*preserved_view_details)->prefix_gap = src_vd->prefix_gap;
  (*preserved_view_details)->scale_on = src_vd->scale_on;
  (*preserved_view_details)->scale_base = src_vd->scale_base;
  (*preserved_view_details)->scale_off = src_vd->scale_off;
  (*preserved_view_details)->scope_all = src_vd->scope_all;
  (*preserved_view_details)->shadow = src_vd->shadow;
  (*preserved_view_details)->stay = src_vd->stay;
  (*preserved_view_details)->synonym = src_vd->synonym;
  (*preserved_view_details)->tab_on = src_vd->tab_on;
  (*preserved_view_details)->tab_base = src_vd->tab_base;
  (*preserved_view_details)->tab_off = src_vd->tab_off;
  (*preserved_view_details)->tabsinc = src_vd->tabsinc;
  (*preserved_view_details)->numtabs = src_vd->numtabs;
  (*preserved_view_details)->tofeof = src_vd->tofeof;
  (*preserved_view_details)->verify_col = src_vd->verify_col;
  (*preserved_view_details)->verify_start = src_vd->verify_start;
  (*preserved_view_details)->verify_end = src_vd->verify_end;
  (*preserved_view_details)->word = src_vd->word;
  (*preserved_view_details)->wordwrap = src_vd->wordwrap;
  (*preserved_view_details)->wrap = src_vd->wrap;
  (*preserved_view_details)->zone_start = src_vd->zone_start;
  (*preserved_view_details)->zone_end = src_vd->zone_end;
  memcpy((*preserved_view_details)->tabs, src_vd->tabs, sizeof(src_vd->tabs));
  (*preserved_view_details)->thighlight_on = src_vd->thighlight_on;
  (*preserved_view_details)->thighlight_active = src_vd->thighlight_active;
  (*preserved_view_details)->thighlight_target = src_vd->thighlight_target;
  /*
   * Save the FILE details...
   */
  (*preserved_file_details)->autosave = src_fd->autosave;
  (*preserved_file_details)->backup = src_fd->backup;
  (*preserved_file_details)->eolout = src_fd->eolout;
  (*preserved_file_details)->tabsout_on = src_fd->tabsout_on;
  (*preserved_file_details)->tabsout_num = src_fd->tabsout_num;
  (*preserved_file_details)->trailing = src_fd->trailing;
  memcpy((*preserved_file_details)->attr, src_fd->attr, ATTR_MAX * sizeof(COLOUR_ATTR));
  memcpy((*preserved_file_details)->ecolour, src_fd->ecolour, ECOLOUR_MAX * sizeof(COLOUR_ATTR));
  (*preserved_file_details)->colouring = src_fd->colouring;
  (*preserved_file_details)->autocolour = src_fd->autocolour;
  (*preserved_file_details)->parser = src_fd->parser;
  return (rc);
}

short execute_restore(VIEW_DETAILS *dst_vd, PRESERVED_VIEW_DETAILS **preserved_view_details, FILE_DETAILS *dst_fd, PRESERVED_FILE_DETAILS **preserved_file_details, bool rebuild_screen) {
  short rc = RC_OK;

  /*
   * If we don't have any preserved settings, return an error.
   */
  if (*preserved_view_details == NULL) {
    display_error(51, (uchar *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Restore the VIEW details...
   */
  dst_vd->arbchar_status = (*preserved_view_details)->arbchar_status;
  dst_vd->arbchar_single = (*preserved_view_details)->arbchar_single;
  dst_vd->arbchar_multiple = (*preserved_view_details)->arbchar_multiple;
  dst_vd->arrow_on = (*preserved_view_details)->arrow_on;
  dst_vd->case_enter = (*preserved_view_details)->case_enter;
  dst_vd->case_enter_cmdline = (*preserved_view_details)->case_enter_cmdline;
  dst_vd->case_enter_prefix = (*preserved_view_details)->case_enter_prefix;
  dst_vd->case_locate = (*preserved_view_details)->case_locate;
  dst_vd->case_change = (*preserved_view_details)->case_change;
  dst_vd->case_sort = (*preserved_view_details)->case_sort;
  dst_vd->cmd_line = (*preserved_view_details)->cmd_line;
  dst_vd->current_row = (*preserved_view_details)->current_row;
  dst_vd->current_base = (*preserved_view_details)->current_base;
  dst_vd->current_off = (*preserved_view_details)->current_off;
  dst_vd->display_low = (*preserved_view_details)->display_low;
  dst_vd->display_high = (*preserved_view_details)->display_high;
  dst_vd->hex = (*preserved_view_details)->hex;
  dst_vd->hexshow_on = (*preserved_view_details)->hexshow_on;
  dst_vd->hexshow_base = (*preserved_view_details)->hexshow_base;
  dst_vd->hexshow_off = (*preserved_view_details)->hexshow_off;
  dst_vd->highlight = (*preserved_view_details)->highlight;
  dst_vd->highlight_high = (*preserved_view_details)->highlight_high;
  dst_vd->highlight_low = (*preserved_view_details)->highlight_low;
  dst_vd->id_line = (*preserved_view_details)->id_line;
  dst_vd->imp_macro = (*preserved_view_details)->imp_macro;
  dst_vd->imp_os = (*preserved_view_details)->imp_os;
  dst_vd->inputmode = (*preserved_view_details)->inputmode;
  dst_vd->linend_status = (*preserved_view_details)->linend_status;
  dst_vd->linend_value = (*preserved_view_details)->linend_value;
  dst_vd->macro = (*preserved_view_details)->macro;
  dst_vd->margin_left = (*preserved_view_details)->margin_left;
  dst_vd->margin_right = (*preserved_view_details)->margin_right;
  dst_vd->margin_indent = (*preserved_view_details)->margin_indent;
  dst_vd->margin_indent_offset_status = (*preserved_view_details)->margin_indent_offset_status;
  dst_vd->msgline_base = (*preserved_view_details)->msgline_base;
  dst_vd->msgline_off = (*preserved_view_details)->msgline_off;
  dst_vd->msgline_rows = (*preserved_view_details)->msgline_rows;
  dst_vd->msgmode_status = (*preserved_view_details)->msgmode_status;
  dst_vd->newline_aligned = (*preserved_view_details)->newline_aligned;
  dst_vd->number = (*preserved_view_details)->number;
  dst_vd->position_status = (*preserved_view_details)->position_status;
  dst_vd->prefix = (*preserved_view_details)->prefix;
  dst_vd->prefix_width = (*preserved_view_details)->prefix_width;
  dst_vd->prefix_gap = (*preserved_view_details)->prefix_gap;
  dst_vd->scale_on = (*preserved_view_details)->scale_on;
  dst_vd->scale_base = (*preserved_view_details)->scale_base;
  dst_vd->scale_off = (*preserved_view_details)->scale_off;
  dst_vd->scope_all = (*preserved_view_details)->scope_all;
  dst_vd->shadow = (*preserved_view_details)->shadow;
  dst_vd->stay = (*preserved_view_details)->stay;
  dst_vd->synonym = (*preserved_view_details)->synonym;
  dst_vd->tab_on = (*preserved_view_details)->tab_on;
  dst_vd->tab_base = (*preserved_view_details)->tab_base;
  dst_vd->tab_off = (*preserved_view_details)->tab_off;
  dst_vd->tabsinc = (*preserved_view_details)->tabsinc;
  dst_vd->numtabs = (*preserved_view_details)->numtabs;
  dst_vd->tofeof = (*preserved_view_details)->tofeof;
  dst_vd->verify_col = (*preserved_view_details)->verify_col;
  dst_vd->verify_start = (*preserved_view_details)->verify_start;
  dst_vd->verify_end = (*preserved_view_details)->verify_end;
  dst_vd->word = (*preserved_view_details)->word;
  dst_vd->wordwrap = (*preserved_view_details)->wordwrap;
  dst_vd->wrap = (*preserved_view_details)->wrap;
  dst_vd->zone_start = (*preserved_view_details)->zone_start;
  dst_vd->zone_end = (*preserved_view_details)->zone_end;
  memcpy(dst_vd->tabs, (*preserved_view_details)->tabs, sizeof((*preserved_view_details)->tabs));       /* FGC */
  dst_vd->thighlight_on = (*preserved_view_details)->thighlight_on;
  dst_vd->thighlight_active = (*preserved_view_details)->thighlight_active;
  dst_vd->thighlight_target = (*preserved_view_details)->thighlight_target;
  /*
   * Restore the FILE details...
   */
  dst_fd->autosave = (*preserved_file_details)->autosave;
  dst_fd->backup = (*preserved_file_details)->backup;
  dst_fd->eolout = (*preserved_file_details)->eolout;
  dst_fd->tabsout_on = (*preserved_file_details)->tabsout_on;
  dst_fd->tabsout_num = (*preserved_file_details)->tabsout_num;
  dst_fd->trailing = (*preserved_file_details)->trailing;
  memcpy(dst_fd->attr, (*preserved_file_details)->attr, ATTR_MAX * sizeof(COLOUR_ATTR));
  memcpy(dst_fd->ecolour, (*preserved_file_details)->ecolour, ECOLOUR_MAX * sizeof(COLOUR_ATTR));
  dst_fd->colouring = (*preserved_file_details)->colouring;
  dst_fd->autocolour = (*preserved_file_details)->autocolour;
  dst_fd->parser = (*preserved_file_details)->parser;
  /*
   * Free any memory for preserved VIEW and FILE details
   */
  free((*preserved_view_details));
  (*preserved_view_details) = NULL;
  free((*preserved_file_details)->attr);
  (*preserved_file_details)->attr = NULL;
  free((*preserved_file_details)->ecolour);
  (*preserved_file_details)->ecolour = NULL;
  free(*preserved_file_details);
  (*preserved_file_details) = NULL;
  /*
   * Now that all the settings are back in place apply any screen changes...
   */
  if (rebuild_screen) {
    set_screen_defaults();
    if (set_up_windows(current_screen) != RC_OK) {
      return (RC_OK);
    }
    build_screen(current_screen);
    display_screen(current_screen);
    display_cmdline(current_screen, CURRENT_VIEW);
  }
  return (rc);
}

