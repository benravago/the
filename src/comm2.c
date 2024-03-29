// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* Commands D-J                                              */

/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */

#include "the.h"
#include "proto.h"

#define DEF_PARAMS  2
#define DEF_MOUSE_PARAMS  4

short Define(char_t *params) {
  char_t *word[DEF_MOUSE_PARAMS + 1];
  char_t strip[DEF_MOUSE_PARAMS];
  char_t *ptr = NULL;
  unsigned short num_params = 0;
  int key_value = 0;
  short rc = RC_OK;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_LEADING;
  num_params = param_split(params, word, DEF_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * The first parameter is the key name mnemonic , the next is one or
   * more commands and/or parameters.
   * First check the mnemonic for decimal string value. ie begins with \
   */
  if (word[0][0] == '\\') {
    if ((key_value = atoi((char *) word[0] + 1)) == 0) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  } else {
    if ((key_value = find_key_name(word[0])) == (-1)) {
      if ((key_value = find_mouse_key_value(word[0])) == (-1)) {
        return (RC_INVALID_OPERAND);
      }
      /*
       * We have an invalid key, but a valid mouse key
       */
      rc = RC_INVALID_OPERAND;
    }
  }
  if (rc == RC_OK) {
    /*
     * Determine if the first word of the supplied command is REXX (either
     * case)...
     */
    if (strlen((char *) word[1]) > 5 && memcmpi(word[1], (char_t *) "REXX ", 5) == 0) {
      ptr = word[1];
      rc = add_define(&first_define, &last_define, key_value, ptr + 5, TRUE, FALSE, 0);
    } else {
      rc = add_define(&first_define, &last_define, key_value, word[1], FALSE, FALSE, 0);
    }
    /*
     * Return after processing a KEY definition...
     */
    return (rc);
  }
  /*
   * To get here it is a MOUSE key definition waiting to be validated.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_NONE;
  num_params = param_split(params, word, DEF_MOUSE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 3) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (!equal((char_t *) "in", word[1], 2)) {
    display_error(1, word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if ((key_value = find_mouse_key_value_in_window(word[0], word[2])) == (-1)) {
    return (RC_INVALID_OPERAND);
  }
  /*
   * Determine if the first word of the supplied command is REXX (either
   * case)...
   */
  if (strlen((char *) word[1]) > 5 && memcmpi(word[3], (char_t *) "REXX ", 5) == 0) {
    ptr = word[3];
    rc = add_define(&first_mouse_define, &last_mouse_define, key_value, ptr + 5, TRUE, FALSE, 0);
  } else {
    rc = add_define(&first_mouse_define, &last_mouse_define, key_value, word[3], FALSE, FALSE, 0);
  }
  return (rc);
}

short DeleteLine(char_t *params) {
  line_t start_line = 0L, end_line = 0L, dest_line = 0L, lines_affected = 0L;
  short rc = RC_OK;
  char_t *args = NULL;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_ALL | TARGET_BLOCK_CURRENT;
  bool lines_based_on_scope = FALSE;

  /*
   * If no parameter is supplied, 1 is assumed.
   */
  if (blank_field(params)) {
    args = (char_t *) "+1";
  } else {
    args = params;
  }
  initialise_target(&target);
  if ((rc = validate_target(args, &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  /*
   * If the target is BLOCK and the marked block is a box block, call
   * box_operations(), otherwise delete specified lines.
   */
  if (target.rt[0].target_type == TARGET_BLOCK_CURRENT) {
    /*
     * For box blocks, call the appropriate function...
     */
    if (MARK_VIEW->mark_type != M_LINE) {
      free_target(&target);
      box_operations(BOX_D, SOURCE_BLOCK_RESET, FALSE, ' ');
      return (RC_OK);
    }
    start_line = MARK_VIEW->mark_start_line;
    end_line = MARK_VIEW->mark_end_line;
    dest_line = MARK_VIEW->mark_start_line;
    lines_based_on_scope = FALSE;
  } else {
    start_line = target.true_line;
    if (target.num_lines < 0L) {
      end_line = (target.true_line + target.num_lines) + 1L;
      dest_line = end_line;
    } else {
      end_line = (target.true_line + target.num_lines) - 1L;
      dest_line = start_line;
    }
    lines_based_on_scope = TRUE;
  }
  free_target(&target);
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (target.num_lines != 0L) {
    rc = rearrange_line_blocks(COMMAND_DELETE, SOURCE_COMMAND, start_line, end_line, dest_line, 1L, CURRENT_VIEW, CURRENT_VIEW, lines_based_on_scope, &lines_affected);
  }
  CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, NULL, CURRENT_VIEW->current_line, DIRECTION_FORWARD);
  start_line = find_next_in_scope(CURRENT_VIEW, NULL, CURRENT_VIEW->focus_line, DIRECTION_FORWARD);
  if (CURRENT_VIEW->focus_line != start_line) {
    CURRENT_VIEW->focus_line = start_line;
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  if (rc == RC_OK) {
    if (CURRENT_BOF || CURRENT_TOF) {
      rc = RC_TOF_EOF_REACHED;
    }
  }
  return (rc);
}

short Dialog(char_t *params) {
  short rc = RC_OK;

  rc = prepare_dialog(params, FALSE, (char_t *) "DIALOG");
  return (rc);
}

#define DIR_PARAMS  1

short Directory(char_t *params) {
  char_t *word[DIR_PARAMS + 1];
  char_t strip[DIR_PARAMS];
  char_t quoted[DIR_PARAMS];
  VIEW_DETAILS *dir = NULL;
  PRESERVED_VIEW_DETAILS *preserved_view_details = NULL;
  PRESERVED_FILE_DETAILS *preserved_file_details = NULL;
  unsigned short num_params = 0;
  short rc = RC_OK;

  /*
   * Validate the parameters that have been supplied. The one and only
   * parameter should be the directory to display.
   */
  strip[0] = STRIP_BOTH;
  quoted[0] = '"';
  num_params = quoted_param_split(params, word, DIR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE, quoted);
  if (num_params > 1) {
    display_error(1, (char_t *) word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate that the supplied directory is valid.
   */
  if ((rc = splitpath(strrmdup(strtrans(word[0], OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    display_error(10, (char_t *) word[0], FALSE);
    return (rc);
  }
  if ((rc = read_directory()) != RC_OK) {
    if (strcmp((char *) sp_fname, "") == 0) {
      display_error(10, (char_t *) word[0], FALSE);
    } else {
      display_error(9, (char_t *) word[0], FALSE);
    }
    return (rc);
  }
  strcpy((char *) temp_cmd, (char *) dir_pathname);
  strcat((char *) temp_cmd, (char *) dir_filename);
  /*
   * If we have a DIR.DIR file in the ring, find it...
   */
  dir = find_pseudo_file(PSEUDO_DIR);
  if (dir) {
    /* ... and preserve the settings so we can apply them to the new DIR.DIR */
    execute_preserve(dir, &preserved_view_details, dir->file_for_view, &preserved_file_details);
  }
  /*
   * Edit the DIR.DIR file
   */
  rc = EditFile(temp_cmd, FALSE);
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
  return (RC_OK);
}

#define DUP_PARAMS  2

short Duplicate(char_t *params) {
  char_t *word[DUP_PARAMS + 1];
  char_t strip[DUP_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  line_t num_occ = 0L;
  line_t start_line = 0L, end_line = 0L, dest_line = 0L, lines_affected = 0L;
  char_t command_source = 0;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  bool lines_based_on_scope = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_LEADING;
  num_params = param_split(params, word, DUP_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If no parameters, default to 1 1
   */
  if (num_params == 0) {
    word[0] = (char_t *) "1";
    word[1] = (char_t *) "1";
  }
  /*
   * If 1 parameter, default 2nd parameter to 1
   */
  if (num_params == 1) {
    word[1] = (char_t *) "1";
  }
  /*
   * If first parameter is not an integer, error.
   */
  if (!valid_integer(word[0])) {
    display_error(4, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  num_occ = atol((char *) word[0]);
  if (num_occ == 0L) {
    display_error(6, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate second parameter is a valid target...
   */
  initialise_target(&target);
  if ((rc = validate_target(word[1], &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    free_target(&target);
    return (rc);
  }
  /*
   * Duplicate lines depending on target type...
   */
  switch (target.rt[0].target_type) {

    case TARGET_BLOCK_CURRENT:
      /*
       * This function not valid for box  blocks.
       */
      if (MARK_VIEW->mark_type == M_BOX) {
        display_error(48, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      command_source = SOURCE_BLOCK;
      start_line = MARK_VIEW->mark_start_line;
      end_line = dest_line = MARK_VIEW->mark_end_line;
      lines_based_on_scope = FALSE;
      break;

    default:
      command_source = SOURCE_COMMAND;
      if (target.num_lines < 0L) {
        start_line = target.true_line + target.num_lines + 1L;
        end_line = dest_line = target.true_line;
      } else {
        start_line = target.true_line;
        end_line = dest_line = (target.true_line + target.num_lines) - 1L;
      }
      lines_based_on_scope = TRUE;
      break;
  }
  if ((start_line == 0 && end_line == 0) || target.num_lines == 0) {
    ; // no-op
  } else {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    rc = rearrange_line_blocks(COMMAND_DUPLICATE, command_source, start_line, end_line, dest_line, num_occ, CURRENT_VIEW, CURRENT_VIEW, lines_based_on_scope, &lines_affected);
  }
  free_target(&target);
  return (rc);
}

#define EDITV_PARAMS  2

short THEEditv(char_t *params) {
  char_t *word[EDITV_PARAMS + 1];
  char_t strip[EDITV_PARAMS];
  unsigned short num_params = 0;
  short editv_type = 0;
  short rc = RC_OK;
  bool editv_file = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_LEADING;
  num_params = param_split(params, word, EDITV_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Determine the subcommand...
   */
  if (equal((char_t *) "get", word[0], 3)) {
    editv_type = EDITV_GET;
  } else if (equal((char_t *) "put", word[0], 3)) {
    editv_type = EDITV_PUT;
  } else if (equal((char_t *) "set", word[0], 3)) {
    editv_type = EDITV_SET;
  } else if (equal((char_t *) "setl", word[0], 4)) {
    editv_type = EDITV_SETL;
  } else if (equal((char_t *) "list", word[0], 4)) {
    editv_type = EDITV_LIST;
  } else if (equal((char_t *) "getf", word[0], 4)) {
    editv_type = EDITV_GET;
    editv_file = TRUE;
  } else if (equal((char_t *) "putf", word[0], 4)) {
    editv_type = EDITV_PUT;
    editv_file = TRUE;
  } else if (equal((char_t *) "setf", word[0], 4)) {
    editv_type = EDITV_SET;
    editv_file = TRUE;
  } else if (equal((char_t *) "setlf", word[0], 5)) {
    editv_type = EDITV_SETL;
    editv_file = TRUE;
  } else if (equal((char_t *) "setfl", word[0], 5)) {
    editv_type = EDITV_SETL;
    editv_file = TRUE;
  } else if (equal((char_t *) "listf", word[0], 5)) {
    editv_type = EDITV_LIST;
    editv_file = TRUE;
  } else if (equal((char_t *) "getstem", word[0], 7)) {
    editv_type = EDITV_GETSTEM;
  } else if (equal((char_t *) "getstemf", word[0], 8)) {
    editv_type = EDITV_GETSTEM;
    editv_file = TRUE;
  } else {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Only LIST and LISTF are allowed no parameters...
   */
  if (editv_type != EDITV_LIST && num_params == 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * GET, PUT, GETF and PUTF only allowed in a macro...
   */
  if (editv_type == EDITV_GET || editv_type == EDITV_PUT) {
    if (!in_macro) {
      display_error(53, (char_t *) "", FALSE);
      return (RC_INVALID_ENVIRON);
    }
  }
  rc = execute_editv(editv_type, editv_file, word[1]);
  return (rc);
}

short Emsg(char_t *params) {
  display_error(0, params, FALSE);
  return (RC_OK);
}

short Enter(char_t *params) {
  unsigned y = 0;
  short rc = RC_OK;

  switch (CURRENT_VIEW->current_window) {

    case WINDOW_COMMAND:
      rc = Sos_execute((char_t *) "");
      break;

    case WINDOW_PREFIX:
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
      if (CURRENT_FILE->first_ppc == NULL) {    /* no pending prefix cmds */
        THEcursor_down(current_screen, CURRENT_VIEW, TRUE);
        rc = Sos_firstcol((char_t *) "");
      } else {
        Sos_doprefix((char_t *) "");
      }
      break;

    case WINDOW_FILEAREA:
      /*
       * If in readonly mode, ignore new line addition...
       */
      if (!ISREADONLY(CURRENT_FILE)) {
        if (equal((char_t *) "cua", params, 3)) {
          /*
           * Split the line at the cursor position
           * move the cursor to the first character of the FILEAREA
           */
          if (CURRENT_VIEW->newline_aligned) {
            rc = execute_split_join(SPLTJOIN_SPLIT, TRUE, TRUE);
            rc = Sos_firstchar((char_t *) "");
          } else {
            rc = execute_split_join(SPLTJOIN_SPLIT, FALSE, TRUE);
            rc = Sos_firstcol((char_t *) "");
          }
          THEcursor_down(current_screen, CURRENT_VIEW, TRUE);
          return (rc);
        } else {
          if (CURRENT_VIEW->inputmode == INPUTMODE_LINE) {
            post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
            insert_new_line(current_screen, CURRENT_VIEW, (char_t *) "", 0, 1, get_true_line(FALSE), FALSE, FALSE, TRUE, CURRENT_VIEW->display_low, TRUE, TRUE);
            break;
          }
        }
      }
      THEcursor_down(current_screen, CURRENT_VIEW, TRUE);
      y = getcury(CURRENT_WINDOW);
      wmove(CURRENT_WINDOW, y, 0);
      break;
  }
  return (rc);
}

short Expand(char_t *params) {
  short rc = RC_OK;

  rc = execute_expand_compress(params, TRUE, TRUE, TRUE, TRUE);
  return (rc);
}

short Extract(char_t *params) {
  register short i = 0;
  short rc = RC_OK, itemno = 0, num_items = 0, len = 0, num_values = 0;
  short pos = 0, arglen = 0;
  char_t *args = NULL;
  char_t delim;
  bool invalid_item = FALSE;
  char_t item_type = 0;

  if (!in_macro || !rexx_support) {
    display_error(53, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  /*
   * The first character is saved as the delimiter...
   */
  delim = *(params);
  params++;                     /* throw away first delimiter */
  strtrunc(params);
  len = strlen((char *) params);
  /*
   * Check that we have an item to extract...
   */
  if (len == 0) {
    invalid_item = TRUE;
  } else {
    if (len == 1 && (*(params) == delim)) {
      invalid_item = TRUE;
    }
  }
  if (invalid_item) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Allow for no trailing delimiter...
   */
  if (*(params + len - 1) == delim) {
    num_items = 0;
  } else {
    num_items = 1;
  }
  /*
   * Replace all / with nul character to give us separate strings.
   */
  for (i = 0; i < len; i++) {
    if (*(params + i) == delim) {
      *(params + i) = '\0';
      num_items++;
    }
  }
  /*
   * For each item, extract its variables...
   */
  for (i = 0; i < num_items; i++) {
    /*
     * First check if the item has any arguments with it.
     */
    arglen = strlen((char *) params);
    pos = strzeq(params, ' ');
    if (pos == (-1)) {
      args = (char_t *) "";
    } else {
      *(params + pos) = '\0';
      args = strtrunc(params + pos + 1);
    }
    /*
     * Find the item in the list of valid extract options...
     */
    if ((itemno = find_query_item(params, strlen((char *) params), &item_type)) == (-1) || !(item_type & QUERY_EXTRACT)) {
      display_error(1, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    /*
     * Get the current settings for the valid item...
     */
    num_values = get_item_values(1, itemno, args, QUERY_EXTRACT, 0L, NULL, 0L);
    /*
     * If the arguments to the item are invalid, return with an error.
     */
    if (num_values == EXTRACT_ARG_ERROR) {
      return (RC_INVALID_OPERAND);
    }
    /*
     * If the REXX variables have already been set, don't try to set them.
     */
    if (num_values != EXTRACT_VARIABLES_SET) {
      rc = set_extract_variables(itemno);
      if (rc == RC_SYSTEM_ERROR) {
        break;
      }
    }
    params += arglen + 1;
  }
  return (rc);
}

short Ffile(char_t *params) {
  short rc = RC_OK;

  /*
   * The filename can be quoted; so strip leading and trailing
   * double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if ((rc = save_file(CURRENT_FILE, params, TRUE, CURRENT_FILE->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, FALSE)) != RC_OK) {
    return (rc);
  }
  /*
   * If autosave is on at the time of FFiling, remove the .aus file...
   */
  if (CURRENT_FILE->autosave > 0) {
    rc = remove_aus_file(CURRENT_FILE);
  }
  free_view_memory(TRUE, TRUE);
  return (rc);
}

short File(char_t *params) {
  short rc = RC_OK;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * The filename can be quoted; so strip leading and trailing
   * double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  if ((rc = save_file(CURRENT_FILE, params, FALSE, CURRENT_FILE->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, FALSE)) != RC_OK) {
    return (rc);
  }
  /*
   * If autosave is on at the time of Filing, remove the .aus file...
   */
  if (CURRENT_FILE->autosave > 0) {
    rc = remove_aus_file(CURRENT_FILE);
  }
  free_view_memory(TRUE, TRUE);
  return (rc);
}

short Fillbox(char_t *params) {
  int key = 0;
  short len_params = 0;
  short y = 0, x = 0;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Validate the marked block.
   */
  if (marked_block(TRUE) != RC_OK) {
    return (RC_INVALID_ENVIRON);
  }
  /*
   * Check if hex on in effect and translate hex char if required...
   */
  if (CURRENT_VIEW->hex) {
    len_params = convert_hex_strings(params);
    switch (len_params) {

      case -1:                 /* invalid hex value */
        display_error(32, params, FALSE);
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
    len_params = strlen((char *) params);
  }
  /*
   * Whew, now do something...
   */
  if (len_params > 1) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (len_params == 0) {
    key = (int) ' ';
  } else {
    key = (int) *(params);
  }
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND && len_params != 1) {
    getyx(CURRENT_WINDOW, y, x);
    display_prompt((char_t *) "Enter fill character...");
    wmove(CURRENT_WINDOW_FILEAREA, y, x);
    wrefresh(CURRENT_WINDOW_FILEAREA);
    for (;;) {
      key = wgetch(CURRENT_WINDOW);
      if (!is_modifier_key(key)) {
        break;
      }
    }
    clear_msgline(-1);
  }
  box_operations(BOX_F, SOURCE_BLOCK, TRUE, (char_t) key);
  return (RC_OK);
}

short Find(char_t *params) {
  short rc = RC_OK;

  rc = execute_find_command(params, TARGET_FIND);
  return (rc);
}

short Findup(char_t *params) {
  short rc = RC_OK;

  rc = execute_find_command(params, TARGET_FINDUP);
  return (rc);
}

#define FOR_PARAMS  2

short Forward(char_t *params) {
  char_t *word[FOR_PARAMS + 1];
  char_t strip[FOR_PARAMS];
  unsigned short num_params = 0;
  short scroll_by_page = 1;     /* by default we scroll pages */
  line_t num_pages = 0L;
  short rc = RC_OK;

  /*
   * Validate parameters...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, FOR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {

    case 0:
      num_pages = 1;
      break;

    case 1:
      if (strcmp((char *) word[0], "*") == 0) {
        /*
         * If parameter is '*', set current line equal to last line in file...
         */
        rc = Bottom((char_t *) "");
        return (rc);
      } else if (equal((char_t *) "HALF", word[0], 4)) {
        /*
         * If parameter is 'HALF', advance half a page
         */
        scroll_by_page = 0;
        num_pages = CURRENT_SCREEN.rows[WINDOW_FILEAREA] / 2;
      }
      else if (!valid_positive_integer(word[0])) {
        /*
         * If the parameter is not a valid positive integer, error.
         */
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
   * If the current line is already on "Bottom of File" or the parameter
   * is 0, go to the top of the file.
   */
  if (num_pages == 0 || (CURRENT_BOF && PAGEWRAPx)) {
    rc = Top((char_t *) "");
    return (rc);
  }
  /*
   * Scroll the screen num_pages...
   */
  if (scroll_by_page) {
    rc = scroll_page(DIRECTION_FORWARD, num_pages, FALSE);
  } else {
    rc = advance_current_line(num_pages);
  }
  return (rc);
}

#define GET_PARAMS  3

short Get(char_t *params) {
  char_t *word[GET_PARAMS + 1];
  char_t strip[GET_PARAMS];
  char_t quoted[GET_PARAMS];
  unsigned short num_params = 0;
  char_t *filename = NULL;
  FILE *fp = NULL;
  LINE *curr = NULL;
  LINE *save_curr = NULL;
  LINE *save_next = NULL;
  line_t old_number_lines = 0L, true_line = 0L;
  short rc = RC_OK;
  line_t fromline = 1L, numlines = 0L;
  bool clip = FALSE;
  // int clip_type;
  char_t  buffer[100];

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  quoted[0] = '"';
  quoted[1] = '\0';
  quoted[2] = '\0';
  num_params = quoted_param_split(params, word, GET_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE, quoted);
  if (num_params > 3) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (num_params == 0) {
    filename = tempfilename;
  } else {
    if (equal((char_t *) "clip:", word[0], 5)) {
      clip = TRUE;
      if (num_params > 2) {
        display_error(2, (char_t *) "", FALSE);
        return (RC_INVALID_ENVIRON);
      } else if (num_params == 1) {
        // clip_type = M_LINE;
      } else {
        if (equal((char_t *) "line", word[1], 4)) {
          // clip_type = M_LINE;
        } else if (equal((char_t *) "stream", word[1], 6)) {
          // clip_type = M_STREAM;
        } else if (equal((char_t *) "box", word[1], 3)) {
          // clip_type = M_BOX;
        } else {
          display_error(1, word[1], FALSE);
          return (RC_INVALID_ENVIRON);
        }
      }
    } else {
      if ((rc = splitpath(strrmdup(strtrans(word[0], OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
        display_error(10, word[0], FALSE);
        return (rc);
      }
      strcpy((char *) temp_cmd, (char *) sp_path);
      strcat((char *) temp_cmd, (char *) sp_fname);
      filename = temp_cmd;
      if (num_params == 2 || num_params == 3) {
        if ((rc = valid_positive_integer_against_maximum(word[1], MAX_WIDTH_NUM)) != 0) {
          if (rc == 4) {
            sprintf((char *) buffer, "%s", word[1]);
          } else {
            sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
          }
          display_error(rc, buffer, FALSE);
          return (RC_INVALID_OPERAND);
        }
        fromline = atol((char *) word[1]);
        if (fromline == 0L) {
          display_error(4, word[1], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
      if (num_params == 3) {
        if (strcmp((char *) word[2], "*") == 0) {
          numlines = 0L;
        } else {
          if ((rc = valid_positive_integer_against_maximum(word[2], MAX_WIDTH_NUM)) != 0) {
            if (rc == 4) {
              sprintf((char *) buffer, "%s", word[2]);
            } else {
              sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
            }
            display_error(rc, buffer, FALSE);
            return (RC_INVALID_OPERAND);
          } else
            numlines = atol((char *) word[2]);
        }
      }
    }
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (!clip) {
    if (!file_readable(filename)) {
      display_error(8, filename, FALSE);
      return (RC_ACCESS_DENIED);
    }
    if ((fp = fopen((char *) filename, "r")) == NULL) {
      display_error(9, params, FALSE);
      return (RC_ACCESS_DENIED);
    }
  }
  true_line = get_true_line(TRUE);
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  if (curr->next == NULL) {     /* on bottom of file */
    curr = curr->prev;
  }
  old_number_lines = CURRENT_FILE->number_lines;
  save_curr = curr;
  save_next = curr->next;
  if (clip) {
    curr = getclipboard(curr, 1);
  } else {
    curr = read_file(fp, curr, filename, fromline, numlines, TRUE);
  }
  if (curr == NULL) {
    for (curr = save_curr; curr != save_next;) {
      if (curr != save_curr) {
        curr = lll_del(&CURRENT_FILE->first_line, &CURRENT_FILE->last_line, curr, DIRECTION_FORWARD);
      } else {
        curr = curr->next;
      }
    }
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    if (!clip) {
      fclose(fp);
    }
    return (RC_ACCESS_DENIED);
  }
  if (!clip) {
    fclose(fp);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  /*
   * Fix the positioning of the marked block (if there is one and it is
   * in the current view).
   */
  adjust_marked_lines(TRUE, true_line, CURRENT_FILE->number_lines - old_number_lines);
  adjust_pending_prefix(CURRENT_VIEW, TRUE, true_line, CURRENT_FILE->number_lines - old_number_lines);
  /*
   * Increment the number of lines counter for the current file and the
   * number of alterations.
   */
  increment_alt(CURRENT_FILE);
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}

short Help(char_t *params) {
  static bool first = TRUE;
  char *envptr = NULL;

  /*
   * No arguments are allowed; error if any are present.
   */
  if (strcmp((char *) params, "") != 0) {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Set up help file name.
   */
  if (first) {
    if ((envptr = getenv("THE_HELP_FILE")) != NULL) {
      strcpy((char *) the_help_file, envptr);
    } else {
      strcpy((char *) the_help_file, (char *) the_home_dir);
      strcat((char *) the_help_file, "THE_Help.txt");
    }
    strrmdup(strtrans(the_help_file, OSLASH, ISLASH), ISLASH, TRUE);
    first = FALSE;
  }
  if (file_exists(the_help_file) != THE_FILE_EXISTS) {
    display_error(23, (char_t *) the_help_file, FALSE);
    return (RC_FILE_NOT_FOUND);
  }
  Xedit(the_help_file);
  return (RC_OK);
}

#define HIT_MOUSE_PARAMS  4

short Hit(char_t *params) {
  char_t *word[HIT_MOUSE_PARAMS + 1];
  char_t strip[HIT_MOUSE_PARAMS];
  unsigned short num_params = 0;
  int key = 0;
  short rc = RC_OK;
  bool save_in_macro = in_macro;
  bool mouse_details_present = FALSE;

  /*
   * Parse the parameters into multiple words. If only one word then it
   * must be a key.  If 3 words its a mouse key, otherwise asn error.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_NONE;
  num_params = param_split(params, word, HIT_MOUSE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {

    case 1:                    /* key definition */
      key = find_key_name(params);
      if (key == (-1)) {
        display_error(13, params, FALSE);
        return (RC_INVALID_OPERAND);
      }
      break;

    case 3:                    /* mouse definition */
      if (!equal((char_t *) "in", word[1], 2)) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if ((key = find_mouse_key_value_in_window(word[0], word[2])) == (-1)) {
        return (RC_INVALID_OPERAND);
      }
      mouse_details_present = TRUE;
      break;

    default:                   /* error */
      break;
  }
  /*
   * Only argument is the name of a valid key.
   */
  in_macro = FALSE;
  rc = process_key(key, mouse_details_present);
  in_macro = save_in_macro;
  if (number_of_files == 0) {
    rc = RC_INVALID_ENVIRON;
  }
  /* how to exit ???? */
  return (rc);
}

short Input(char_t *params) {
  length_t len_params = 0;
  short rc = RC_OK;

  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (CURRENT_VIEW->hex) {
    len_params = convert_hex_strings(params);
    switch (len_params) {

      case -1:                 /* invalid hex value */
        display_error(32, params, FALSE);
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
    len_params = strlen((char *) params);
  }
  if (len_params > max_line_length) {
    display_error(0, (char_t *) "Truncated", FALSE);
    len_params = max_line_length;
  }
  /*
   * If we in XEDIT compatability, and no text is supplied, insert a blank line at
   * focus line and move the cursor to the first viewable column of the file area
   * This is what sos_lineadd() does so call that!
   */
  if (compatible_feel == COMPAT_XEDIT && len_params == 0) {
    rc = Sos_addline((char_t *) "");
  } else {
    insert_new_line(current_screen, CURRENT_VIEW, params, len_params, 1L, get_true_line(TRUE), TRUE, TRUE, TRUE, CURRENT_VIEW->display_low, TRUE, FALSE);
  }
  return (rc);
}

#define JOI_PARAMS  2

short Join(char_t *params) {
  char_t *word[JOI_PARAMS + 1];
  char_t strip[JOI_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  bool aligned = FALSE;
  bool cursorarg = FALSE;

  /*
   * Split parameters up...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, JOI_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    aligned = FALSE;
    cursorarg = FALSE;
  } else {
    if (equal((char_t *) "aligned", word[0], 2)) {
      aligned = TRUE;
      if (equal((char_t *) "cursor", word[1], 6)) {
        cursorarg = TRUE;
      } else {
        if (equal((char_t *) "column", word[1], 1)) {
          cursorarg = FALSE;
        } else {
          display_error(1, (char_t *) word[1], FALSE);
          return (RC_INVALID_ENVIRON);
        }
      }
    } else {
      if (equal((char_t *) "cursor", word[0], 6)) {
        aligned = FALSE;
        cursorarg = TRUE;
      } else {
        if (equal((char_t *) "column", word[0], 1)) {
          aligned = FALSE;
          cursorarg = FALSE;
        } else {
          display_error(1, (char_t *) word[0], FALSE);
          return (RC_INVALID_ENVIRON);
        }
      }
    }
  }
  rc = execute_split_join(SPLTJOIN_JOIN, aligned, cursorarg);
  return (rc);
}

