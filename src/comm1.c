/* COMM1.C - Commands A-C                                              */
/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2013 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling, mark@rexx.org  http://www.rexx.org/
 */

#include <the.h>
#include <proto.h>

/*#define DEBUG 1*/

short Add(char_t * params) {
#define ADD_PARAMS  1
  char_t *word[ADD_PARAMS + 1];
  char_t strip[ADD_PARAMS];
  unsigned short num_params = 0;
  line_t num_lines = 0L;

  /*
   * Validate the parameters that have been supplied. The one and only
   * parameter should be a positive integer greater than zero.
   * If no parameter is supplied, 1 is assumed.
   */
  strip[0] = STRIP_BOTH;
  num_params = param_split(params, word, ADD_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    num_params = 1;
    word[0] = (char_t *) "1";
  }
  if (num_params != 1) {
    display_error(1, word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (!valid_positive_integer(word[0])) {
    display_error(4, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  num_lines = atol((char *) word[0]);
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  insert_new_line(current_screen, CURRENT_VIEW, (char_t *) "", 0, num_lines, get_true_line(TRUE), FALSE, FALSE, TRUE, CURRENT_VIEW->display_low, TRUE, FALSE);
  if (curses_started && CURRENT_VIEW->current_window == WINDOW_COMMAND)
    THEcursor_home(current_screen, CURRENT_VIEW, TRUE);
  return (RC_OK);
}
short Alert(char_t * params) {
  short rc = RC_OK;

  /*
   * If we have a beep() functiond and its ON, ring it..
   */
  if (BEEPx)
    beep();
  rc = prepare_dialog(params, TRUE, (char_t *) "ALERT");
  return (rc);
}
short All(char_t * params) {
  short rc = RC_OK;
  LINE *curr = NULL;
  bool target_found = FALSE;
  short status = RC_OK;
  long target_type = TARGET_NORMAL | TARGET_REGEXP;
  TARGET target;
  line_t line_number = 0L;
  unsigned short x = 0, y = 0;
  bool save_scope = FALSE;
  line_t num_lines = 0L;

  if (strlen((char *) params) == 0) {
    if (CURRENT_FILE->number_lines == 0L) {
      return (rc);
    }
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    curr = CURRENT_FILE->first_line->next;
    while (1) {
      curr->select = 0;
      curr = curr->next;
      if (curr->next == NULL)
        break;
    }
    CURRENT_VIEW->display_low = 0;
    CURRENT_VIEW->display_high = 0;
    build_screen(current_screen);
    display_screen(current_screen);
    return (rc);
  }
  if (CURRENT_FILE->number_lines == 0L) {
    display_error(17, params, FALSE);
    return (RC_TARGET_NOT_FOUND);
  }
  /*
   * Validate the parameters as valid targets...
   */
  initialise_target(&target);
  rc = parse_target(params, get_true_line(TRUE), &target, target_type, TRUE, TRUE, FALSE);
  if (rc != RC_OK) {
    free_target(&target);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Save the select levels for all lines in case no target is found.
   */
  curr = CURRENT_FILE->first_line->next;
  while (1) {
    curr->save_select = curr->select;
    curr = curr->next;
    if (curr->next == NULL)
      break;
  }
  /*
   * Find all lines for the supplied target...
   */
  curr = CURRENT_FILE->first_line;
  status = FALSE;
  save_scope = CURRENT_VIEW->scope_all;
  CURRENT_VIEW->scope_all = TRUE;
  /*
   * Tell the target finding stuff we are the TAG command...
   */
  target.all_tag_command = TRUE;
  for (line_number = 0L; curr->next != NULL; line_number++) {
    status = find_rtarget_target(curr, &target, 0L, line_number, &num_lines);
    if (status == RC_OK) {      /* target found */
      target_found = TRUE;
      curr->select = 1;
    } else if (status == RC_TARGET_NOT_FOUND) { /* target not found */
      curr->select = 0;
    } else                      /* error */
      break;
    curr = curr->next;
  }
  /*
   * If at least one line matches the target, set DISPLAY to 1 1,
   * otherwise reset the select levels as they were before the command.
   */
  if (target_found) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    CURRENT_VIEW->display_low = 1;
    CURRENT_VIEW->display_high = 1;
    CURRENT_VIEW->scope_all = FALSE;
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, CURRENT_FILE->first_line->next, 1L, DIRECTION_FORWARD);
    build_screen(current_screen);
    display_screen(current_screen);
    CURRENT_VIEW->focus_line = calculate_focus_line(CURRENT_VIEW->focus_line, CURRENT_VIEW->current_line);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
      getyx(CURRENT_WINDOW, y, x);
      y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
      wmove(CURRENT_WINDOW, y, x);
    }
    /*
     * If the same file is in the other screen, refresh it
     */
    adjust_other_screen_shadow_lines();
  } else {
    CURRENT_VIEW->scope_all = save_scope;
    curr = CURRENT_FILE->first_line->next;
    while (1) {
      curr->select = curr->save_select;
      curr = curr->next;
      if (curr->next == NULL)
        break;
    }
    if (status == RC_TARGET_NOT_FOUND) {
      display_error(17, params, FALSE);
      rc = RC_TARGET_NOT_FOUND;
    } else
      rc = status;
  }
  free_target(&target);
  return (rc);
}
short Backward(char_t * params) {
#define BAC_PARAMS  2
  char_t *word[BAC_PARAMS + 1];
  char_t strip[BAC_PARAMS];
  unsigned short num_params = 0;
  line_t num_pages = 0L;
  short scroll_by_page = 1;     /* by default we scroll pages */
  short rc = RC_OK;

  /*
   * Validate parameters...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, BAC_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 0:
      num_pages = 1;
      break;
    case 1:
      /*
       * If parameter is '*', set current line equal to "Top of File".
       */
      if (strcmp((char *) word[0], "*") == 0) {
        rc = Top((char_t *) "");
        return (rc);
      }
      /*
       * If parameter is 'HALF', advance half a page
       */
      else if (equal((char_t *) "HALF", word[0], 4)) {
        scroll_by_page = 0;
        num_pages = CURRENT_SCREEN.rows[WINDOW_FILEAREA] / 2;
      }
      /*
       * If the parameter is not a valid positive integer, error.
       */
      else if (!valid_positive_integer(word[0])) {
        display_error(1, (char_t *) word[0], FALSE);
        return (RC_INVALID_OPERAND);
      } else {
        /*
         * Number of screens to scroll is set here.
         */
        num_pages = atol((char *) word[0]);
      }
      break;
    case 2:
      if (equal((char_t *) "Lines", word[1], 1)) {
        scroll_by_page = 0;
        if (!valid_positive_integer(word[0])) {
          display_error(1, (char_t *) word[0], FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        display_error(1, (char_t *) word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      num_pages = atol((char *) word[0]);
      break;
    default:
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
      break;
  }
  /*
   * If the current line is already on "Top of File" or the parameter is
   * 0, go to the bottom of the file.
   */
  if (num_pages == 0 || (CURRENT_TOF && PAGEWRAPx)) {
    rc = Bottom((char_t *) "");
    return (rc);
  }
  /*
   * Scroll the screen num_pages...
   */
  if (scroll_by_page) {
    rc = scroll_page(DIRECTION_BACKWARD, num_pages, FALSE);
  } else {
    rc = advance_current_line(-num_pages);
  }
  return (rc);
}
short Bottom(char_t * params) {
  short rc = RC_OK;
  unsigned short x = 0, y = 0;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (CURRENT_VIEW->scope_all)
    CURRENT_VIEW->current_line = CURRENT_FILE->number_lines;
  else
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, CURRENT_FILE->last_line->prev, CURRENT_FILE->number_lines, DIRECTION_BACKWARD);
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  build_screen(current_screen);
  if (!line_in_view(current_screen, CURRENT_VIEW->focus_line))
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  if (curses_started) {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      getyx(CURRENT_PREV_WINDOW, y, x);
    else
      getyx(CURRENT_WINDOW, y, x);
    display_screen(current_screen);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      wmove(CURRENT_PREV_WINDOW, y, x);
    else
      wmove(CURRENT_WINDOW, y, x);
  }
  return (rc);
}
short Cancel(char_t * params) {
  VIEW_DETAILS *save_current_view = (VIEW_DETAILS *) NULL;
  line_t save_number_of_files = number_of_files;
  VIEW_DETAILS **save_view;
  register int i = 0;
  short to_save = 0, rc;

  /*
   * One optional argument allowed
   */
  if (strcmp("", (char *) params) == 0) {
    to_save = 0;
  } else if (equal((char_t *) "save", params, 4)) {
    to_save = 1;
  } else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);

  if ((save_view = (VIEW_DETAILS **) malloc((save_number_of_files) * sizeof(VIEW_DETAILS *))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Set the numbr of screens to 1; stops crashes when there are multiple
   * screens.  A hack yes.
   */
  THEScreen((char_t *) "1");
  CURRENT_VIEW = vd_first;
  for (i = 0; i < save_number_of_files; i++) {
    save_view[i] = CURRENT_VIEW;
    CURRENT_VIEW = CURRENT_VIEW->next;
  }
  CURRENT_VIEW = vd_first;
  for (i = 0; i < save_number_of_files; i++) {
    CURRENT_VIEW = save_view[i];
    if (CURRENT_FILE->save_alt == 0) {
      free_view_memory(TRUE, FALSE);
    } else if (to_save == 1) {
      if ((rc = save_file(CURRENT_FILE, (char_t *) "", FALSE, CURRENT_FILE->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, FALSE)) != RC_OK) {
        return (rc);
      }
      /*
       * If autosave is on at the time of Filing, remove the .aus file...
       */
      if (CURRENT_FILE->autosave > 0) {
        rc = remove_aus_file(CURRENT_FILE);
      }
      free_view_memory(TRUE, FALSE);
    }
  }
  save_current_view = CURRENT_VIEW = vd_first;
  free(save_view);

  if (save_current_view != (VIEW_DETAILS *) NULL) {
    CURRENT_VIEW = save_current_view;
    CURRENT_SCREEN.screen_view = CURRENT_VIEW;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    build_screen(current_screen);
    display_screen(current_screen);
    if (curses_started) {
      if (CURRENT_WINDOW_PREFIX != NULL)
        touchwin(CURRENT_WINDOW_PREFIX);
      if (CURRENT_WINDOW_COMMAND != NULL)
        touchwin(CURRENT_WINDOW_COMMAND);
      touchwin(CURRENT_WINDOW_FILEAREA);
      touchwin(CURRENT_WINDOW);
    }
  }
  if (number_of_files > 0) {
    sprintf((char *) temp_cmd, "%ld file(s) remain with outstanding changes", number_of_files);
    display_error(0, (char_t *) temp_cmd, TRUE);
  }
  return (QUIT);
}
short Cappend(char_t * params) {
  short rc = RC_OK;

  rc = column_command(params, COLUMN_CAPPEND);
  return (rc);
}
short Ccancel(char_t * params) {
  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW = vd_first;
  while (CURRENT_VIEW != (VIEW_DETAILS *) NULL) {
    free_view_memory(TRUE, FALSE);
  }
  return (QUIT);
}
short Cdelete(char_t * params) {
  short rc = RC_OK;
  short target_type = TARGET_ABSOLUTE | TARGET_RELATIVE | TARGET_STRING | TARGET_BLANK;
  TARGET target;
  length_t start_col = 0, del_start = 0;
  unsigned int y = 0, x = 0;

  /*
   * Validate the cursor position...
   */
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    getyx(CURRENT_WINDOW, y, x);
    rc = processable_line(CURRENT_VIEW, CURRENT_SCREEN.sl[y].line_number, CURRENT_SCREEN.sl[y].current);
    switch (rc) {
      case LINE_SHADOW:
        display_error(87, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
        break;
/*       case LINE_TOF_EOF: MH12 */
      case LINE_TOF:
      case LINE_EOF:
        display_error(36, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
        break;
      default:
        break;
    }
  }
  /*
   * Determine at which column to start the search...
   */
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      start_col = (CURRENT_VIEW->verify_col) + x;
      if (start_col > CURRENT_VIEW->zone_end)
        start_col = min(max_line_length, CURRENT_VIEW->zone_end + 1);
      if (start_col < CURRENT_VIEW->zone_start)
        start_col = max(1, CURRENT_VIEW->zone_start - 1);
      break;
    case WINDOW_PREFIX:
      start_col = max(CURRENT_VIEW->current_column, max(1, CURRENT_VIEW->zone_start));
      break;
    case WINDOW_COMMAND:
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL);
      start_col = CURRENT_VIEW->current_column;
      break;
  }
  /*
   * Validate the parameters as valid targets...
   */
  initialise_target(&target);
  rc = parse_target(params, (line_t) start_col, &target, target_type, TRUE, TRUE, TRUE);
  if (rc != RC_OK) {
    free_target(&target);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Find the valid column target. If found process the command...
   */
  if ((find_column_target(rec, rec_len, &target, start_col, TRUE, TRUE)) == RC_OK) {
    CURRENT_VIEW->current_column = start_col;
    if (target.num_lines < 0) {
      if (start_col > CURRENT_VIEW->zone_end) {
        start_col--;
        target.num_lines++;
      }
      del_start = start_col + target.num_lines;
      (void) memdeln(rec, del_start, rec_len, -target.num_lines);
      rec_len = calculate_rec_len(ADJUST_DELETE, rec, rec_len, del_start, -target.num_lines, CURRENT_FILE->trailing);
    } else {
      if (start_col < CURRENT_VIEW->zone_start) {
        start_col++;
        target.num_lines--;
      }
      del_start = start_col - 1;
      (void) memdeln(rec, del_start, rec_len, target.num_lines);
      rec_len = calculate_rec_len(ADJUST_DELETE, rec, rec_len, del_start, target.num_lines, CURRENT_FILE->trailing);
    }
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL, TRUE);
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    } else {
      rc = THEcursor_column();
    }
    build_screen(current_screen);
    display_screen(current_screen);
  } else {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->current_line, (LINE *) NULL, TRUE);
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
  }
  free_target(&target);
  return (rc);
}
short Cfirst(char_t * params) {
  short rc = RC_OK;
  bool need_to_redisplay = FALSE;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (column_in_view(current_screen, CURRENT_VIEW->current_column - 1))
    need_to_redisplay = TRUE;
  CURRENT_VIEW->current_column = CURRENT_VIEW->zone_start;
  if (column_in_view(current_screen, CURRENT_VIEW->current_column - 1))
    need_to_redisplay = TRUE;
  if (need_to_redisplay)
    display_screen(current_screen);
  return (rc);
}
short Change(char_t * params) {
  short rc = RC_OK;

  rc = execute_change_command(params, FALSE);
  return (rc);
}
short Cinsert(char_t * params) {
  short rc = RC_OK;

  rc = column_command(params, COLUMN_CINSERT);
  return (rc);
}
short Clast(char_t * params) {
  short rc = RC_OK;
  bool need_to_redisplay = FALSE;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (column_in_view(current_screen, CURRENT_VIEW->current_column - 1))
    need_to_redisplay = TRUE;
  CURRENT_VIEW->current_column = CURRENT_VIEW->zone_end;
  if (column_in_view(current_screen, CURRENT_VIEW->current_column - 1))
    need_to_redisplay = TRUE;
  if (need_to_redisplay)
    display_screen(current_screen);
  return (rc);
}
short THEClipboard(char_t * params) {
  short rc = RC_OK;

  /*
   * Validate the argument
   */
  if (equal((char_t *) "copy", params, 4)) {
    /*
     * We must be in the FILEAREA...
     */
    if (CURRENT_WINDOW != WINDOW_FILEAREA && MARK_VIEW != CURRENT_VIEW) {
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    rc = execute_put((char_t *) "block clip:", FALSE);
  } else if (equal((char_t *) "cut", params, 3)) {
    /*
     * We must be in the FILEAREA...
     */
    if (CURRENT_WINDOW != WINDOW_FILEAREA && MARK_VIEW != CURRENT_VIEW) {
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    rc = execute_put((char_t *) "block clip:", TRUE);
  } else if (equal((char_t *) "paste", params, 5)) {
    /*
     * If there is a marked block in the current view, we need to replace
     * the contents of the marked block with the contents of the clipboard
     * The current marked block type determines the how the clipboard
     * contents replace the text.
     * If no marked block, simply insert the text in the clipboard at the
     * cursor position, adding lines as required.
     */
    display_error(82, (char_t *) "CLIP:", FALSE);
    rc = RC_INVALID_ENVIRON;
  } else if (equal((char_t *) "clear", params, 5)) {
    display_error(82, (char_t *) "CLIP:", FALSE);
    rc = RC_INVALID_ENVIRON;
  } else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  return (rc);
}
short Clocate(char_t * params) {
  short rc = RC_OK;
  short target_type = TARGET_ABSOLUTE | TARGET_RELATIVE | TARGET_STRING | TARGET_BLANK;
  TARGET target;
  char_t *line = NULL;
  LINE *curr = NULL;
  length_t len = 0, start_col = 0;
  unsigned int y = 0, x = 0;

  /*
   * If no arguments have been supplied, pass the last clocate command
   * to be executed. If no last clocate command, return error 39.
   */
  if (blank_field(params)) {
    params = lastop[LASTOP_CLOCATE].value;
    if (blank_field(params)) {
      display_error(39, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Determine at which column to start the search...
   */
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      if (compatible_feel == COMPAT_XEDIT) {
        curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->current_line, CURRENT_FILE->number_lines);
        line = curr->line;
        len = curr->length;
        start_col = CURRENT_VIEW->current_column;
      } else {
        line = rec;
        len = rec_len;
        getyx(CURRENT_WINDOW, y, x);
        start_col = (CURRENT_VIEW->verify_col) + x;
        if (start_col > CURRENT_VIEW->zone_end)
          start_col = min(max_line_length, CURRENT_VIEW->zone_end + 1);
        if (start_col < CURRENT_VIEW->zone_start)
          start_col = max(1, CURRENT_VIEW->zone_start - 1);
      }
      break;
    case WINDOW_PREFIX:
      line = rec;
      len = rec_len;
      start_col = max(CURRENT_VIEW->current_column, max(1, CURRENT_VIEW->zone_start));
      break;
    case WINDOW_COMMAND:
      curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->current_line, CURRENT_FILE->number_lines);
      line = curr->line;
      len = curr->length;
      start_col = CURRENT_VIEW->current_column;
      break;
  }
  /*
   * Save the params as lastop for clocate
   */
  if (save_lastop(LASTOP_CLOCATE, params) != RC_OK) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Validate the parameters as valid targets...
   */
  initialise_target(&target);
  rc = parse_target(params, (line_t) start_col, &target, target_type, TRUE, TRUE, TRUE);
  if (rc != RC_OK) {
    free_target(&target);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Find the valid column target. If found process the command...
   */
  if ((find_column_target(line, len, &target, start_col, TRUE, FALSE)) == RC_OK) {
    CURRENT_VIEW->current_column = start_col + target.num_lines;
    if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
      rc = THEcursor_column();
    build_screen(current_screen);
    display_screen(current_screen);
  }
  free_target(&target);
  return (rc);
}
short Cmatch(char_t * params) {
  static char_t *match = (char_t *) "[]{}<>()";
  unsigned short x = 0, y = 0, current_y = 0;
  char_t ch = 0, match_ch = 0;
  length_t i = 0;
  short direction_backward = 0;
  length_t match_col = (-1L), start_col = 0L, focus_column = 0L;
  short matches = 1;
  line_t offset = 0L;
  LINE *curr = NULL;
  line_t focus_line = 0L;
  bool use_current = TRUE;

  if (curses_started)
    getyx(CURRENT_WINDOW, y, x);
  /*
   * Determine variables based on which window we are in
   */
  if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    current_y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    focus_line = CURRENT_VIEW->focus_line;
    focus_column = CURRENT_VIEW->verify_col + x - 1;
    ch = rec[focus_column];
    use_current = FALSE;
  } else {
    y = current_y = CURRENT_VIEW->current_row;
    focus_line = CURRENT_VIEW->current_line;
    focus_column = CURRENT_VIEW->current_column - 1;
    ch = CURRENT_SCREEN.sl[current_y].contents[focus_column];
    use_current = TRUE;
  }
  /*
   * This command cannot be issued on TOF or BOF.
   */
  if (TOF(focus_line)
      || BOF(focus_line)) {
    display_error(66, (char_t *) "", FALSE);
    return (RC_TOF_EOF_REACHED);
  }
  /*
   * This command cannot be entered on a shadow line.
   */
  if (CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW) {
    display_error(87, (char_t *) "", FALSE);
    return (RC_TARGET_NOT_FOUND);
  }
  /*
   * Check if the character under the cursor is a valid match character.
   */
  match_ch = 0;
  for (i = 0; i < strlen((char *) match); i++) {
    if (ch == *(match + i)) {
      direction_backward = (i % 2);
      match_ch = (direction_backward) ? *(match + i - 1) : *(match + i + 1);
      break;
    }
  }
  if (match_ch == 0) {
    display_error(67, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Calculate the actual position of the character in the LINE.
   */
  start_col = focus_column + ((direction_backward) ? (-1) : 1);
  /*
   * Find the focus line linked list entry.
   */
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, focus_line, CURRENT_FILE->number_lines);
  while (curr->next != NULL && curr->prev != NULL) {
    if (direction_backward) {
      for (i = start_col; i > (-1); i--) {
        if (*(curr->line + i) == ch)
          matches++;
        else {
          if (*(curr->line + i) == match_ch) {
            matches--;
          }
        }
        if (matches == 0) {     /* found matching one */
          match_col = i;
          break;
        }
      }
      if (match_col != (-1))
        break;
      curr = curr->prev;
      offset--;
      start_col = curr->length;
    } else {
      for (i = start_col; i < curr->length; i++) {
        if (*(curr->line + i) == ch)
          matches++;
        else {
          if (*(curr->line + i) == match_ch) {
            matches--;
          }
        }
        if (matches == 0) {     /* found matching one */
          match_col = i;
          break;
        }
      }
      if (match_col != (-1))
        break;
      curr = curr->next;
      offset++;
      start_col = 0;
    }
  }
  /*
   * If no match found, return with error.
   */
  if (match_col == (-1)) {      /* no match found */
    display_error(68, (char_t *) "", FALSE);
    return (RC_TARGET_NOT_FOUND);
  }
  /*
   * If we get here, we have found the matching character, so we have to
   * move the cursor to the new column and/or line.
   */
  if (offset == 0L) {
    if (use_current)
      CURRENT_VIEW->current_column = match_col + 1;
    if (match_col >= CURRENT_VIEW->verify_col - 1 && match_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA] + (CURRENT_VIEW->verify_col - 1)) - 1) {
      /*
       * If the new cursor position is in the same panel and on the same line
       * just move the cursor there and get out.
       */
      if (use_current) {
        build_screen(current_screen);
        if (curses_started) {
          display_screen(current_screen);
          wmove(CURRENT_WINDOW, y, x);
        }
      } else {
        if (curses_started) {
          wmove(CURRENT_WINDOW, y, match_col - (CURRENT_VIEW->verify_col - 1));
        }
      }
      return (RC_OK);
    } else {
      x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
      CURRENT_VIEW->verify_col = max(1, match_col - (short) x);
      build_screen(current_screen);
      if (curses_started) {
        display_screen(current_screen);
        wmove(CURRENT_WINDOW, y, (match_col - (CURRENT_VIEW->verify_col - 1)));
      }
      return (RC_OK);
    }
  }
  /*
   * If a match IS found on a different line, further checks are required
   * for SCOPE.
   */
  if (IN_SCOPE(CURRENT_VIEW, curr)) {
    /*
     * Set the cursor position for the matching character.
     */
    if (use_current) {
      CURRENT_VIEW->current_column = match_col + 1;
      CURRENT_VIEW->current_line += offset;
      if (line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
        y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
      } else {
        CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
        y = CURRENT_VIEW->current_row;
      }
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    } else {
      CURRENT_VIEW->focus_line += offset;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      if (line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
        y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
      } else {
        CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
        y = CURRENT_VIEW->current_row;
      }
    }
  } else {
    if (CURRENT_VIEW->scope_all) {
      curr->select = CURRENT_VIEW->display_low;
      if (use_current) {
        CURRENT_VIEW->current_column = match_col + 1;
        CURRENT_VIEW->current_line += offset;
        CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
      } else {
        CURRENT_VIEW->focus_line += offset;
        CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
      }
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      y = CURRENT_VIEW->current_row;
    } else {
      display_error(68, (char_t *) "", FALSE);
      return (RC_TARGET_NOT_FOUND);
    }
  }
  if (match_col >= CURRENT_VIEW->verify_col - 1 && match_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA] + (CURRENT_VIEW->verify_col - 1)) - 1)
    x = match_col - (CURRENT_VIEW->verify_col - 1);
  else {
    x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
    CURRENT_VIEW->verify_col = max(1, match_col - x);
    x = (match_col - (CURRENT_VIEW->verify_col - 1));
  }

  build_screen(current_screen);
  if (curses_started) {
    display_screen(current_screen);
    wmove(CURRENT_WINDOW, y, x);
  }
  return (RC_OK);
}
short Cmsg(char_t * params) {
  memset(cmd_rec, ' ', max_line_length);
  cmd_rec_len = strlen((char *) params);
  memcpy(cmd_rec, params, cmd_rec_len);
  display_cmdline(current_screen, CURRENT_VIEW);
  Sos_endchar((char_t *) "");
  return (RC_OK);
}
short THECommand(char_t * params) {
  short rc = RC_OK;

  rc = command_line(params, COMMAND_ONLY_TRUE);
  return (rc);
}
short Compress(char_t * params) {
  short rc = RC_OK;

  rc = execute_expand_compress(params, FALSE, TRUE, TRUE, TRUE);
  return (rc);
}
short ControlChar(char_t * params) {
  unsigned short y = 0, x = 0;
  int key = 0;

  getyx(CURRENT_WINDOW, y, x);
  /*
   * If in the MAIN window, this command can only be issued on a real
   * line.
   */
  if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
    }
  }
  display_prompt((char_t *) "Press the character you require.");
  wmove(CURRENT_WINDOW, y, x);
  wrefresh(CURRENT_WINDOW);
  for (;;) {
    key = wgetch(CURRENT_WINDOW);
    break;
  }
  clear_msgline(-1);
  if (islower(key))
    key = toupper(key);
  if (key >= (int) 'A'          /* was '@' for ASCII 0, but Text() command fails */
      && key <= (int) '_') {
    return ((RAW_KEY * 2) + (short) key - (short) '@');
  }
  display_error(69, (char_t *) "- must be between 'A' and '_'", FALSE);
  return (RC_INVALID_OPERAND);
}
short Copy(char_t * params) {
  char_t reset_block = SOURCE_UNKNOWN;
  short rc = RC_OK;
  line_t start_line = 0L, end_line = 0L, true_line = 0L, lines_affected = 0L;
  VIEW_DETAILS *source_view = NULL, *dest_view = NULL;
  TARGET target1, target2;
  long target_type1 = TARGET_NORMAL | TARGET_BLOCK_ANY | TARGET_ALL | TARGET_SPARE;
  long target_type2 = TARGET_NORMAL;
  bool lines_based_on_scope = FALSE;

  initialise_target(&target1);
  initialise_target(&target2);
  if ((rc = validate_target(params, &target1, target_type1, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target1);
    return (rc);
  }
  /*
   * If there is no second argument, the only valid target type for the
   * first argument then is BLOCK.
   */
  if (target1.spare == (-1)) {
    if (target1.rt[0].target_type != TARGET_BLOCK_ANY && target1.rt[0].target_type != TARGET_BLOCK_CURRENT) {
      free_target(&target1);
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    } else
      reset_block = SOURCE_BLOCK;
  } else {
    if (equal((char_t *) "reset", strtrunc(target1.rt[target1.spare].string), 5))
      reset_block = SOURCE_BLOCK_RESET;
    else
      reset_block = SOURCE_COMMAND;
  }
  /*
   * Validate the arguments following the target...
   */
  switch (reset_block) {
    case SOURCE_BLOCK:
    case SOURCE_BLOCK_RESET:
      /*
       * For box blocks, call the appropriate function...
       */
      if (MARK_VIEW->mark_type != M_LINE) {
        free_target(&target1);
        box_operations(BOX_C, reset_block, FALSE, ' ');
        return (RC_OK);
      }
      source_view = MARK_VIEW;
      dest_view = CURRENT_VIEW;
      start_line = MARK_VIEW->mark_start_line;
      end_line = MARK_VIEW->mark_end_line;
      true_line = get_true_line(FALSE);
      lines_based_on_scope = FALSE;
      break;
    default:
      if ((rc = validate_target(target1.rt[target1.spare].string, &target2, target_type2, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
        free_target(&target2);
        return (rc);
      }
      source_view = CURRENT_VIEW;
      dest_view = CURRENT_VIEW;
      start_line = target1.true_line;
      if (target1.num_lines < 0)
        end_line = (target1.true_line + target1.num_lines) + 1L;
      else
        end_line = (target1.true_line + target1.num_lines) - 1L;
      true_line = target2.true_line + target2.num_lines;
      lines_based_on_scope = TRUE;
      break;
  }
  free_target(&target1);
  free_target(&target2);
  /*
   * If the destination line for the copy is the *** Bottom of File ***
   * line, then subtract 1 to ensure lines don't get copied below the
   * *** Bottom of File *** line.
   */
  if (BOF(true_line))
    true_line--;
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  rc = rearrange_line_blocks(COMMAND_COPY, (char_t) reset_block, start_line, end_line, true_line, 1L, source_view, dest_view, lines_based_on_scope, &lines_affected);

  return (rc);
}
short Coverlay(char_t * params) {
  short rc = RC_OK;

  rc = column_command(params, COLUMN_COVERLAY);
  return (rc);
}
short Creplace(char_t * params) {
  short rc = RC_OK;

  rc = column_command(params, COLUMN_CREPLACE);
  return (rc);
}
short Cursor(char_t * params) {
  register short idx = 0;

#define CUR_PARAMS  6
  char_t *word[CUR_PARAMS + 1];
  char_t strip[CUR_PARAMS];
  unsigned short num_params = 0;
  bool time_to_leave = FALSE;
  short error_number = 1;
  char_t *error_message = (char_t *) "";
  short colno = 1;
  short rc = RC_OK;
  short state = CURSOR_START;
  short row = 0;
  col_t col = 0;
  line_t line = 0L;
  length_t column = 0L;
  char_t buffer[100];

  /* int priority; */

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(params, word, CUR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  error_message = word[0];
  /*
   * If the 2nd last word is "priority", then get the priority and reduce the number
   * of args by 2
   */
  if (num_params > 2) {
    if (equal((char_t *) "priority", word[num_params - 2], 1)) {
      if ((error_number = valid_positive_integer_against_maximum(word[num_params - 1], 256)) != 0) {
        if (error_number == 4)
          sprintf((char *) buffer, "%s", word[num_params - 1]);
        else
          sprintf((char *) buffer, "- MUST be <= %d", 256);
        display_error(error_number, buffer, FALSE);
        rc = RC_INVALID_OPERAND;
        return (rc);
      }
      /* priority = atoi( (char *)word[num_params-1] ); */
      num_params = num_params - 2;
    }
  }
  state = CURSOR_START;
  idx = 0;
  while (1) {
    switch (state) {
      case CURSOR_START:
        if (equal((char_t *) "escreen", word[idx], 1)) {
          state = CURSOR_ESCREEN;
          idx++;
          break;
        }
        if (equal((char_t *) "screen", word[idx], 1)) {
          state = CURSOR_SCREEN;
          idx++;
          break;
        }
        if (equal((char_t *) "cua", word[idx], 3)) {
          state = CURSOR_CUA;
          idx++;
          break;
        }
        if (equal((char_t *) "left", word[idx], 4)) {
          if (num_params > 1) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_left(CURSOR_ESCREEN, FALSE);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "right", word[idx], 5)) {
          if (num_params > 1) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_right(CURSOR_ESCREEN, FALSE);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "up", word[idx], 2)) {
          if (num_params > 1) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_up(CURSOR_ESCREEN);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "down", word[idx], 4)) {
          if (num_params > 1) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_down(current_screen, CURRENT_VIEW, CURSOR_ESCREEN);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "home", word[idx], 4)) {
          state = CURSOR_HOME;
          idx++;
          break;
        }
        if (equal((char_t *) "column", word[idx], 1)) {
          state = CURSOR_COLUMN;
          idx++;
          break;
        }
        if (equal((char_t *) "cmdline", word[idx], 2)) {
          state = CURSOR_CMDLINE;
          idx++;
          break;
        }
        if (equal((char_t *) "file", word[idx], 1)) {
          state = CURSOR_FILE;
          idx++;
          break;
        }
        if (equal((char_t *) "goto", word[idx], 4)) {
          state = CURSOR_GOTO;
          idx++;
          break;
        }
        if (equal((char_t *) "kedit", word[idx], 1)) {
          state = CURSOR_KEDIT;
          idx++;
          break;
        }
        if (equal((char_t *) "mouse", word[idx], 1)) {
          state = CURSOR_MOUSE;
          idx++;
          break;
        }
        if (equal((char_t *) "prefix", word[idx], 1)) {
          state = CURSOR_PREFIX;
          idx++;
          break;
        }
        state = CURSOR_ESCREEN;
        break;
      case CURSOR_HOME:
        if (num_params > 2) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        if (num_params == 2 && !equal((char_t *) "save", word[1], 4)) {
          state = CURSOR_ERROR;
          error_number = 1;
          error_message = word[idx];
          break;
        }
        if (num_params == 2)
          rc = THEcursor_home(current_screen, CURRENT_VIEW, TRUE);
        else
          rc = THEcursor_home(current_screen, CURRENT_VIEW, FALSE);
        time_to_leave = TRUE;
        break;
      case CURSOR_COLUMN:
        if (num_params != 1) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        rc = THEcursor_column();
        time_to_leave = TRUE;
        break;
      case CURSOR_FILE:
        if (num_params > 3) {
          state = CURSOR_ERROR;
          error_message = (char_t *) "";
          error_number = 2;
          break;
        }
        if (num_params < 2) {
          state = CURSOR_ERROR;
          error_message = (char_t *) "";
          error_number = 3;
          break;
        }
        if ((error_number = valid_positive_integer_against_maximum(word[1], CURRENT_FILE->number_lines)) != 0) {
          state = CURSOR_ERROR;
          if (error_number == 4)
            sprintf((char *) buffer, "%s", word[1]);
          else
            sprintf((char *) buffer, "- MUST be <= %ld", CURRENT_FILE->number_lines);
          error_message = buffer;
          break;
        }
        line = atol((char *) word[1]);
        if (num_params == 3) {
          if ((error_number = valid_positive_integer_against_maximum(word[2], MAX_WIDTH_NUM)) != 0) {
            state = CURSOR_ERROR;
            if (error_number == 4)
              sprintf((char *) buffer, "%s", word[2]);
            else
              sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
            error_message = buffer;
            break;
          }
          column = atol((char *) word[2]);
        } else
          column = 0L;
        rc = THEcursor_file(TRUE, line, column);
        time_to_leave = TRUE;
        break;
      case CURSOR_GOTO:
        if (num_params > 3) {
          state = CURSOR_ERROR;
          error_message = (char_t *) "";
          error_number = 2;
          break;
        }
        if (num_params < 3) {
          state = CURSOR_ERROR;
          error_message = (char_t *) "";
          error_number = 3;
          break;
        }
        if ((error_number = valid_positive_integer_against_maximum(word[1], CURRENT_FILE->number_lines)) != 0) {
          state = CURSOR_ERROR;
          if (error_number == 4)
            sprintf((char *) buffer, "%s", word[1]);
          else
            sprintf((char *) buffer, "- MUST be <= %ld", CURRENT_FILE->number_lines);
          error_message = buffer;
          break;
        }
        line = atol((char *) word[1]);
        if (num_params == 3) {
          if ((error_number = valid_positive_integer_against_maximum(word[2], MAX_WIDTH_NUM)) != 0) {
            state = CURSOR_ERROR;
            if (error_number == 4)
              sprintf((char *) buffer, "%s", word[2]);
            else
              sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
            error_message = buffer;
            break;
          }
          column = atol((char *) word[2]);
        } else
          column = 0;
        rc = THEcursor_goto(line, column);
        time_to_leave = TRUE;
        break;
      case CURSOR_MOUSE:
        if (num_params != 1) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        rc = THEcursor_mouse();
        time_to_leave = TRUE;
        break;
      case CURSOR_PREFIX:
        if (num_params != 1) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        /*
         * If the cursor is in the command line or there is no prefix on, exit.
         */
        if (CURRENT_VIEW->current_window == WINDOW_COMMAND || !CURRENT_VIEW->prefix) {
          ;
        } else {
          if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
            rc = Sos_prefix((char_t *) "");
          else
            rc = Sos_leftedge((char_t *) "");
        }
        time_to_leave = TRUE;
        break;
      case CURSOR_CMDLINE:
        if (num_params > 2) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        if (num_params == 2) {
          colno = atoi((char *) word[idx]);
          if (colno < 1) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
        }
        rc = THEcursor_cmdline(current_screen, CURRENT_VIEW, colno);
        time_to_leave = TRUE;
        break;
      case CURSOR_SCREEN:
      case CURSOR_ESCREEN:
      case CURSOR_CUA:
        if (equal((char_t *) "left", word[idx], 4)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_left(state, FALSE);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "right", word[idx], 5)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_right(state, FALSE);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "up", word[idx], 2)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_up(state);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "down", word[idx], 4)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_down(current_screen, CURRENT_VIEW, state);
          time_to_leave = TRUE;
          break;
        }
        /*
         * Only CURSOR SCREEN and CURSOR ESCREEN is valid here.
         * If CURSOR CUA, report an error
         */
        if (state == CURSOR_CUA) {
          state = CURSOR_ERROR;
          error_message = word[idx];
          error_number = 1;
          break;
        }
        if (equal(word[idx], EQUIVCHARstr, 1))
          row = 0;
        else {
          if (!valid_positive_integer(word[idx])) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 4;
            break;
          }
          row = atoi((char *) word[idx]);
          if (row == 0) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 5;
            break;
          }
        }
        idx++;
        if (strcmp((char *) word[idx], "") == 0) {
          if (state == CURSOR_ESCREEN)
            col = 1;
          else
            col = (CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT ? CURRENT_VIEW->prefix_width + 1 : 1;
        } else {
          if (equal(word[idx], EQUIVCHARstr, 1))
            col = 0;
          else {
            if (!valid_positive_integer(word[idx])) {
              state = CURSOR_ERROR;
              error_message = word[idx];
              error_number = 4;
              break;
            }
            col = atoi((char *) word[idx]);
            if (col == 0) {
              state = CURSOR_ERROR;
              error_message = word[idx];
              error_number = 5;
              break;
            }
          }
        }
        rc = THEcursor_move(current_screen, CURRENT_VIEW, TRUE, (bool) ((state == CURSOR_ESCREEN) ? TRUE : FALSE), row, col);
        time_to_leave = TRUE;
        break;
      case CURSOR_KEDIT:
        if (equal((char_t *) "left", word[idx], 4)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_left(CURSOR_ESCREEN, TRUE);
          time_to_leave = TRUE;
          break;
        }
        if (equal((char_t *) "right", word[idx], 5)) {
          if (num_params > 2) {
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
          }
          rc = THEcursor_right(CURSOR_ESCREEN, TRUE);
          time_to_leave = TRUE;
          break;
        }
        state = CURSOR_ERROR;
        error_message = word[idx];
        error_number = 1;
        break;
      case CURSOR_ERROR:
        display_error(error_number, error_message, FALSE);
        rc = RC_INVALID_OPERAND;
        time_to_leave = TRUE;
        break;
    }
    if (time_to_leave)
      break;
  }
  return (rc);
}
