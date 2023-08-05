// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* sos commands.                                           */

/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */

#include "the.h"
#include "proto.h"

static short sosdelback (bool);
static short sosdelchar (bool);

short Sos_addline(char_t *params) {
  short rc = RC_OK;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  insert_new_line(current_screen, CURRENT_VIEW, (char_t *) "", 0, 1, get_true_line(FALSE), FALSE, FALSE, TRUE, CURRENT_VIEW->display_low, TRUE, TRUE);
  if (compatible_feel == COMPAT_XEDIT) {
    advance_current_line(1L);
  }
  if (curses_started && CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    THEcursor_home(current_screen, CURRENT_VIEW, FALSE);
    rc = Sos_firstcol((char_t *) "");
  }
  return (rc);
}

short Sos_blockend(char_t *params) {
  short rc = RC_OK;
  LINE *curr = NULL;
  length_t col = 0;
  line_t line = 0L;
  short save_compat = 0;
  char_t cmd[20];

  if (MARK_VIEW != CURRENT_VIEW) {
    display_error(45, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  switch (MARK_VIEW->mark_type) {

    case M_COLUMN:
      col = MARK_VIEW->mark_end_col;
      line = CURRENT_FILE->number_lines;
      break;

    case M_LINE:
      line = MARK_VIEW->mark_end_line;
      break;

    default:
      col = MARK_VIEW->mark_end_col;
      line = MARK_VIEW->mark_end_line;
      break;
  }
  /* work out if block boundary is not excluded */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, line, CURRENT_FILE->number_lines);
  if (!IN_SCOPE(CURRENT_VIEW, curr)) {
    display_error(46, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
    Sos_leftedge((char_t *) "");
  }
  /*
   * Move to the line
   */
  save_compat = compatible_feel;
  compatible_feel = COMPAT_THE;
  sprintf((char *) cmd, ":%ld", (long) line);
  rc = command_line(cmd, COMMAND_ONLY_FALSE);
  compatible_feel = save_compat;;
  /*
   * Move to the column, except if the block is a LINE block or
   * we are on command line.
   */
  if (MARK_VIEW->mark_type != M_LINE && CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    execute_move_cursor(current_screen, CURRENT_VIEW, col - 1);
  }
  return (rc);
}

short Sos_blockstart(char_t *params) {
  short rc = RC_OK;
  LINE *curr = NULL;
  length_t col = 0;
  line_t line = 0L;
  short save_compat = 0;
  char_t cmd[20];

  if (MARK_VIEW != CURRENT_VIEW) {
    display_error(45, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  switch (MARK_VIEW->mark_type) {

    case M_COLUMN:
      col = MARK_VIEW->mark_start_col;
      line = 1;
      break;

    case M_LINE:
      line = MARK_VIEW->mark_start_line;
      break;

    default:
      col = MARK_VIEW->mark_start_col;
      line = MARK_VIEW->mark_start_line;
      break;
  }
  /* work out if block boundary is not excluded */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, line, CURRENT_FILE->number_lines);
  if (!IN_SCOPE(CURRENT_VIEW, curr)) {
    display_error(46, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
    Sos_leftedge((char_t *) "");
  }
  /*
   * Move to the line first
   */
  save_compat = compatible_feel;
  compatible_feel = COMPAT_THE;
  sprintf((char *) cmd, ":%ld", (long) line);
  rc = command_line(cmd, COMMAND_ONLY_FALSE);
  compatible_feel = save_compat;;
  /*
   * Move to the column, except if the block is a LINE block or
   * we are on command line.
   */
  if (MARK_VIEW->mark_type != M_LINE && CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    execute_move_cursor(current_screen, CURRENT_VIEW, col - 1);
  }
  return (rc);
}

short Sos_bottomedge(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0, x = 0, row = 0;

  getyx(CURRENT_WINDOW, y, x);
  /*
   * Get the last enterable row. If an error, stay where we are...
   */
  if (find_last_focus_line(current_screen, &row) != RC_OK) {
    return (rc);
  }
  /*
   * For each window determine what needs to be done...
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) != PREFIX_LEFT) {
        x += CURRENT_VIEW->prefix_width;
      }
      CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      CURRENT_VIEW->current_window = WINDOW_FILEAREA;
      wmove(CURRENT_WINDOW, row, x);
      break;

    case WINDOW_FILEAREA:
    case WINDOW_PREFIX:
      if (row != y) {           /* different rows */
        post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
        CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
        wmove(CURRENT_WINDOW, row, x);
      }
      break;
  }
  return (rc);
}

short Sos_cuadelback(char_t *params) {
  short rc;

  rc = sosdelback(TRUE);
  return (rc);
}

short Sos_cuadelchar(char_t *params) {
  short rc = RC_OK;

  rc = sosdelchar(TRUE);
  return (rc);
}

short Sos_current(char_t *params) {
  return do_Sos_current(params, current_screen, CURRENT_VIEW);
}

short do_Sos_current(char_t *params, char_t curr_screen, VIEW_DETAILS *curr_view) {
  short rc = RC_OK;
  unsigned short x = 0, y = 0;
  bool same_line = TRUE;

  getyx(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
  switch (curr_view->current_window) {

    case WINDOW_FILEAREA:
      if (curr_view->focus_line != curr_view->current_line) {
        post_process_line(curr_view, curr_view->focus_line, (LINE *) NULL, TRUE);
        curr_view->focus_line = curr_view->current_line;
        same_line = FALSE;
      }
      y = get_row_for_focus_line(curr_screen, curr_view->focus_line, curr_view->current_row);
      wmove(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
      if (!same_line) {
        pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
      }
      break;

    case WINDOW_PREFIX:
    case WINDOW_COMMAND:
      curr_view->focus_line = curr_view->current_line;
      y = get_row_for_focus_line(curr_screen, curr_view->focus_line, curr_view->current_row);
      curr_view->current_window = WINDOW_FILEAREA;
      wmove(SCREEN_WINDOW_FILEAREA(curr_screen), y, x);
      pre_process_line(curr_view, curr_view->focus_line, (LINE *) NULL);
      break;

    default:
      break;
  }
  return (rc);
}

short Sos_cursoradj(char_t *params) {
  length_t num_cols = 0, first_non_blank_col = 0, col = 0;
  short rc = RC_OK;
  unsigned short x = 0;

  x = getcurx(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      if (FOCUS_TOF || FOCUS_BOF) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      col = x + CURRENT_VIEW->verify_col - 1;
      first_non_blank_col = strzne(rec, ' ');
      if (first_non_blank_col == (-1)) {
        first_non_blank_col = 0;
      }
      num_cols = first_non_blank_col - col;
      if (num_cols < 0) {
        rc = execute_shift_command(current_screen, CURRENT_VIEW, FALSE, -num_cols, CURRENT_VIEW->focus_line, 1, FALSE, TARGET_UNFOUND, TRUE, FALSE);
      } else {
        if (num_cols > 0) {
          rc = execute_shift_command(current_screen, CURRENT_VIEW, TRUE, num_cols, CURRENT_VIEW->focus_line, 1, FALSE, TARGET_UNFOUND, TRUE, FALSE);
        }
      }
      break;

    default:
      break;
  }
  return (rc);
}

short Sos_cursorshift(char_t *params) {
  short rc = RC_OK;
  length_t num_cols = 0, first_non_blank_col = 0, col = 0;
  unsigned short x = 0;

  x = getcurx(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      if (FOCUS_TOF || FOCUS_BOF) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      col = x + CURRENT_VIEW->verify_col - 1;
      first_non_blank_col = col + strzne(rec + col, ' ');
      num_cols = first_non_blank_col - col;
      if (num_cols > 0) {
        memdeln(rec, col, rec_len, num_cols);
        rec_len -= num_cols;
      }
      break;

    default:
      break;
  }
  /*
   * If we changed anything, redisplay the screen.
   */
  if (num_cols > 0) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}

short Sos_delback(char_t *params) {
  short rc;

  rc = sosdelback(FALSE);
  return (rc);
}

short Sos_delchar(char_t *params) {
  short rc = RC_OK;

  rc = sosdelchar(FALSE);
  return (rc);
}

short Sos_delend(char_t *params) {
  length_t i, col;
  unsigned short x = 0, y = 0;

  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      /*
       * If running in read-only mode and an attempt is made to execute this
       * command in the MAIN window, then error...
       */
      if (ISREADONLY(CURRENT_FILE)) {
        display_error(56, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      col = x + CURRENT_VIEW->verify_col - 1;
      for (i = col; i < max_line_length; i++) {
        rec[i] = ' ';
      }
      if (rec_len > col) {
        rec_len = col;
      }
      my_wclrtoeol(CURRENT_WINDOW);
      break;

    case WINDOW_COMMAND:
      /*
       * Get a temporary value for position in cmd_rec
       */
      col = x + cmd_verify_col - 1;
      for (i = col; i < cmd_rec_len; i++) {
        cmd_rec[i] = ' ';
      }
      if (cmd_rec_len > col) {
        cmd_rec_len = col;
      }
      my_wclrtoeol(CURRENT_WINDOW);
      break;

    case WINDOW_PREFIX:
      if (x < pre_rec_len) {
        prefix_changed = TRUE;
        for (i = x; i < CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap; i++) {
          pre_rec[i] = ' ';
        }
        if (pre_rec_len > x) {
          pre_rec_len = x;
        }
        my_wclrtoeol(CURRENT_WINDOW);
      }
      break;

    default:
      break;
  }
  /*
   * If the character being deleted is on a line which is in the marked
   * block, and we are in the filearea, redisplay the screen.
   */
  if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    if ((CURRENT_VIEW == MARK_VIEW && CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line && CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line) || (CURRENT_FILE->colouring && CURRENT_FILE->parser)) {
      build_screen(current_screen);
      display_screen(current_screen);
    }
  }
  return (RC_OK);
}

short Sos_delline(char_t *params) {
  short rc = RC_OK;
  unsigned short x = 0, y = 0;
  line_t true_line = 0L, lines_affected = 0L;

  if (CURRENT_VIEW->current_window == WINDOW_FILEAREA || CURRENT_VIEW->current_window == WINDOW_PREFIX) {
    getyx(CURRENT_WINDOW, y, x);
    if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE && !CURRENT_VIEW->scope_all) {
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
    }
  }
  true_line = get_true_line(FALSE);
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  rc = rearrange_line_blocks(COMMAND_DELETE, SOURCE_COMMAND, true_line, true_line, true_line, 1L, CURRENT_VIEW, CURRENT_VIEW, FALSE, &lines_affected);
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    if (curses_started) {
      getyx(CURRENT_WINDOW, y, x);
    }
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen, CURRENT_VIEW->focus_line, y);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (curses_started) {
      wmove(CURRENT_WINDOW, y, x);
    }
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  return (rc);
}

short Sos_delword(char_t *params) {
  short rc = RC_OK;
  length_t first_col = 0, last_col = 0;
  unsigned short x = 0, y = 0;
  length_t num_cols = 0, left_col = 0, temp_rec_len = 0;
  char_t *temp_rec = NULL;

  /*
   * This function is not applicable to the PREFIX window.
   */
  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
      break;

    case WINDOW_FILEAREA:
      /*
       * If running in read-only mode and an attempt is made to execute this
       * command in the MAIN window, then error...
       */
      if (ISREADONLY(CURRENT_FILE)) {
        display_error(56, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      temp_rec = rec;
      temp_rec_len = rec_len;
      left_col = CURRENT_VIEW->verify_col - 1;
      break;

    case WINDOW_COMMAND:
      temp_rec = (char_t *) cmd_rec;
      temp_rec_len = cmd_rec_len;
      left_col = cmd_verify_col;
      break;
  }
  if (get_word(temp_rec, temp_rec_len, x + left_col, &first_col, &last_col) == 0) {
    return (0);
  }
  /*
   * Delete from the field the number of columns calculated above
   * and adjust the appropriate record length.
   */
  num_cols = last_col - first_col + 1;
  memdeln(temp_rec, first_col, temp_rec_len, num_cols);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      rec_len -= num_cols;
      rc = execute_move_cursor(current_screen, CURRENT_VIEW, first_col);
      build_screen(current_screen);
      display_screen(current_screen);
      break;

    case WINDOW_COMMAND:
      cmd_rec_len -= num_cols;
      display_cmdline(current_screen, CURRENT_VIEW);
      break;
  }
  return (rc);
}

short Sos_doprefix(char_t *params) {
  short rc = RC_OK;

  rc = execute_prefix_commands();
  return (rc);
}

short Sos_edit(char_t *params) {
  LINE *curr = NULL;
  char_t edit_fname[MAX_FILE_NAME];
  short rc = RC_OK;
  line_t true_line = 0L;
  char_t *lname = NULL, *fname = NULL;
  VIEW_DETAILS *dir = NULL;
  PRESERVED_VIEW_DETAILS *preserved_view_details = NULL;
  PRESERVED_FILE_DETAILS *preserved_file_details = NULL;

  /*
   * If the current file is not the special DIR.DIR file exit.
   */
  if (CURRENT_FILE->pseudo_file != PSEUDO_DIR) {
    return (RC_INVALID_ENVIRON);
  }
  /*
   * Determine which line contains a vaild file to edit. TOF and EOF are
   * invalid positions.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    if (CURRENT_TOF || CURRENT_BOF) {
      return (RC_INVALID_ENVIRON);
    }
    true_line = CURRENT_VIEW->current_line;
  } else {
    if (FOCUS_TOF || FOCUS_BOF) {
      return (RC_INVALID_ENVIRON);
    }
    true_line = CURRENT_VIEW->focus_line;
  }
  /*
   * Find the current LINE pointer for the focus_line.
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * Ensure that the line is long enough to have a filename on it...
   */
  if (rec_len <= FILE_START) {
    return (RC_INVALID_ENVIRON);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Validate that the supplied file is valid.
   */
  strcpy((char *) edit_fname, (char *) dir_path);
  fname = curr->line + FILE_START;
  if (*(curr->line) == 'l') {
    /*
     * We have a symbolic link.  Get the "real" file if there is one
     * i.e. the string AFTER "->" is the "real" file name.
     */
    lname = (char_t *) strstr((char *) fname, " -> ");
    if (lname != NULL) {
      if (strlen((char *) lname) > 4) {
        fname = lname + 4 * sizeof(char_t);
        if (*fname == '/') {    /* symbolic link is absolute... */
          strcpy((char *) edit_fname, "");
        }
      }
    }
  }
  strcat((char *) edit_fname, (char *) fname);
  if ((rc = splitpath(edit_fname)) != RC_OK) {
    display_error(10, edit_fname, FALSE);
    return (rc);
  }
  strcpy((char *) edit_fname, (char *) sp_path);
  strcat((char *) edit_fname, (char *) sp_fname);
  /*
   * If we are editing a directory and have a DIR.DIR file in the ring, find it...
   */
  if (strlen((char *) sp_fname) == 0) {
    dir = find_pseudo_file(PSEUDO_DIR);
    if (dir) {
      /* ... and preserve the settings so we can apply them to the new DIR.DIR */
      execute_preserve(dir, &preserved_view_details, dir->file_for_view, &preserved_file_details);
    }
  }
  /*
   * Edit the DIR.DIR file
   */
  rc = EditFile(edit_fname, FALSE);
  if (dir) {
    execute_restore(CURRENT_VIEW, &preserved_view_details, CURRENT_FILE, &preserved_file_details, FALSE);
    /*
     * Reset DISPLAY as we don't keep the settings
     */
    CURRENT_VIEW->display_low = 0;
    CURRENT_VIEW->display_high = 0;
    CURRENT_VIEW->scope_all = TRUE;
    build_screen(current_screen);
    display_screen(current_screen);
    display_cmdline(current_screen, CURRENT_VIEW);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  return (rc);
}

short Sos_endchar(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0;
  length_t charnum;

  y = getcury(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      charnum = pre_rec_len;
      wmove(CURRENT_WINDOW, y, min(charnum, CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap - 1));
      rc = RC_OK;
      break;

    case WINDOW_COMMAND:
      charnum = cmd_rec_len;
      rc = execute_move_cursor(current_screen, CURRENT_VIEW, charnum);
      break;

    case WINDOW_FILEAREA:
      charnum = rec_len;
      rc = execute_move_cursor(current_screen, CURRENT_VIEW, charnum);
      break;
  }
  return (rc);
}

short Sos_execute(char_t *params) {
  length_t i;
  short rc = RC_OK;
  bool save_in_macro;

  save_for_repeat = 1;
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    rc = THEcursor_cmdline(current_screen, CURRENT_VIEW, 1);
  }
  if (rc == RC_OK) {
    save_in_macro = in_macro;
    in_macro = FALSE;
    for (i = 0; i < cmd_rec_len; i++) {
      temp_cmd[i] = cmd_rec[i];
    }
    temp_cmd[cmd_rec_len] = '\0';
    MyStrip(temp_cmd, STRIP_LEADING, ' ');
    add_command(temp_cmd);
    rc = command_line(temp_cmd, COMMAND_ONLY_FALSE);
    in_macro = save_in_macro;
  }
  return (rc);
}

short Sos_firstchar(char_t *params) {
  short rc = RC_OK;
  length_t new_col = 0;
  unsigned short y = 0;
  LINE *curr = NULL;

  /*
   * For the command line and prefix area, just go to the first column...
   */
  y = getcury(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      wmove(CURRENT_WINDOW, y, 0);
      return (rc);

    case WINDOW_COMMAND:
      new_col = memne(cmd_rec, ' ', cmd_rec_len);
      break;

    case WINDOW_FILEAREA:
      curr = CURRENT_SCREEN.sl[y].current;
      new_col = memne(curr->line, ' ', curr->length);
      break;
  }
  /*
   * For the filearea and cmdline, move to the first non-blank...
   */
  if (new_col == (-1)) {
    new_col = 0;
  }
  rc = execute_move_cursor(current_screen, CURRENT_VIEW, new_col);
  return (rc);
}

short Sos_firstcol(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0;

  y = getcury(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      if (cmd_verify_col != 1) {
        cmd_verify_col = 1;
        display_cmdline(current_screen, CURRENT_VIEW);
      }
      break;

    case WINDOW_PREFIX:
      break;

    case WINDOW_FILEAREA:
      if (CURRENT_VIEW->verify_col != 1) {
        rc = execute_move_cursor(current_screen, CURRENT_VIEW, 0);
      }
      break;
  }
  wmove(CURRENT_WINDOW, y, 0);
  return (rc);
}

short Sos_instab(char_t *params) {
  unsigned short x = 0;
  short rc = RC_OK;
  length_t col = 0, tabcol = 0, i;

  if (CURRENT_VIEW->current_window != WINDOW_FILEAREA) {
    return (rc);
  }
  x = getcurx(CURRENT_WINDOW);
  col = x + CURRENT_VIEW->verify_col;
  for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
    if (col < CURRENT_VIEW->tabs[i]) {
      tabcol = CURRENT_VIEW->tabs[i];
      break;
    }
  }
  if (tabcol == 0) {            /* after last tab column or on a tab column */
    return (rc);
  }
  for (i = 0; i < tabcol - col; i++) {
    meminschr(rec, ' ', col - 1, max_line_length, rec_len++);
  }
  rec_len = min(max_line_length, rec_len);
  Sos_tabf((char_t *) "nochar");
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}

short Sos_lastcol(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0, x = 0;

  getyx(CURRENT_WINDOW, y, x);
  x = getmaxx(CURRENT_WINDOW) - 1;
  wmove(CURRENT_WINDOW, y, x);
  return (rc);
}

short Sos_leftedge(char_t *params) {
  unsigned short y = 0;

  y = getcury(CURRENT_WINDOW);
  if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
  }
  wmove(CURRENT_WINDOW, y, 0);
  return (RC_OK);
}

short Sos_makecurr(char_t *params) {
  short rc = RC_OK;

  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    rc = execute_makecurr(current_screen, CURRENT_VIEW, CURRENT_VIEW->focus_line);
  }
  return (rc);
}

short Sos_marginl(char_t *params) {
  short rc = RC_OK;

  if (Sos_leftedge((char_t *) "") == RC_OK) {
    rc = execute_move_cursor(current_screen, CURRENT_VIEW, CURRENT_VIEW->margin_left - 1);
  }
  return (rc);
}

short Sos_marginr(char_t *params) {
  short rc = RC_OK;

  if (Sos_leftedge((char_t *) "") == RC_OK) {
    rc = execute_move_cursor(current_screen, CURRENT_VIEW, CURRENT_VIEW->margin_right - 1);
  }
  return (rc);
}

short Sos_parindent(char_t *params) {
  short rc = RC_OK;
  length_t parindent = 0;

  if (CURRENT_VIEW->margin_indent_offset_status) {
    parindent = CURRENT_VIEW->margin_left + CURRENT_VIEW->margin_indent - 1;
  } else {
    parindent = CURRENT_VIEW->margin_indent - 1;
  }
  if (Sos_leftedge((char_t *) "") == RC_OK) {
    rc = execute_move_cursor(current_screen, CURRENT_VIEW, parindent);
  }
  return (rc);
}

short Sos_pastecmdline(char_t *params) {
  short rc = RC_OK;
  unsigned short x = 0, y = 0;
  LINE *curr = NULL;
  length_t start_col = 0, end_col = 0;
  length_t cursor_location = 0;
  length_t new_verify_col = 0;
  col_t new_screen_col = 0;
  length_t verify_col = 0;

  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    display_error(38, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (MARK_VIEW == NULL) {
    display_error(44, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (MARK_VIEW->mark_start_line != MARK_VIEW->mark_end_line) {
    display_error(81, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  curr = lll_find(MARK_FILE->first_line, MARK_FILE->last_line, MARK_VIEW->mark_start_line, MARK_FILE->number_lines);
  if (MARK_VIEW->mark_type == M_LINE) {
    start_col = 0;
    end_col = curr->length - 1;
  } else {
    start_col = MARK_VIEW->mark_start_col - 1;
    end_col = MARK_VIEW->mark_end_col - 1;
  }
  getyx(CURRENT_WINDOW, y, x);
  if (INSERTMODEx) {
    meminsmem(cmd_rec, curr->line + start_col, end_col - start_col + 1, x + cmd_verify_col - 1, max_line_length, cmd_rec_len);
    cmd_rec_len = max(cmd_rec_len, x + cmd_verify_col - 1) + end_col - start_col + 1;
  } else {
    memcpy(cmd_rec + x + cmd_verify_col - 1, curr->line + start_col, end_col - start_col + 1);
    cmd_rec_len = max(x + end_col - start_col + 1 + cmd_verify_col - 1, cmd_rec_len);
  }
  if (curses_started && CURRENT_WINDOW_COMMAND != (WINDOW *) NULL) {
    cursor_location = x + cmd_verify_col - 1 + end_col - start_col + 1;
    calculate_new_column(current_screen, CURRENT_VIEW, x, cmd_verify_col, cursor_location, &new_screen_col, &new_verify_col);
    if (verify_col != new_verify_col) {
      cmd_verify_col = new_verify_col;
    }
    display_cmdline(current_screen, CURRENT_VIEW);
    wmove(CURRENT_WINDOW, y, new_screen_col);
  }
  return (rc);
}

short Sos_prefix(char_t *params) {
  return do_Sos_prefix(params, current_screen, CURRENT_VIEW);
}

short do_Sos_prefix(char_t *params, char_t curr_screen, VIEW_DETAILS *curr_view) {
  short rc = RC_OK;
  unsigned short y = 0, x = 0;

  /*
   * If the cursor is in the command line or there is no prefix on, exit.
   */
  if (curr_view->current_window == WINDOW_COMMAND || !curr_view->prefix) {
    return (RC_OK);
  }
  post_process_line(curr_view, curr_view->focus_line, (LINE *) NULL, TRUE);
  getyx(SCREEN_WINDOW(curr_screen), y, x);
  if (curr_view->current_window == WINDOW_FILEAREA) {
    curr_view->current_window = WINDOW_PREFIX;
  }
  x = 0;
  wmove(SCREEN_WINDOW(curr_screen), y, x);
  return (rc);
}

short Sos_qcmnd(char_t *params) {
  short rc = RC_OK;

  if ((rc = THEcursor_cmdline(current_screen, CURRENT_VIEW, 1)) == RC_OK) {
    if (CURRENT_WINDOW_COMMAND != (WINDOW *) NULL) {
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
    }
    memset(cmd_rec, ' ', max_line_length);
    cmd_rec_len = 0;
  }
  return (rc);
}

short Sos_rightedge(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0, x = 0;

  getyx(CURRENT_WINDOW, y, x);
  if (CURRENT_VIEW->current_window == WINDOW_PREFIX) {
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
  }
  x = getmaxx(CURRENT_WINDOW) - 1;
  wmove(CURRENT_WINDOW, y, x);
  return (rc);
}

#define SETTAB_INSERT 0
#define SETTAB_APPEND 1
#define SETTAB_REMOVE 2

short Sos_settab(char_t *params) {
  unsigned short x = 0;
  short rc = RC_OK;
  length_t col = 0;
  length_t max_tab_col = 0;
  int action = SETTAB_INSERT;
  int i = 0, j = 0;

  if (CURRENT_VIEW->current_window != WINDOW_FILEAREA) {
    return (rc);
  }
  x = getcurx(CURRENT_WINDOW);
  col = x + CURRENT_VIEW->verify_col;
  for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
    if (CURRENT_VIEW->tabs[i] == col) {
      action = SETTAB_REMOVE;
      break;
    }
    if (max_tab_col < CURRENT_VIEW->tabs[i]) {
      max_tab_col = CURRENT_VIEW->tabs[i];
    }
  }
  if (action != SETTAB_REMOVE && col > max_tab_col) {
    action = SETTAB_APPEND;
  }
  switch (action) {

    case SETTAB_REMOVE:
      for (i = 0, j = 0; i < CURRENT_VIEW->numtabs; i++, j++) {
        if (CURRENT_VIEW->tabs[i] == col) {
          j++;
        }
        if (j < CURRENT_VIEW->numtabs) {
          CURRENT_VIEW->tabs[i] = CURRENT_VIEW->tabs[j];
        }
      }
      CURRENT_VIEW->numtabs--;
      CURRENT_VIEW->tabsinc = 0;
      break;

    case SETTAB_APPEND:
      if (CURRENT_VIEW->numtabs < MAX_NUMTABS) {
        CURRENT_VIEW->tabs[CURRENT_VIEW->numtabs] = col;
        CURRENT_VIEW->numtabs++;
        CURRENT_VIEW->tabsinc = 0;
      } else {
        display_error(79, (char_t *) "", FALSE);
        rc = RC_INVALID_ENVIRON;
      }
      break;

    default:
      if (CURRENT_VIEW->numtabs < MAX_NUMTABS) {
        for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
          if (col < CURRENT_VIEW->tabs[i]) {
            for (j = CURRENT_VIEW->numtabs - 1; j > i; j--) {
              CURRENT_VIEW->tabs[j] = CURRENT_VIEW->tabs[j - 1];
            }
            CURRENT_VIEW->tabs[i] = col;
            break;
          }
        }
        CURRENT_VIEW->numtabs++;
        CURRENT_VIEW->tabsinc = 0;
      } else {
        display_error(79, (char_t *) "", FALSE);
        rc = RC_INVALID_ENVIRON;
      }
      break;
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}

short Sos_startendchar(char_t *params) {
  unsigned short x = 0, y = 0;
  short rc = RC_OK;
  length_t charnum;

  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      charnum = pre_rec_len;
      if (x >= charnum) {
        wmove(CURRENT_WINDOW, y, 0);
      } else {
        wmove(CURRENT_WINDOW, y, charnum);
      }
      break;

    case WINDOW_COMMAND:
      charnum = cmd_rec_len;
      if (x + cmd_verify_col > charnum) {
        rc = Sos_firstcol((char_t *) "");
      } else {
        rc = Sos_endchar((char_t *) "");
      }
      break;

    case WINDOW_FILEAREA:
      charnum = rec_len;
      if (x + CURRENT_VIEW->verify_col > min(charnum, CURRENT_VIEW->verify_end)) {
        rc = Sos_firstcol((char_t *) "");
      } else {
        rc = Sos_endchar((char_t *) "");
      }
      break;
  }
  return (rc);
}

short Sos_tabb(char_t *params) {
  unsigned short x = 0, y = 0;
  length_t prev_tab_col = 0, current_col = 0;
  col_t new_screen_col = 0;
  length_t new_verify_col = 0;
  length_t verify_col = 0;
  short rc = RC_OK;

  getyx(CURRENT_WINDOW, y, x);
  /*
   * Determine action depending on current window...
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      return (RC_OK);
      break;

    case WINDOW_FILEAREA:
      current_col = x + CURRENT_VIEW->verify_col;
      verify_col = CURRENT_VIEW->verify_col;
      break;

    case WINDOW_COMMAND:
      current_col = x + cmd_verify_col;
      verify_col = cmd_verify_col;
      break;
  }
  /*
   * First determine the next tab stop column...
   */
  prev_tab_col = find_prev_tab_col(current_col);
  /*
   * If no prev tab column, stay where we are and return...
   */
  if (prev_tab_col == 0) {
    return (RC_OK);
  }
  /*
   * For all windows, if the new cursor position does not exceed the
   * right edge, move there.
   */
  prev_tab_col--;               /* zero base the column */
  calculate_new_column(current_screen, CURRENT_VIEW, x, verify_col, prev_tab_col, &new_screen_col, &new_verify_col);
  if (verify_col != new_verify_col) {
    switch (CURRENT_VIEW->current_window) {

      case WINDOW_COMMAND:
        cmd_verify_col = new_verify_col;
        display_cmdline(current_screen, CURRENT_VIEW);
        break;

      case WINDOW_FILEAREA:
        CURRENT_VIEW->verify_col = new_verify_col;
        build_screen(current_screen);
        display_screen(current_screen);
        break;
    }
  }
  wmove(CURRENT_WINDOW, y, new_screen_col);
  return (rc);
}

short Sos_tabf(char_t *params) {
  unsigned short x = 0, y = 0;
  length_t next_tab_col = 0, current_col = 0;
  col_t new_screen_col = 0;
  length_t new_verify_col = 0;
  length_t verify_col = 0;
  short rc = RC_OK;

  /*
   * If the actual tab character is to be display then exit so that
   * editor() can process it as a raw key.
   * Ignore this test if the parameter 'nochar' is passed.  The command
   * SOS INSTAB is the only way that 'nochar' can be passed; you cannot
   * "DEFINE akey SOS TABF NOCHAR"!!
   */
  if (strcmp((char *) params, "nochar") != 0) {
    if (INSERTMODEx && tabkey_insert == 'C') {
      return (RAW_KEY);
    }
    if (!INSERTMODEx && tabkey_overwrite == 'C') {
      return (RAW_KEY);
    }
  }
  getyx(CURRENT_WINDOW, y, x);
  /*
   * Determine action depending on current window...
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      return (RC_OK);
      break;

    case WINDOW_FILEAREA:
      current_col = x + CURRENT_VIEW->verify_col;
      verify_col = CURRENT_VIEW->verify_col;
      break;

    case WINDOW_COMMAND:
      current_col = x + cmd_verify_col;
      verify_col = cmd_verify_col;
      break;
  }
  /*
   * First determine the next tab stop column...
   */
  next_tab_col = find_next_tab_col(current_col);
  /*
   * If no next tab column, stay where we are and return...
   */
  if (next_tab_col == 0) {
    return (RC_OK);
  }
  /*
   * Check for going past end of line - max_line_length
   */
  if (next_tab_col > max_line_length) {
    return (RC_TRUNCATED);
  }
  /*
   * For all windows, if the new cursor position does not exceed the
   * right edge, move there.
   */
  next_tab_col--;               /* zero base the column */
  calculate_new_column(current_screen, CURRENT_VIEW, x, verify_col, next_tab_col, &new_screen_col, &new_verify_col);
  if (verify_col != new_verify_col) {
    switch (CURRENT_VIEW->current_window) {

      case WINDOW_COMMAND:
        cmd_verify_col = new_verify_col;
        display_cmdline(current_screen, CURRENT_VIEW);
        break;

      case WINDOW_FILEAREA:
        CURRENT_VIEW->verify_col = new_verify_col;
        build_screen(current_screen);
        display_screen(current_screen);
        break;
    }
  }
  wmove(CURRENT_WINDOW, y, new_screen_col);
  return (rc);
}

short Sos_tabfieldb(char_t *params) {
  short rc = RC_OK;
  long save_where = 0L, where = 0L, what_current = 0L, what_other = 0L;
  unsigned short y = 0, x = 0;

  /*
   * Determine if the cursor is in the left-most column of the current
   * field (col 0)...
   */
  getyx(CURRENT_WINDOW, y, x);
  if (x != 0) {
    wmove(CURRENT_WINDOW, y, 0);
    return (rc);
  }
  /*
   * ... otherwise determine which is the previous enterable field and
   * move the cursor there.
   */
  save_where = where = where_now();
  what_current = what_current_now();
  what_other = what_other_now();
  for (;;) {
    where = where_before(where, what_current, what_other);
    if (where == save_where) {
      break;
    }
    if (enterable_field(where)) {
      break;
    }
  }
  /*
   * If we can't go anywhere, stay where we are...
   */
  if (where == save_where) {
    return (rc);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  rc = go_to_new_field(save_where, where);
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  return (rc);
}

short Sos_tabfieldf(char_t *params) {
  short rc = RC_OK;
  long save_where = 0L, where = 0L, what_current = 0L, what_other = 0L;

  save_where = where = where_now();
  what_current = what_current_now();
  what_other = what_other_now();
  for (;;) {
    where = where_next(where, what_current, what_other);
    if (where == save_where) {
      break;
    }
    if (enterable_field(where)) {
      break;
    }
  }
  /*
   * If we can't go anywhere, stay where we are...
   */
  if (where == save_where) {
    return (rc);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  rc = go_to_new_field(save_where, where);
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  return (rc);
}

short Sos_tabwordb(char_t *params) {
  unsigned short x = 0, y = 0;
  length_t start_word_col = 0;
  unsigned short word_break = 0;
  char_t *temp_rec = NULL;
  length_t i = 0;
  bool blank_found = FALSE;
  length_t left_col = 0;
  length_t verify_col = 0;
  col_t new_screen_col = 0;
  length_t new_verify_col = 0;
  short current_char_type = 0;
  char_t this_char = 0;
  short rc = RC_OK;

  /*
   * This function is not applicable to the PREFIX window.
   */
  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
      break;

    case WINDOW_FILEAREA:
      temp_rec = rec;
      verify_col = CURRENT_VIEW->verify_col;
      break;

    case WINDOW_COMMAND:
      temp_rec = (char_t *) cmd_rec;
      verify_col = cmd_verify_col;
      break;
  }
  left_col = verify_col - 1;
  /*
   * Determine the start of the prior word, or go to the start of the
   * line if already at or before beginning of prior word.
   */
  word_break = 0;
  start_word_col = (-1);
  if (CURRENT_VIEW->word == 'N') {
    /*
     * Word break is non-blank
     */
    for (i = left_col + x; i > (-1); i--) {
      switch (word_break) {

        case 0:                /* still in current word */
          if (*(temp_rec + i) == ' ') {
            word_break++;
          }
          break;

        case 1:                /* in first blank space */
          if (*(temp_rec + i) != ' ') {
            word_break++;
          }
          break;

        case 2:                /* in previous word */
          if (*(temp_rec + i) == ' ') {
            start_word_col = i + 1;
            word_break++;
          }
          break;

        default:               /* should not get here */
          break;
      }
      if (word_break == 3) {
        break;
      }
    }
  } else {
    /*
     * Word break is non-blank
     */
    word_break = 0;
    this_char = *(temp_rec + left_col + x);
    current_char_type = my_isalphanum(this_char);
    for (i = left_col + x; i > (-1); i--) {
      switch (word_break) {

        case 0:                /* still in current word or blank */
          if (current_char_type == CHAR_SPACE && my_isalphanum(*(temp_rec + i)) != CHAR_SPACE) {
            word_break++;
            current_char_type = my_isalphanum(*(temp_rec + i));
            break;
          }
          if (*(temp_rec + i) == ' ') {
            blank_found = TRUE;
            break;
          }
          if (blank_found) {
            word_break++;
            current_char_type = my_isalphanum(*(temp_rec + i));
            break;
          }
          if (current_char_type == CHAR_ALPHANUM && my_isalphanum(*(temp_rec + i)) != CHAR_ALPHANUM) {
            word_break++;
            current_char_type = my_isalphanum(*(temp_rec + i));
            break;
          }
          if (current_char_type == CHAR_OTHER && my_isalphanum(*(temp_rec + i)) != CHAR_OTHER) {
            word_break++;
            current_char_type = my_isalphanum(*(temp_rec + i));
            break;
          }
          break;

        case 1:                /* now at end of previous word */
          if (current_char_type == CHAR_ALPHANUM && (my_isalphanum(*(temp_rec + i)) != CHAR_ALPHANUM || *(temp_rec + i) == ' ')) {
            word_break++;
            break;
          }
          if (current_char_type == CHAR_OTHER && (my_isalphanum(*(temp_rec + i)) != CHAR_OTHER || *(temp_rec + i) == ' ')) {
            word_break++;
            break;
          }
          break;

        default:               /* should not get here */
          break;
      }
      if (word_break == 2) {
        start_word_col = i + 1;
        break;
      }
    }
  }
  if (start_word_col == (-1)) {
    start_word_col = 0;
  }
  calculate_new_column(current_screen, CURRENT_VIEW, x, verify_col, start_word_col, &new_screen_col, &new_verify_col);
  if (verify_col != new_verify_col) {
    switch (CURRENT_VIEW->current_window) {

      case WINDOW_COMMAND:
        cmd_verify_col = new_verify_col;
        display_cmdline(current_screen, CURRENT_VIEW);
        break;

      case WINDOW_FILEAREA:
        CURRENT_VIEW->verify_col = new_verify_col;
        build_screen(current_screen);
        display_screen(current_screen);
        break;
    }
  }
  wmove(CURRENT_WINDOW, y, new_screen_col);
  return (rc);
}

short Sos_tabwordf(char_t *params) {
  unsigned short x = 0, y = 0;
  length_t temp_rec_len = 0;
  length_t start_word_col = 0, left_col = 0;
  length_t verify_col = 0;
  bool word_break = FALSE;
  short current_char_type = 0;
  char_t *temp_rec = NULL;
  char_t this_char = 0;
  length_t i = 0;
  col_t new_screen_col = 0;
  length_t new_verify_col = 0;
  short rc = RC_OK;

  /*
   * This function is not applicable to the PREFIX window.
   */
  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_PREFIX:
      display_error(38, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
      break;

    case WINDOW_FILEAREA:
      temp_rec = rec;
      temp_rec_len = rec_len;
      verify_col = CURRENT_VIEW->verify_col;
      break;

    case WINDOW_COMMAND:
      temp_rec = (char_t *) cmd_rec;
      temp_rec_len = cmd_rec_len;
      verify_col = cmd_verify_col;
      break;
  }
  left_col = verify_col - 1;
  /*
   * If we are after the last column of the line, then just ignore the
   * command and leave the cursor where it is.
   */
  if ((x + left_col) > temp_rec_len) {
    return (RC_OK);
  }
  /*
   * Determine the start of the next word, or go to the end of the line
   * if already at or past beginning of last word.
   */
  word_break = FALSE;
  start_word_col = (-1);
  if (CURRENT_VIEW->word == 'N') {
    /*
     * Word break is non-blank
     */
    for (i = left_col + x; i < temp_rec_len; i++) {
      if (*(temp_rec + i) == ' ') {
        word_break = TRUE;
      } else {
        if (word_break) {
          start_word_col = i;
          break;
        }
      }
    }
  } else {
    /*
     * Word break is non-blank
     */
    this_char = *(temp_rec + left_col + x);
    current_char_type = my_isalphanum(this_char);
    for (i = left_col + x; i < temp_rec_len; i++) {
      switch (current_char_type) {

        case CHAR_SPACE:
          if (*(temp_rec + i) != ' ') {
            start_word_col = i;
          }
          break;

        case CHAR_ALPHANUM:
          if (*(temp_rec + i) == ' ') {
            word_break = TRUE;
            break;
          }
          if (my_isalphanum(*(temp_rec + i)) != CHAR_ALPHANUM || word_break) {
            start_word_col = i;
          }
          break;

        default:
          if (*(temp_rec + i) == ' ') {
            word_break = TRUE;
            break;
          }
          if (my_isalphanum(*(temp_rec + i)) == CHAR_ALPHANUM || word_break) {
            start_word_col = i;
          }
          break;
      }
      if (start_word_col != (-1)) {
        break;
      }
    }
  }
  if (start_word_col == (-1)) {
    start_word_col = temp_rec_len;
  }
  calculate_new_column(current_screen, CURRENT_VIEW, x, verify_col, start_word_col, &new_screen_col, &new_verify_col);
  if (verify_col != new_verify_col) {
    switch (CURRENT_VIEW->current_window) {

      case WINDOW_COMMAND:
        cmd_verify_col = new_verify_col;
        display_cmdline(current_screen, CURRENT_VIEW);
        break;

      case WINDOW_FILEAREA:
        CURRENT_VIEW->verify_col = new_verify_col;
        build_screen(current_screen);
        display_screen(current_screen);
        break;
    }
  }
  wmove(CURRENT_WINDOW, y, new_screen_col);
  return (rc);
}

short Sos_topedge(char_t *params) {
  short rc = RC_OK;
  unsigned short y = 0, x = 0, row = 0;

  getyx(CURRENT_WINDOW, y, x);
  /*
   * Get the last enterable row. If an error, stay where we are...
   */
  if (find_first_focus_line(current_screen, &row) != RC_OK) {
    return (rc);
  }
  /*
   * For each window determine what needs to be done...
   */
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) != PREFIX_LEFT) {
        x += CURRENT_VIEW->prefix_width;
      }
      CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      CURRENT_VIEW->current_window = WINDOW_FILEAREA;
      wmove(CURRENT_WINDOW, row, x);
      break;

    case WINDOW_FILEAREA:
    case WINDOW_PREFIX:
      if (row != y) {           /* different rows */
        post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
        CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
        wmove(CURRENT_WINDOW, row, x);
      }
      break;
  }
  return (rc);
}

short Sos_undo(char_t *params) {
  unsigned short y = 0;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      build_screen(current_screen);
      display_screen(current_screen);
      break;

    case WINDOW_COMMAND:
      memset(cmd_rec, ' ', max_line_length);
      cmd_rec_len = 0;
      wmove(CURRENT_WINDOW, 0, 0);
      my_wclrtoeol(CURRENT_WINDOW);
      break;

    case WINDOW_PREFIX:
      prefix_changed = TRUE;
      memset(pre_rec, ' ', MAX_PREFIX_WIDTH);
      pre_rec_len = 0;
      y = getcury(CURRENT_WINDOW);
      wmove(CURRENT_WINDOW, y, 0);
      my_wclrtoeol(CURRENT_WINDOW);
      break;

    default:
      break;
  }
  return (RC_OK);
}

/*
 * Internal functions...
 */
static short sosdelback(bool cua) {
  unsigned short x = 0, y = 0;
  short rc = RC_OK;

  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      /*
       * If running in read-only mode and an attempt is made to execute this
       * command in the MAIN window, then error...
       */
      if (ISREADONLY(CURRENT_FILE)) {
        display_error(56, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      break;

    case WINDOW_COMMAND:
      if (x == 0 && cmd_verify_col == 1) {
        /*
         * We are at the first position of the cmdline
         */
        return (RC_OK);
      }
      if (x == 0) {
        /*
         * We are at the left edge of the CMDLINE with the CMDLINE scrolled
         * so we will need to move the cursor somewhere to the right and
         * scroll the CMDLINE to the left.
         */
        col_t new_screen_col = 0;
        length_t new_verify_col = 0;
        if (x + cmd_verify_col - 1 <= cmd_rec_len) {
          memdeln(cmd_rec, x - 1 + cmd_verify_col - 1, cmd_rec_len, 1);
          cmd_rec_len--;
        }
        calculate_new_column(current_screen, CURRENT_VIEW, x, cmd_verify_col, x + cmd_verify_col - 2, &new_screen_col, &new_verify_col);
        if (cmd_verify_col != new_verify_col) {
          cmd_verify_col = new_verify_col;
          display_cmdline(current_screen, CURRENT_VIEW);
          wmove(CURRENT_WINDOW, y, new_screen_col);
        }
      } else {
        wmove(CURRENT_WINDOW, y, x - 1);
        if (x + cmd_verify_col - 1 <= cmd_rec_len) {
          memdeln(cmd_rec, x - 1 + cmd_verify_col - 1, cmd_rec_len, 1);
          cmd_rec_len--;
        }
        display_cmdline(current_screen, CURRENT_VIEW);
      }
      return (RC_OK);
      break;

    case WINDOW_PREFIX:
      if (x == 0) {
        return (RC_OK);
      }
      wmove(CURRENT_WINDOW, y, x - 1);
      if (x <= pre_rec_len) {
        prefix_changed = TRUE;
        my_wdelch(CURRENT_WINDOW);
        memdeln(pre_rec, x - 1, pre_rec_len, 1);
        pre_rec_len--;
      }
      return (RC_OK);
      break;

    default:
      break;
  }
  // Remainder of processing is only for WINDOW_FILEAREA.
  /*
   * If we have a marked CUA block, then move the cursor to the
   * start of the marked block, and delete the block. Don't attempt
   * to move the cursor left.
   */
  if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
    ResetOrDeleteCUABlock(CUA_DELETE_BLOCK);
    return (RC_OK);
  }

  /*
   * If we are in the first column of the file...
   */
  if (x == 0 && CURRENT_VIEW->verify_start == CURRENT_VIEW->verify_col) {
    /*
     * Check if we have been passed "cua" as an optional parameter.
     * If so, and we are not on the first line of the
     * file, join with the line above
     */
    if (cua && CURRENT_SCREEN.sl[y].line_number != 1L) {
      /*
       * advance to previous line
       * move to end of line
       * JOIN
       * only do this if the previous line is not excluded
       */
      if (1) {
        advance_focus_line(-1);
        Sos_endchar((char_t *) "");
        rc = execute_split_join(SPLTJOIN_JOIN, TRUE, TRUE);
        return (rc);
      } else {
        return (RC_OK);
      }
    } else {
      /*
       * NOT CUA, or on first line, exit.
       */
      return (RC_OK);
    }
  }
  THEcursor_left(TRUE, FALSE);
  /*
   * If we are after the last character of the line, exit.
   */
  if (x + CURRENT_VIEW->verify_col - 1 > rec_len) {
    return (RC_OK);
  }
  getyx(CURRENT_WINDOW, y, x);
  my_wdelch(CURRENT_WINDOW);
  memdeln(rec, CURRENT_VIEW->verify_col - 1 + x, rec_len, 1);
  rec_len--;
  /*
   * If there is a character off the right edge of the screen, display it
   * in the last character of the main window.
   */
  if (CURRENT_VIEW->verify_col - 1 + CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1 < rec_len) {
    wmove(CURRENT_WINDOW, y, CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1);
    put_char(CURRENT_WINDOW, rec[CURRENT_VIEW->verify_col - 1 + CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1], ADDCHAR);
    wmove(CURRENT_WINDOW, y, x);
  }
  /*
   * If the character being deleted is on a line which is in the marked
   * block, redisplay the window.
   */
  if ((CURRENT_VIEW == MARK_VIEW && CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line && CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line) || (CURRENT_FILE->colouring && CURRENT_FILE->parser)) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return rc;
}

static short sosdelchar(bool cua) {
  unsigned short x = 0, y = 0;
  short rc;

  getyx(CURRENT_WINDOW, y, x);
  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      my_wdelch(CURRENT_WINDOW);
      if (x + cmd_verify_col <= cmd_rec_len) {
        memdeln(cmd_rec, x + cmd_verify_col - 1, cmd_rec_len, 1);
        cmd_rec_len--;
        display_cmdline(current_screen, CURRENT_VIEW);
        wmove(CURRENT_WINDOW, y, x);
      }
      return (RC_OK);
      break;

    case WINDOW_PREFIX:
      if (x < pre_rec_len) {
        my_wdelch(CURRENT_WINDOW);
        prefix_changed = TRUE;
        memdeln(pre_rec, x, pre_rec_len, 1);
        pre_rec_len--;
      }
      return (RC_OK);
      break;

    case WINDOW_FILEAREA:
      /*
       * If running in read-only mode and an attempt is made to execute this
       * command in the MAIN window, then error...
       */
      if (ISREADONLY(CURRENT_FILE)) {
        display_error(56, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      /*
       * Do not allow this command on the top or bottom of file lines or on
       * shadow lines.
       */
      if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
        display_error(38, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      break;

    default:
      break;
  }
  // Remainder of processing is only for WINDOW_FILEAREA.
  /*
   * If we have a marked CUA block, then move the cursor to the
   * start of the marked block, and delete the block. Don't attempt
   * to move the cursor left.
   */
  if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
    ResetOrDeleteCUABlock(CUA_DELETE_BLOCK);
    return (RC_OK);
  }
  my_wdelch(CURRENT_WINDOW);
  /*
   * If we are not after the last character of the line...
   */
  if (x + CURRENT_VIEW->verify_col <= rec_len) {
    memdeln(rec, CURRENT_VIEW->verify_col - 1 + x, rec_len, 1);
    rec_len--;
    /*
     * If there is a character off the right edge of the screen, display it
     * in the last character of the main window.
     */
    if (CURRENT_VIEW->verify_col - 1 + CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1 < rec_len) {
      wmove(CURRENT_WINDOW, y, CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1);
      put_char(CURRENT_WINDOW, rec[CURRENT_VIEW->verify_col - 1 + CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1], ADDCHAR);
      wmove(CURRENT_WINDOW, y, x);
    }
  } else {
    /*
     * Check if we have been passed "cua" as an optional parameter.
     * If so, SOS DELCHAR after the end of the line is
     * equivalent to a JOIN
     */
    if (cua) {
      rc = execute_split_join(SPLTJOIN_JOIN, TRUE, TRUE);
      return (rc);
    }
  }
  /*
   * If the character being deleted is on a line which is in the marked
   * block...
   */
  if ((CURRENT_VIEW == MARK_VIEW && CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line && CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line) || (CURRENT_FILE->colouring && CURRENT_FILE->parser)) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (RC_OK);
}

