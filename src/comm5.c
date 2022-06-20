// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * COMM5.C - Commands T-Z
 * This file contains all commands that can be assigned to function keys or typed on the command line.
 */

#include "the.h"
#include "proto.h"

static bool ispf_special_lines_entry(short line_type, int ch, char real_key) {
  bool need_to_build_screen = FALSE;

  /*
   * In ISPF mode, we need to allow certain characters to be
   * typed on the TABS line, and the BNDS line
   * Characters allowed are:
   * Char     TABS       BNDS
   *   <       -          Y
   *   >       -          Y
   *           Y          Y
   * etc... TBD
   *
   */
  switch (line_type) {

    case LINE_BOUNDS:
      switch (real_key) {
        case ' ':
          /*
           * If the existing character is < or >, then reset that particular edge of the BNDS
           */
          if (ch == '<' || ch == '>') {
            need_to_build_screen = TRUE;
          }
          break;

        case '<':
          need_to_build_screen = TRUE;
          break;

        case '>':
          need_to_build_screen = TRUE;
          break;
      }
      break;

    case LINE_TABLINE:
      break;

    default:
      break;
  }
  return need_to_build_screen;
}

short Tabfile(char* params) {
  short rc = RC_OK;
  int x, y;
  byte scrn;
  int w;
  VIEW_DETAILS* curr;
  char edit_fname[MAX_FILE_NAME];

  /*
   * Optionally only 1 argument allowed; + or -
   */
  if (strcmp((char *) params, "") == 0) {
    /*
     * If no parameter it is either called from a mouse click, or we assume we are called from the command line.
     * If called from mouse click, find file under mouse (or arrows)
     * If called from command line, edit first file displayed
     */
    which_window_is_mouse_in(&scrn, &w);
    if (w == (-1)) {
      /*
       * Edit first file displayed.
       */
      curr = find_filetab(-1);
      if (curr != NULL) {
        strcpy((char *) edit_fname, (char *) curr->file_for_view->fpath);
        strcat((char *) edit_fname, (char *) curr->file_for_view->fname);
        rc = EditFile(edit_fname, FALSE);
      }
      return (RC_OK);
    }
    /*
     * Get mouse position
     */
    wmouse_position(filetabs, &y, &x);
    curr = find_filetab(x);
    if (curr != NULL) {
      strcpy((char *) edit_fname, (char *) curr->file_for_view->fpath);
      strcat((char *) edit_fname, (char *) curr->file_for_view->fname);
      rc = EditFile(edit_fname, FALSE);
    }
    /*
     * If curr returned as NULL, then we have either clicked in a blank or on an arrow.
     * If on an arrow, we call this function again with a + or - parameter.
     */
  } else if (equal("+", params, 1)) {
    filetabs_start_view = find_next_file(filetabs_start_view ? filetabs_start_view : vd_current, DIRECTION_FORWARD);
  } else if (equal("-", params, 1)) {
    filetabs_start_view = find_next_file(filetabs_start_view ? filetabs_start_view : vd_current, DIRECTION_BACKWARD);
  } else {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  return (rc);
}

short Tabpre(char* params) {
  /*
   * deprecated
   */
  display_error(0, "TABPRE is deprecated. Use 'CURSOR PREFIX'", FALSE);
  return (RC_OK);
}

#define TAG_RTARGET 0
#define TAG_FOCUS   1
#define TAG_BLOCK   2

#define TAG_REPLACE 0
#define TAG_MORE    1
#define TAG_LESS    2
#define TAG_PARAMS  2

short Tag(char* params) {
  char strip[TAG_PARAMS];
  char* word[TAG_PARAMS + 1];
  short rc = RC_OK;
  LINE* curr = NULL;
  bool target_found = FALSE;
  short status = RC_OK;
  long target_type = TARGET_NORMAL;
  TARGET target;
  line_t line_number = 0L;
  line_t true_line;
  int tag_target = TAG_RTARGET;
  line_t num_lines = 0L;
  int relative = TAG_REPLACE;
  unsigned short num_params = 0;
  char* save_params = NULL;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_LEADING;
  /*
   * Get a copy of the parameters, because we want to manipulate them,
   */
  if ((save_params = my_strdup(params)) == NULL) {
    display_error(30, "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  num_params = param_split(params, word, TAG_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    if (CURRENT_FILE->number_lines == 0L) {
      return (rc);
    }
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE*) NULL, TRUE);
    curr = CURRENT_FILE->first_line->next;
    for (;;) {
      curr->flags.tag_flag = 0;
      curr = curr->next;
      if (curr->next == NULL)
        break;
    }
    build_screen(current_screen);
    display_screen(current_screen);
    return (rc);
  }
  if (CURRENT_FILE->number_lines == 0L) {
    display_error(17, params, FALSE);
    return (RC_TARGET_NOT_FOUND);
  }
  if (num_params == 1) {
    if (equal("more", word[0], 1) || equal("less", word[0], 1)) {
      if (save_params) {
        free(save_params);
      }
      display_error(3, "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    params = save_params;
  } else {
    params = word[1];
    if (equal("more", word[0], 1)) {
      relative = TAG_MORE;
    } else if (equal("less", word[0], 1)) {
      relative = TAG_LESS;
    } else {
      params = save_params;
    }
  }
  true_line = get_true_line(TRUE);
  /*
   * Check parameter
   */
  if (equal("focus", params, 1)) {
    tag_target = TAG_FOCUS;
  } else if (equal("block", params, 1)) {
    tag_target = TAG_BLOCK;
  } else {
    /*
     * Validate the parameters as valid targets...
     */
    initialise_target(&target);
    rc = parse_target(params, true_line, &target, target_type, TRUE, TRUE, FALSE);
    if (rc != RC_OK) {
      free_target(&target);
      if (save_params) {
        free(save_params);
      }
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Save the select levels for all lines in case no target is found.
   */
  curr = CURRENT_FILE->first_line->next;
  for (;;) {
    curr->flags.save_tag_flag = curr->flags.tag_flag;
    curr = curr->next;
    if (curr->next == NULL) {
      break;
    }
  }
  /*
   * Find all lines for the supplied target...
   */
  switch (tag_target) {

    case TAG_FOCUS:
      curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
      if (relative == TAG_LESS) {
        curr->flags.tag_flag = 0;
      } else {
        curr->flags.tag_flag = 1;
      }
      target_found = TRUE;
      break;

    case TAG_BLOCK:
      break;

    default:
      /*
       * Tell the target finding stuff we are the TAG command...
       */
      target.all_tag_command = TRUE;
      curr = CURRENT_FILE->first_line;
      status = FALSE;
      for (line_number = 0L; curr->next != NULL; line_number++) {
        status = find_rtarget_target(curr, &target, 0L, line_number, &num_lines);
        if (status == RC_OK) {  /* target found */
          target_found = TRUE;
          if (relative == TAG_LESS) {
            curr->flags.tag_flag = 0;
          } else {
            curr->flags.tag_flag = 1;
          }
        } else if (status == RC_TARGET_NOT_FOUND) {     /* target not found */
          if (relative == TAG_REPLACE) {
            curr->flags.tag_flag = 0;
          }
        } else {                /* error */
          break;
        }
        curr = curr->next;
      }
      break;
  }
  /*
   * If at least one line matches the target, set DISPLAY to 1 1,
   * otherwise reset the select levels as they were before the command.
   */
  if (target_found) {
    CURRENT_VIEW->highlight = HIGHLIGHT_TAG;
    build_screen(current_screen);
    display_screen(current_screen);
  } else {
    curr = CURRENT_FILE->first_line->next;
    for (;;) {
      curr->flags.tag_flag = curr->flags.save_tag_flag;
      curr = curr->next;
      if (curr->next == NULL) {
        break;
      }
    }
    if (status == RC_TARGET_NOT_FOUND) {
      display_error(17, params, FALSE);
      rc = RC_TARGET_NOT_FOUND;
    } else {
      rc = status;
    }
  }
  if (save_params) {
    free(save_params);
  }
  if (tag_target == TAG_RTARGET) {
    free_target(&target);
  }
  return (rc);
}

short Text(char* params) {
  length_t i = 0L;
  char real_key = 0;
  chtype chtype_key = 0;
  length_t x = 0;
  unsigned short y = 0;
  length_t len_params = 0L;
  chtype attr = 0;
  bool need_to_build_screen = FALSE;
  bool save_in_macro = in_macro;
  length_t new_len;

  /*
   * If running in read-only mode, do not allow any text to be entered in the main window.
   */
  if (ISREADONLY(CURRENT_FILE) && CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    display_error(56, "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  /*
   * If HEX mode is on, convert the hex string...
   */
  if (CURRENT_VIEW->hex && strlen((char *) params) > 3) {
    len_params = convert_hex_strings(params);
    switch (len_params) {

      case -1:                 /* invalid hex value */
        display_error(32, params, FALSE);
        return (RC_INVALID_OPERAND);
        break;

      case -2:                 /* memory exhausted */
        display_error(30, "", FALSE);
        return (RC_OUT_OF_MEMORY);
        break;

      default:
        break;
    }
  } else {
    len_params = strlen((char *) params);
  }
  for (i = 0; i < len_params; i++) {
    real_key = case_translate(*(params + i));
    chtype_key = (chtype) (real_key & A_CHARTEXT);

    getyx(CURRENT_WINDOW, y, x);

    attr = winch(CURRENT_WINDOW) & A_ATTRIBUTES;

    switch (CURRENT_VIEW->current_window) {

      case WINDOW_FILEAREA:
        if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE) {
          if (compatible_feel == COMPAT_ISPF && (CURRENT_SCREEN.sl[y].line_type == LINE_BOUNDS || CURRENT_SCREEN.sl[y].line_type == LINE_TABLINE)) {
            need_to_build_screen = ispf_special_lines_entry(CURRENT_SCREEN.sl[y].line_type, winch(CURRENT_WINDOW) & A_CHARTEXT, real_key);
          }
          break;
        }
        if ((length_t) (x + CURRENT_VIEW->verify_start) > (length_t) (CURRENT_VIEW->verify_end)) {
          break;
        }
        if (INSERTMODEx) {
          rec = meminschr(rec, real_key, CURRENT_VIEW->verify_col - 1 + x, max_line_length, rec_len);
          put_char(CURRENT_WINDOW, chtype_key | attr, INSCHAR);
        } else {
          rec[CURRENT_VIEW->verify_col - 1 + x] = real_key;
          if (x == CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1) {
            put_char(CURRENT_WINDOW, chtype_key | attr, INSCHAR);
          } else {
            put_char(CURRENT_WINDOW, chtype_key | attr, ADDCHAR);
          }
        }
        rec_len = calculate_rec_len((INSERTMODEx) ? ADJUST_INSERT : ADJUST_OVERWRITE, rec, rec_len, CURRENT_VIEW->verify_col + x, 1, CURRENT_FILE->trailing);
        /*
         * If THIGHLIGHT on focus line, reset it.
         */
        if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active && CURRENT_VIEW->thighlight_target.true_line == CURRENT_SCREEN.sl[y].line_number) {
          CURRENT_VIEW->thighlight_active = FALSE;
          need_to_build_screen = TRUE;
        }
        /*
         * check for the cursor moving past the right margin when WORDWRAP is ON.
         * If true, then don't execute the THEcursor_right() function, as this could cause a window scroll.
         */
        if (CURRENT_VIEW->wordwrap && rec_len > CURRENT_VIEW->margin_right) {
          execute_wrap_word(x + CURRENT_VIEW->verify_col);
        } else {
          /*
           * this is done here so that the show_page() in THEcursor_right() is executed AFTER we get the new length of rec_len.
           */
          if (INSERTMODEx || x == CURRENT_SCREEN.cols[WINDOW_FILEAREA] - 1) {
            THEcursor_right(TRUE, FALSE);
          }
        }
        /*
         * If HEXSHOW is on and we are on the current line, build screen...
         */
        if (CURRENT_VIEW->hexshow_on && CURRENT_VIEW->focus_line == CURRENT_VIEW->current_line) {
          need_to_build_screen = TRUE;
        }
        break;

      case WINDOW_COMMAND:
        if (INSERTMODEx) {
          cmd_rec = meminschr(cmd_rec, real_key, x + (cmd_verify_col - 1), max_line_length, cmd_rec_len);
          put_char(CURRENT_WINDOW, chtype_key, INSCHAR);
          cmd_rec_len = max(x + cmd_verify_col, cmd_rec_len + 1);       /* GFUC3 */
          THEcursor_right(TRUE, FALSE);
        } else {
          cmd_rec[x + (cmd_verify_col - 1)] = real_key;
          cmd_rec_len = max(x + cmd_verify_col, cmd_rec_len);
          if (x == CURRENT_SCREEN.cols[WINDOW_COMMAND] - 1) {
            put_char(CURRENT_WINDOW, chtype_key, INSCHAR);
            THEcursor_right(TRUE, FALSE);
          } else {
            put_char(CURRENT_WINDOW, chtype_key, ADDCHAR);
          }
        }
        /*
         * The cursor is now in the correct column for all cases and apart from the case where we have just scrolled right, the contents is displayed correctly.
         * We need to redisplay the cmdline if we just scrolled
         */
        if (x == CURRENT_SCREEN.cols[WINDOW_COMMAND] - 1) {
          display_cmdline(current_screen, CURRENT_VIEW);
        }
        break;

      case WINDOW_PREFIX:
        prefix_changed = TRUE;
        if (pre_rec_len == 0) {
          x = 0;
          wmove(CURRENT_WINDOW, y, x);
          my_wclrtoeol(CURRENT_WINDOW);
          wrefresh(CURRENT_WINDOW);
        }
        if (INSERTMODEx) {
          if (pre_rec_len == (CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap)) {
            break;
          }
          pre_rec = meminschr(pre_rec, real_key, x, CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap, pre_rec_len);
          put_char(CURRENT_WINDOW, chtype_key, INSCHAR);
        } else {
          pre_rec[x] = real_key;
          put_char(CURRENT_WINDOW, chtype_key, ADDCHAR);
        }
        wmove(CURRENT_WINDOW, y, min(x + 1, CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap - 1));
        new_len = memrevne(pre_rec, ' ', CURRENT_VIEW->prefix_width);
        if (new_len == (-1)) {
          pre_rec_len = 0;
        } else {
          pre_rec_len = new_len + 1;
        }
        break;
    }
  }
  if (in_macro) {
    need_to_build_screen = TRUE;
  }
  /*
   * Set in_macro to FALSE to allow for the refreshing of the screen.
   */
  in_macro = FALSE;
  /*
   * If text is being inserted on a line which is in the marked block, build and redisplay the window.
   */
  if (CURRENT_VIEW == MARK_VIEW && CURRENT_VIEW->current_window == WINDOW_FILEAREA && INSERTMODEx && CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line && CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line) {
    need_to_build_screen = TRUE;
  }
  /*
   * If the current file has colouring on and not using the NULL parser, build and redisplay the window.
   */
  if (CURRENT_FILE->colouring && CURRENT_FILE->parser && CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    need_to_build_screen = TRUE;
  }
  /*
   * If the current view has boundmarks on, build and redisplay the window.
   */
  if (CURRENT_VIEW->boundmark) {
    need_to_build_screen = TRUE;
  }
  /*
   * If we have determined we need to rebuild the screen, do it now.
   * FIXME: Need the ability to rebuild and redisplay one line only
   */
  if (need_to_build_screen && !in_readv) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  /*
   * Set in_macro back to its original value...
   */
  in_macro = save_in_macro;
  return (RC_OK);
}

short Toascii(char* params) {
  line_t num_lines = 0L, true_line = 0L, num_actual_lines = 0L, i = 0L, num_file_lines = 0L;
  short direction = 0;
  LINE* curr = NULL;
  length_t start_col = 0, end_col = 0;
  short rc = RC_OK;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  bool lines_based_on_scope = TRUE;
  bool adjust_alt = FALSE;

  if (strcmp("", (char *) params) == 0) {
    params = "+1";
  }
  initialise_target(&target);
  if ((rc = validate_target(params, &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE*) NULL, TRUE);
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
    rc = processable_line(CURRENT_VIEW, true_line + (line_t) (i * direction), curr);
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
          ebc2asc(curr->line, curr->length, mystart, myend);
        } else {
          ebc2asc(curr->line, curr->length, start_col, end_col);
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
    num_file_lines += (line_t) direction;
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
  /*
   * Display the new screen...
   */
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE*) NULL);
  resolve_current_and_focus_lines(current_screen, CURRENT_VIEW, true_line, num_file_lines, direction, TRUE, FALSE);
  return (RC_OK);
}

short Top(char* params) {
  short rc = RC_TOF_EOF_REACHED;
  unsigned short x = 0, y = 0;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW->current_line = 0L;
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE*) NULL, TRUE);
  build_screen(current_screen);
  if (!line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
    CURRENT_VIEW->focus_line = 0L;
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE*) NULL);
  if (curses_started) {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      getyx(CURRENT_PREV_WINDOW, y, x);
    } else {
      getyx(CURRENT_WINDOW, y, x);
    }
    display_screen(current_screen);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      wmove(CURRENT_PREV_WINDOW, y, x);
    } else {
      wmove(CURRENT_WINDOW, y, x);
    }
  }
  return (rc);
}

short Up(char* params) {
  short rc = RC_OK;
  line_t num_lines = 0L, true_line = 0L;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (strcmp("", (char *) params) == 0) {
    params = "1";
  }
  true_line = get_true_line(TRUE);
  if (strcmp("*", (char *) params) == 0) {
    num_lines = true_line + 1L;
  }
  else {
    if (!valid_integer(params)) {
      display_error(4, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    num_lines = atol((char *) params);
    if (num_lines < 0L) {
      display_error(5, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  rc = advance_current_or_focus_line(-num_lines);
  return (rc);
}

short Uppercase(char* params) {
  short rc = RC_OK;

  rc = execute_change_case(params, CASE_UPPER);
  return (rc);
}

short Xedit(char * params) {
  short rc = RC_OK;

  /*
   * The filename can be quoted; so strip leading and trailing double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  /*
   * Parse any parameters...future work.
   */
  rc = EditFile(params, FALSE);
  return (rc);
}

short Retrieve(char* params) {
  char* current_command = NULL;
  char* save_params = NULL;
  int param_len = 0;
  short direction = 0;

  /*
   * No parameters, get the last command...
   */
  if (strcmp((char *) params, "") == 0) {
    current_command = get_next_command(DIRECTION_FORWARD, 1);
  } else {
    /*
     * Get a copy of the parameters, because we want to manipulate them, and also retain the orignal for error reporting.
     */
    if ((save_params = my_strdup(params)) == NULL) {
      display_error(30, "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    /*
     * Strip all spaces from the parameters.
     * We want to be able to specify  ?  ? ? - as a valid set of arguments, equivalent to ???-
     */
    save_params = MyStrip(save_params, STRIP_ALL, ' ');
    param_len = strlen((char *) save_params);
    if (*(save_params + (param_len - 1)) == '+') {
      *(save_params + (param_len - 1)) = '\0';
      direction = DIRECTION_BACKWARD;
    } else {
      if (*(save_params + (param_len - 1)) == '-') {
        *(save_params + (param_len - 1)) = '\0';
        direction = DIRECTION_FORWARD;
      }
    }
    if (strzne(save_params, '?') != (-1)) {
      display_error(1, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    current_command = get_next_command(direction, strlen((char *) save_params) + 1);
  }
  if (save_params) {
    free(save_params);
  }
  wmove(CURRENT_WINDOW_COMMAND, 0, 0);
  my_wclrtoeol(CURRENT_WINDOW_COMMAND);
  if (current_command != NULL) {
    Cmsg(current_command);
  }
  return (RC_OK);
}

short Reexecute(char* params) {
  short rc = RC_OK;

  if (strcmp((char *) params, "")) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Retrieve the last command and execute it.
   */
  rc = command_line(last_command_for_reexecute, COMMAND_ONLY_FALSE);
  return (rc);
}
