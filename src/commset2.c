/* COMMSET2.C - SET commands O-Z                                       */
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

/*--------------------------- global data -----------------------------*/
bool rexx_output = FALSE;

short Pagewrap(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &PAGEWRAPx, TRUE);
  return (rc);
}
short Parser(char_t * params) {
#define PAR_PARAMS  2
  char_t *word[PAR_PARAMS + 1];
  char_t strip[PAR_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK, errnum;
  int num;
  PARSER_DETAILS *curr = NULL, *old_parser = NULL;
  char_t *tldname = NULL, *buffer;
  bool redisplay_screen = FALSE, free_buffer;
  VIEW_DETAILS *curr_vd = vd_first;
  PARSER_MAPPING *curr_mapping = first_parser_mapping;

  /*
   * Validate parameters.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, PAR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If no arguments, error.
   */
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Try to find an existing parser with the same name as
   * supplied in first parameter.
   * If we find one and addition of the new parser works, then
   * this parser is purged.
   */
  old_parser = parserll_find(first_parser, word[0]);
  if (word[1][0] == '*') {
    /*
     * Use one of the builtin parsers
     */
    buffer = find_default_parser(word[1]);
    if (buffer == NULL) {
      display_error(199, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    tldname = word[1];
    num = strlen((char *) buffer);
    free_buffer = FALSE;
  } else {
    /*
     * Read the TLD file from disk.
     */
    if ((tldname = (char_t *) malloc((MAX_FILE_NAME + 1) * sizeof(char_t))) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    /*
     * Find a .tld file in MACROPATH
     */
    rc = get_valid_macro_file_name(word[1], tldname, (char_t *) ".tld", &errnum);
    if (rc != RC_OK) {
      display_error(errnum, word[1], FALSE);
      free(tldname);
      return (rc);
    }
    /*
     * We now have the .tld file in tldname.
     * Now read it into a chunk of memory and construct a parser from
     * it.
     */
    buffer = read_file_into_memory(tldname, &num);
    if (buffer == NULL) {
      free(tldname);
      return (RC_INVALID_OPERAND);
    }
    free_buffer = TRUE;
  }
  /*
   * We now have a chunk of memory pointing to a TLD.
   * Construct a parser from it.
   */
  rc = construct_parser(buffer, num, &curr, word[0], tldname);
  if (rc != RC_OK) {
    if (free_buffer) {
      free(buffer);
      free(tldname);
    }
    return (RC_INVALID_OPERAND);
  }
  /*
   * Now we have successfully constructed a parser, delete the parser
   * with the same name (if we found one)
   * But first, find all files in the ring that have this parser
   * specified, and change to the new parser.
   * Also change the pointer in the parser_mapping list.
   * Check the current file (and the file in the other screen) if they
   * had the old parser; then we need to redisplay the screen.
   */
  if (old_parser) {
    if (CURRENT_FILE->parser == old_parser)
      redisplay_screen = TRUE;
    if (display_screens > 1 && SCREEN_FILE(other_screen)->parser == old_parser)
      redisplay_screen = TRUE;
    while (curr_vd != (VIEW_DETAILS *) NULL) {
      if (curr_vd->file_for_view->parser == old_parser) {
        curr_vd->file_for_view->parser = curr;
      }
      curr_vd = curr_vd->next;
    }
    for (; curr_mapping != NULL; curr_mapping = curr_mapping->next) {
      if (curr_mapping->parser == old_parser)
        curr_mapping->parser = curr;
    }
    destroy_parser(old_parser);
    parserll_del(&first_parser, &last_parser, old_parser, DIRECTION_FORWARD);
  } else {
    if (rexx_support) {
      /*
       * If we have a Rexx interpreter, register an implied extract
       * function for the number of parsers we now have.
       */
      char_t tmp[20];
      int i;

      for (i = 0, old_parser = first_parser; old_parser != NULL; old_parser = old_parser->next, i++);
      sprintf((char *) tmp, "parser.%d", i);
      MyRexxRegisterFunctionExe(tmp);
    }
  }
  if (free_buffer) {
    free(buffer);
    free(tldname);
  }
  if (redisplay_screen) {
    display_screen(current_screen);
    if (display_screens > 1)
      display_screen((char_t) (other_screen));
  }
  return (rc);
}
short Pending(char_t * params) {
#define PEND_ON    1
#define PEND_OFF   2
#define PEND_BLOCK 3
#define PEN_PARAMS  3
  char_t *word[PEN_PARAMS + 1];
  char_t strip[PEN_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  LINE *curr = NULL;
  line_t true_line = 0L;
  short command = 0;

  /*
   * Validate parameters.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, PEN_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If no arguments, error.
   */
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If more than 2 arguments, error.
   */
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate first parameter...
   */
  if (equal((char_t *) "off", word[0], 3))
    command = PEND_OFF;
  else if (equal((char_t *) "on", word[0], 2))
    command = PEND_ON;
  else if (equal((char_t *) "block", word[0], 5))
    command = PEND_BLOCK;
  else {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  true_line = get_true_line(TRUE);
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  switch (command) {
      /*
       * PENDING ON and PENDING BLOCK...
       */
    case PEND_ON:
    case PEND_BLOCK:
      /*
       * The second argument must be present and <= PREFIX_WIDTH.
       */
      if (num_params != 2) {
        display_error(3, (char_t *) "", FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      }
      if (strlen((char *) word[1]) > CURRENT_VIEW->prefix_width) {
        display_error(1, word[1], FALSE);       /* different error ?? */
        rc = RC_INVALID_OPERAND;
        break;
      }
      /*
       * Copy the string into pre_rec and set its length.
       */
      pre_rec_len = strlen((char *) word[1]);
      strcpy((char *) pre_rec, (char *) word[1]);
      pre_rec_len = strlen((char *) word[1]);
      pre_rec[pre_rec_len] = ' ';
      pre_rec[CURRENT_VIEW->prefix_width] = '\0';
      curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
      if (command == PEND_BLOCK)
        add_prefix_command(current_screen, CURRENT_VIEW, curr, true_line, TRUE, TRUE);
      else
        add_prefix_command(current_screen, CURRENT_VIEW, curr, true_line, FALSE, TRUE);
      break;
      /*
       * PENDING OFF...
       */
    case PEND_OFF:
      if (num_params != 1) {
        display_error(2, (char_t *) "", FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      }
      curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
      /*
       * Don't delete the pending prefix command as SET PENDING OFF may have been
       * called by a prefix macro and that would stuff up the pending prefix command linked list.
       * Just set the pedning prefix command to processed and it will be cleaned up later.
       */
      if (curr->pre)
        curr->pre->ppc_processed = TRUE;
      memset(pre_rec, ' ', MAX_PREFIX_WIDTH);
      pre_rec_len = 0;
      break;
  }
  if (rc == RC_OK) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}
short Point(char_t * params) {
#define POI_PARAMS  2
  char_t *word[POI_PARAMS + 1];
  char_t strip[POI_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;

  /*
   * Validate parameters.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, POI_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params == 1) {
    /*
     * Turning on line name...
     */
    if (word[0][0] != '.' || !isalnum(word[0][1])) {
      display_error(18, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if ((rc = execute_set_point(current_screen, CURRENT_VIEW, word[0], get_true_line(TRUE), TRUE)) != RC_OK) {
      return (rc);
    }
  } else {
    /*
     * Turning off line name...
     */
    if (!equal((char_t *) "off", word[1], 3)) {
      display_error(1, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if ((rc = execute_set_point(current_screen, CURRENT_VIEW, word[0], get_true_line(TRUE), FALSE)) != RC_OK) {
      return (rc);
    }
  }
  return (RC_OK);
}
short Position(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->position_status, TRUE);
  return (rc);
}
short Prefix(char_t * params) {
#define PRE_PARAMS  5
  char_t *word[PRE_PARAMS + 1];
  char_t strip[PRE_PARAMS];
  char_t prefix = PREFIX_OFF;
  char_t previous_prefix = CURRENT_VIEW->prefix;
  unsigned short num_params = 0;
  short my_prefix_width = 0, my_prefix_gap = 0;
  short previous_prefix_width = 0;
  short previous_prefix_gap = 0;
  bool previous_gap_line = FALSE;
  bool my_gap_line = FALSE;
  short rc = RC_OK;

  /*
   * Set the default values for the prefix width and gap...
   */
  if (CURRENT_VIEW) {
    previous_prefix_width = CURRENT_VIEW->prefix_width;
    previous_prefix_gap = CURRENT_VIEW->prefix_gap;
    previous_gap_line = CURRENT_VIEW->prefix_gap_line;
  } else {
    previous_prefix_width = DEFAULT_PREFIX_WIDTH;
    previous_prefix_gap = DEFAULT_PREFIX_GAP;
    previous_gap_line = DEFAULT_PREFIX_GAP_LINE;
  }
  /*
   * Parse the parameters...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  num_params = param_split(params, word, PRE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (equal((char_t *) "on", word[0], 2)
      || equal((char_t *) "nulls", word[0], 1)) {
    if (num_params > 4) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (equal((char_t *) "left", word[1], 1))
      prefix = PREFIX_LEFT;
    else {
      if (equal((char_t *) "right", word[1], 1))
        prefix = PREFIX_RIGHT;
      else {
        if (num_params == 1)    /* no left/right, default to left */
          prefix = PREFIX_LEFT;
        else {
          display_error(1, word[1], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
    }
    if (equal((char_t *) "on", word[0], 2))
      CURRENT_VIEW->prefix = prefix | PREFIX_ON;
    else
      CURRENT_VIEW->prefix = prefix | PREFIX_NULLS;
    if (num_params > 2) {
      if (!valid_positive_integer(word[2])) {
        display_error(1, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      my_prefix_width = atoi((char *) word[2]);
      if (my_prefix_width > 20) {
        display_error(6, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (my_prefix_width < 2) {
        display_error(5, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (num_params == 3)
        my_prefix_gap = 0;
      else {
        if (!valid_positive_integer(word[3])) {
          display_error(1, word[3], FALSE);
          return (RC_INVALID_OPERAND);
        }
        my_prefix_gap = atoi((char *) word[3]);
        if (my_prefix_gap >= my_prefix_width) {
          display_error(6, word[3], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
      CURRENT_VIEW->prefix_width = my_prefix_width;
      CURRENT_VIEW->prefix_gap = my_prefix_gap;
    }
  } else if (equal((char_t *) "off", word[0], 3)) {
    if (num_params > 1) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (strcmp((char *) word[1], "") == 0) {
      CURRENT_VIEW->prefix = PREFIX_OFF;
      switch (CURRENT_VIEW->current_window) {
        case WINDOW_FILEAREA:
          break;
        case WINDOW_PREFIX:
          CURRENT_VIEW->current_window = WINDOW_FILEAREA;
          break;
        case WINDOW_COMMAND:
          CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
          break;
      }
    } else {
      display_error(2, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  } else if (equal((char_t *) "synonym", word[0], 1)) {
    if (num_params < 3) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params > 3) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (strlen((char *) word[1]) > MAX_PREFIX_WIDTH) {
      display_error(37, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    rc = add_prefix_synonym(word[1], word[2]);
    return (rc);
  } else if (equal((char_t *) "gap", word[0], 3)) {
    if (num_params < 2) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params > 3) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (!valid_positive_integer(word[1])) {
      display_error(1, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    my_prefix_gap = atoi((char *) word[1]);
    if (my_prefix_gap >= CURRENT_VIEW->prefix_width) {
      display_error(6, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params == 3) {
      if (equal((char_t *) "line", word[2], 4)) {
        my_gap_line = TRUE;
      } else {
        display_error(1, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    CURRENT_VIEW->prefix_gap = my_prefix_gap;
    CURRENT_VIEW->prefix_gap_line = my_gap_line;
  } else {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the new setting for PREFIX is identical with the previous values
   * don't do anything more.
   */
  if (previous_prefix == CURRENT_VIEW->prefix && previous_prefix_width == CURRENT_VIEW->prefix_width && previous_prefix_gap == CURRENT_VIEW->prefix_gap && previous_gap_line == CURRENT_VIEW->prefix_gap_line) {
    return (RC_OK);
  }
  /*
   * To get here something has changed, so rebuild the windows and
   * display the screen.
   */
  set_screen_defaults();
  if (set_up_windows(current_screen) != RC_OK) {
    return (RC_OK);
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}
short THEPrinter(char_t * params) {
#define PRI_PARAMS  3
  char_t *word[PRI_PARAMS + 1];
  char_t strip[PRI_PARAMS];
  unsigned short num_params = 0;

  /*
   * Parse the parameters...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, PRI_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (equal((char_t *) "option", word[0], 6)) {
    display_error(78, (char_t *) "OPTIONS not supported on this platform", FALSE);
    return (RC_INVALID_OPERAND);
  } else {
    if (num_params == 0) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params > 1) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    strcpy((char *) spooler_name, (char *) params);
  }
  return (RC_OK);
}
short Pscreen(char_t * params) {
#define PSC_PARAMS  3
  char_t *word[PSC_PARAMS + 1];
  char_t strip[PSC_PARAMS];
  unsigned short num_params = 0;
  int current_cols = COLS, current_lines = LINES;
  short rc = RC_OK;

  /*
   * Validate parameters.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, PSC_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate height parameter...
   */
  if ((current_lines = atoi((char *) word[0])) == 0) {
    display_error(4, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate width parameter...
   */
  if (num_params > 1) {
    if ((current_cols = atoi((char *) word[1])) == 0) {
      display_error(4, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  rc = THE_Resize(current_lines, current_cols);
  (void) THERefresh((char_t *) "");
  draw_cursor(TRUE);
  return (rc);
}
short THEReadonly(char_t * params) {
#define REO_PARAMS  2
  short rc = RC_OK;
  int readonly;
  int file_level = 0;
  char_t *word[REO_PARAMS + 1];
  char_t strip[REO_PARAMS];
  unsigned short num_params = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, REO_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "on", word[0], 2))
    readonly = READONLY_ON;
  else if (equal((char_t *) "off", word[0], 3))
    readonly = READONLY_OFF;
  else if (equal((char_t *) "force", word[0], 5))
    readonly = READONLY_FORCE;
  else {
    display_error(1, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params == 2) {
    if (equal((char_t *) "file", word[1], 1)) {
      file_level = 1;
    } else {
      display_error(1, (char_t *) word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  if (file_level) {
    CURRENT_FILE->readonly = readonly;
  } else {
    READONLYx = readonly;
  }
  return (rc);
}
short Reprofile(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &REPROFILEx, TRUE);
  return (rc);
}
short Reserved(char_t * params) {
#define RSR_PARAMS  2
  char_t *word[RSR_PARAMS + 1];
  char_t strip[RSR_PARAMS];
  unsigned short num_params = 0;
  short base = 0, off = 0;
  COLOUR_ATTR attr, save_attr;  /* initialisation done below */
  char_t *string = NULL;
  char_t *saveparams = NULL;
  short rc = RC_OK;
  unsigned short x = 0, y = 0;
  line_t new_focus_line = 0L;
  bool any_colours = FALSE;
  bool autoscroll = 0;
  RESERVED *curr = NULL;

  /*
   * Initialise attr and save_sttr...
   */
  memset((char *) &attr, 0, sizeof(COLOUR_ATTR));
  memset((char *) &save_attr, 0, sizeof(COLOUR_ATTR));
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_NONE;
  num_params = param_split(params, word, RSR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * First check for the special case of * OFF...
   */
  if (strcmp((char *) word[0], "*") == 0 && equal((char_t *) "off", word[1], 3))
    CURRENT_FILE->first_reserved = rll_free(CURRENT_FILE->first_reserved);
  else {
    /*
     * Look for AUTOSCroll
     */
    if (equal((char_t *) "autoscroll", word[0], 6)) {
      /*
       * We found AUTOSCroll, so copy the second word and split this as though it were
       * the original args
       */
      autoscroll = 1;
      saveparams = my_strdup(word[1]);
      num_params = param_split(saveparams, word, RSR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
      if (num_params < 1) {
        if (saveparams)
          free(saveparams);
        display_error(3, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    /*
     * Parse the position parameter...
     */
    rc = execute_set_row_position(word[0], &base, &off);
    if (rc != RC_OK) {
      if (saveparams)
        free(saveparams);
      return (rc);
    }
    if (equal((char_t *) "off", word[1], 3))
      rc = delete_reserved_line(base, off);
    else {
      /*
       * Parse the colour arguments (if any)...
       */
      if ((rc = parse_colours(word[1], &attr, &string, TRUE, &any_colours)) != RC_OK) {
        if (saveparams)
          free(saveparams);
        return (rc);
      }
      if (!any_colours)
        memcpy(&attr, CURRENT_FILE->attr + ATTR_RESERVED, sizeof(COLOUR_ATTR));
      /*
       * If the reserved row is the same row as CURLINE, return ERROR.
       */
      if (calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], FALSE)) {
        if (saveparams)
          free(saveparams);
        display_error(64, (char_t *) "- same line as CURLINE", FALSE);
        return (RC_INVALID_ENVIRON);
      }
      /*
       * If no colours were specified, use the default colour as set by any
       * SET COLOUR RESERVED... command.
       */
      if (memcmp(&attr, &save_attr, sizeof(COLOUR_ATTR)) == 0)
        curr = add_reserved_line(word[0], string, base, off, CURRENT_FILE->attr + ATTR_RESERVED, autoscroll);
      else
        curr = add_reserved_line(word[0], string, base, off, &attr, autoscroll);
      if (curr == NULL)
        rc = RC_OUT_OF_MEMORY;
    }
  }
  build_screen(current_screen);
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    if (curses_started)
      getyx(CURRENT_WINDOW, y, x);
    new_focus_line = get_focus_line_in_view(current_screen, CURRENT_VIEW->focus_line, y);
    if (new_focus_line != CURRENT_VIEW->focus_line) {
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
      CURRENT_VIEW->focus_line = new_focus_line;
      y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
      if (curses_started)
        wmove(CURRENT_WINDOW, y, x);
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
  }
  display_screen(current_screen);
  if (saveparams)
    free(saveparams);
  return (rc);
}
short Rexxhalt(char_t * params) {
#define REXH_PARAMS  2
  char_t *word[REXH_PARAMS + 1];
  char_t strip[REXH_PARAMS];
  unsigned short num_params = 0;
  int option, limit;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, REXH_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "command", word[0], 1))
    option = 1;
  else if (equal((char_t *) "function", word[0], 1))
    option = 2;
  else {
    display_error(1, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "off", word[1], 3)) {
    limit = 0;
  } else if (valid_positive_integer(word[1])) {
    limit = atoi((char *) word[1]);
  } else {
    display_error(1, (char_t *) word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  switch (option) {
    case 1:
      COMMANDCALLSx = limit;
      break;
    case 2:
      FUNCTIONCALLSx = limit;
      break;
  }
  return (RC_OK);
}
short Rexxoutput(char_t * params) {
#define REX_PARAMS  2
  char_t *word[REX_PARAMS + 1];
  char_t strip[REX_PARAMS];
  unsigned short num_params = 0;

  if (rexx_output) {
    display_error(0, (char_t *) "Error: Unable to alter REXXOUTPUT settings", FALSE);
    return (RC_INVALID_OPERAND);
  }
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, REX_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "file", word[0], 1))
    CAPREXXOUTx = TRUE;
  else {
    if (equal((char_t *) "display", word[0], 1))
      CAPREXXOUTx = FALSE;
    else {
      display_error(1, (char_t *) word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  if (!valid_positive_integer(word[1])) {
    display_error(4, (char_t *) word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CAPREXXMAXx = atol((char *) word[1]);
  return (RC_OK);
}
short Scale(char_t * params) {
#define SCA_PARAMS  2
  char_t *word[SCA_PARAMS + 1];
  char_t strip[SCA_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  short base = CURRENT_VIEW->scale_base;
  short off = CURRENT_VIEW->scale_off;
  bool scalests = FALSE;
  unsigned short x = 0, y = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, SCA_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  rc = execute_set_on_off(word[0], &scalests, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }
  /*
   * Parse the position parameter...
   */
  if (num_params > 1) {
    rc = execute_set_row_position(word[1], &base, &off);
    if (rc != RC_OK) {
      return (rc);
    }
  }
  /*
   * If the SCALE row is the same row as CURLINE and it is being turned
   * on, return ERROR.
   */
  if (calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE)
      && scalests) {
    display_error(64, (char_t *) "- same line as CURLINE", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  CURRENT_VIEW->scale_base = (char_t) base;
  CURRENT_VIEW->scale_off = off;
  CURRENT_VIEW->scale_on = scalests;
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  build_screen(current_screen);
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    if (curses_started)
      getyx(CURRENT_WINDOW, y, x);
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen, CURRENT_VIEW->focus_line, y);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (curses_started)
      wmove(CURRENT_WINDOW, y, x);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  display_screen(current_screen);
  return (RC_OK);
}
short Scope(char_t * params) {
  short rc = RC_OK;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "all", params, 1))
    CURRENT_VIEW->scope_all = TRUE;
  else if (equal((char_t *) "display", params, 1))
    CURRENT_VIEW->scope_all = FALSE;
  else {
    display_error(1, params, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}
short THEScreen(char_t * params) {
#define SCR_PARAMS  3
#define SCR_MIN_LINES 3
#define SCR_MIN_COLS 10
  char_t *word[SCR_PARAMS + 1];
  char_t strip[SCR_PARAMS];
  register short i = 0;
  unsigned short num_params = 0, num_views = 0;
  char_t save_display_screens = 0;
  bool save_horizontal = FALSE;
  int horiz = (-1);
  VIEW_DETAILS *save_current_view = NULL;
  short save_screen_rows[MAX_SCREENS];
  short save_screen_cols[MAX_SCREENS];
  char_t save_current_screen = 0;
  short rc = RC_OK;
  int size1 = 0, size2 = 0, offset = 0;
  int width1 = 0, width2 = 0;
  bool diff_sizes = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, SCR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  for (i = 0; i < MAX_SCREENS; i++) {
    save_screen_rows[i] = screen_rows[i];
    save_screen_cols[i] = screen_cols[i];
  }
  if (equal((char_t *) "size", word[0], 1)) {
    if (equal((char_t *) "*", word[1], 1))
      size1 = -1;
    else {
      if (!valid_positive_integer(word[1])) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      size1 = atoi((char *) word[1]);
    }
    num_views = 1;
    if (num_params == 3) {
      if (equal((char_t *) "*", word[2], 1))
        size2 = -1;
      else {
        if (!valid_positive_integer(word[2])) {
          display_error(1, word[2], FALSE);
          return (RC_INVALID_OPERAND);
        }
        size2 = atoi((char *) word[2]);
      }
      num_views = 2;
    }
    if (size1 == -1 && size2 == -1) {
      display_error(1, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (STATUSLINEON())
      offset = 1;
    else
      offset = 0;
    if (size1 == -1) {
      size1 = (terminal_lines - offset) - size2;
      if (size1 < SCR_MIN_LINES) {
        display_error(6, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    if (size2 == -1) {
      size2 = (terminal_lines - offset) - size1;
      if (size2 < SCR_MIN_LINES) {
        display_error(6, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    if (size1 < SCR_MIN_LINES) {
      display_error(5, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (size2 < SCR_MIN_LINES) {
      display_error(5, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (size1 + size2 > terminal_lines - offset) {
      display_error(6, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (size1 + size2 < terminal_lines - offset) {
      display_error(5, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    horiz = TRUE;
  } else if (equal((char_t *) "width", word[0], 1)) {
    if (equal((char_t *) "*", word[1], 1))
      width1 = -1;
    else {
      if (!valid_positive_integer(word[1])) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      width1 = atoi((char *) word[1]);
    }
    num_views = 1;
    if (num_params == 3) {
      if (equal((char_t *) "*", word[2], 1))
        width2 = -1;
      else {
        if (!valid_positive_integer(word[2])) {
          display_error(1, word[2], FALSE);
          return (RC_INVALID_OPERAND);
        }
        width2 = atoi((char *) word[2]);
      }
      num_views = 2;
    }
    if (width1 == -1 && width2 == -1) {
      display_error(1, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (width1 == -1) {
      width1 = terminal_cols - width2;
      if (width1 < SCR_MIN_COLS) {
        display_error(6, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    if (width2 == -1) {
      width2 = terminal_cols - width1;
      if (width2 < SCR_MIN_COLS) {
        display_error(6, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    if (width1 < SCR_MIN_COLS) {
      display_error(5, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (width2 < SCR_MIN_COLS) {
      display_error(5, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (width1 + width2 > terminal_cols) {
      display_error(6, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (width1 + width2 < terminal_cols) {
      display_error(5, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
    horiz = FALSE;
  } else {
    if (!valid_positive_integer(word[0])) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if ((num_views = atoi((char *) word[0])) > MAX_SCREENS) {
      display_error(6, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_views == 1)
      horiz = TRUE;
    else {
      if (equal((char_t *) "horizontal", word[1], 1)
          || strcmp((char *) word[1], "") == 0)
        horiz = TRUE;
      if (equal((char_t *) "vertical", word[1], 1))
        horiz = FALSE;
      if (horiz == (-1)) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
  }
  screen_rows[0] = size1;
  screen_rows[1] = size2;
  screen_cols[0] = width1;
  screen_cols[1] = width2;
  /*
   * Set the global variable display_screens to indicate the number of
   * screens currently displayed and the orientation of those screens
   * Save the old values first so we know how the screens were oriented.
   */
  save_display_screens = display_screens;
  save_horizontal = horizontal;

  display_screens = (char_t) num_views;
  horizontal = (bool) horiz;
  /*
   * If there is no change to the screens, exit.
   */
  for (i = 0; i < MAX_SCREENS; i++) {
    if (screen_rows[i] != save_screen_rows[i]
        || screen_cols[i] != save_screen_cols[i])
      diff_sizes = TRUE;
  }
  if (display_screens == save_display_screens && horizontal == save_horizontal && diff_sizes == FALSE) {
    return (RC_OK);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Save the screen coordinates for later retrieval.
   */
  if (curses_started) {
    getyx(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
    if (CURRENT_WINDOW_PREFIX != NULL)
      getyx(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
  }
  /*
   * Set up the screen views correctly so that when we create the new
   * window for each screen, the screens have a view to point to...
   */
  switch (display_screens) {
    case 1:                    /* now have only 1 screen (from 2) */
      save_current_view = CURRENT_VIEW;
      if (CURRENT_SCREEN.screen_view->file_for_view == OTHER_SCREEN.screen_view->file_for_view) {
        CURRENT_VIEW = OTHER_SCREEN.screen_view;
        CURRENT_FILE->file_views--;
        free_a_view();
        CURRENT_VIEW = save_current_view;
      }
      if (divider != (WINDOW *) NULL) {
        delwin(divider);
        divider = NULL;
      }
      current_screen = 0;
      CURRENT_SCREEN.screen_view = CURRENT_VIEW = save_current_view;
      OTHER_SCREEN.screen_view = NULL;
      break;
    case 2:                    /* now have 2 screens (from 1) */
      save_current_view = CURRENT_VIEW;
      save_current_screen = current_screen;
      current_screen = other_screen;    /* make other screen current */
      if (number_of_views == 1) {
        if ((rc = defaults_for_other_files(PREVIOUS_VIEW)) != RC_OK) {
          return (rc);
        }
        CURRENT_FILE = save_current_view->file_for_view;
        CURRENT_FILE->file_views++;
      } else {
        if (NEXT_VIEW == (VIEW_DETAILS *) NULL)
          CURRENT_VIEW = vd_first;
        else
          CURRENT_VIEW = NEXT_VIEW;
      }
      CURRENT_SCREEN.screen_view = CURRENT_VIEW;
      CURRENT_VIEW = save_current_view;
      current_screen = save_current_screen;
      break;
  }

  set_screen_defaults();
  if (curses_started) {
    for (i = 0; i < display_screens; i++) {
      if ((rc = set_up_windows(i)) != RC_OK) {
        return (rc);
      }
      if (screen[i].screen_view->prefix) {
        wmove(screen[i].win[WINDOW_PREFIX], screen[i].screen_view->y[WINDOW_PREFIX], screen[i].screen_view->x[WINDOW_PREFIX]);
      }
      wmove(screen[i].win[WINDOW_FILEAREA], screen[i].screen_view->y[WINDOW_FILEAREA], screen[i].screen_view->x[WINDOW_FILEAREA]);
    }
  }

  if (!horizontal && display_screens > 1 && curses_started) {
/*    redraw_window(divider);*/
    touchwin(divider);
    wnoutrefresh(divider);
  }

  if (display_screens > 1) {
    pre_process_line(OTHER_VIEW, OTHER_VIEW->focus_line, (LINE *) NULL);
    (void) prepare_view((char_t) (other_screen));
    display_screen((char_t) (other_screen));
    show_heading((char_t) (other_screen));
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  (void) prepare_view(current_screen);
  display_screen(current_screen);

  return (RC_OK);
}
short Select(char_t * params) {
#define SEL_PARAMS  2
  short rc = RC_OK;
  char_t *word[SEL_PARAMS + 1];
  char_t strip[SEL_PARAMS];
  line_t true_line = 0L;
  short num_params = 0;
  bool relative = FALSE;
  short off = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, SEL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the first parameter...
   *
   * Determine if the selection level is relative to the existing value
   * or is an absolute value.
   */
  if (*(word[0]) == '-' || *(word[0]) == '+')
    relative = TRUE;
  else
    relative = FALSE;
  /*
   * Get the value, positive or negative.
   */
  if (!valid_integer(word[0])) {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  off = atoi((char *) word[0]);
  /*
   * Parse the next parameter...
   */
  true_line = get_true_line(TRUE);
  /*
   * If no target specified, just apply to the current line...
   */
  if (num_params == 1)
    word[1] = (char_t *) "+1";
  rc = execute_select(word[1], relative, off);
  /*
   * If we are on the command line and the result of this statement means
   * that the current line is no longer in scope, we need to make the
   * current line and possibly the focus line the next line in scope.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, NULL, true_line, DIRECTION_FORWARD);
    build_screen(current_screen);
    if (!line_in_view(current_screen, CURRENT_VIEW->focus_line)) {
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  /*
   * If the same file is in the other screen, refresh it
   */
  adjust_other_screen_shadow_lines();
  return (rc);
}
short Shadow(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->shadow, TRUE);
  if (rc == RC_OK) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}
short Slk(char_t * params) {
#define SLK_PARAMS  2
  char_t *word[SEL_PARAMS + 1];
  char_t strip[SEL_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  short key = 0;

  if (max_slk_labels) {
    strip[0] = STRIP_BOTH;
    strip[1] = STRIP_NONE;
    num_params = param_split(params, word, SLK_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
    if (num_params < 1) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params == 1 && equal((char_t *) "off", word[0], 3)) {
      SLKx = FALSE;
      slk_clear();
      return (RC_OK);
    }
    if (num_params == 1 && equal((char_t *) "on", word[0], 2)) {
      SLKx = TRUE;
      slk_restore();
      return (RC_OK);
    }
    key = atoi((char *) word[0]);
    if (key == 0) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (key > max_slk_labels) {
      display_error(6, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    SLKx = TRUE;
    slk_restore();
    slk_set(key, (char *) word[1], 1);
    slk_noutrefresh();
  } else {
    display_error(82, (char_t *) "- use -k command line switch to enable", FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}
short Span(char_t * params) {
  short rc = RC_OK;

  display_error(0, (char_t *) "This command not yet implemented", FALSE);
  return (rc);
}
short Spill(char_t * params) {
  short rc = RC_OK;

  display_error(0, (char_t *) "This command not yet implemented", FALSE);
  return (rc);
}
short Statopt(char_t * params) {
#define OPT_PARAMS  5
  char_t *word[OPT_PARAMS + 1];
  char_t strip[OPT_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  LINE *curr = NULL;
  int tail = 0, len = 0, col = 0, itemno = 0;
  char_t item_type = 0;

  /*
   * Validate parameters.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_LEADING;
  num_params = param_split(params, word, OPT_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If no arguments, error.
   */
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate first parameter...
   */
  if (equal((char_t *) "off", word[0], 3)) {
    if (num_params > 2) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (strcmp((char *) word[1], "*") == 0)
      first_option = last_option = lll_free(first_option);
    else {
      curr = lll_locate(first_option, make_upper(word[1]));
      if (curr == NULL) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      delete_LINE(&first_option, &last_option, curr, DIRECTION_FORWARD, TRUE);
    }
  } else {
    if (equal((char_t *) "on", word[0], 2)) {
      if (num_params < 3) {
        display_error(3, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      }
      tail = split_function_name(word[1], &len);
      if (tail == (-1)
          || tail > MAX_VARIABLES_RETURNED) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if ((itemno = find_query_item(word[1], len, &item_type)) == (-1)
          || !(item_type & QUERY_EXTRACT)) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (!valid_positive_integer(word[2])) {
        display_error(4, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      len = 0;
      col = atoi((char *) word[2]);
      if (col > COLS) {
        display_error(6, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (col <= STATAREA_OFFSET) {
        display_error(5, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (num_params > 3) {
        if (!valid_positive_integer(word[3])) {
          display_error(4, word[3], FALSE);
          return (RC_INVALID_OPERAND);
        }
        len = atoi((char *) word[3]);
      }
      curr = lll_locate(first_option, make_upper(word[1]));
      if (curr != NULL)
        lll_del(&first_option, &last_option, curr, DIRECTION_FORWARD);
      curr = lll_add(first_option, last_option, sizeof(LINE));
      if (curr == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      /*
       * Use the name member to store the option name so that lll_locate()
       * can find it.
       */
      curr->name = (char_t *) malloc((strlen((char *) word[1]) + 1) * sizeof(char_t));
      if (curr->name == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) curr->name, (char *) make_upper(word[1]));

      curr->length = itemno;
      curr->select = (select_t) col - STATAREA_OFFSET - 1;
      curr->save_select = (select_t) len;
      curr->pre = NULL;
      if (num_params > 4) {
        curr->line = (char_t *) malloc((strlen((char *) word[4]) + 1) * sizeof(char_t));
        if (curr->line == NULL) {
          display_error(30, (char_t *) "", FALSE);
          return (RC_OUT_OF_MEMORY);
        }
        strcpy((char *) curr->line, (char *) word[4]);
      } else
        curr->line = NULL;
      curr->flags.new_flag = tail;
      curr->flags.changed_flag = FALSE;
      curr->flags.tag_flag = FALSE;
      curr->flags.save_tag_flag = FALSE;
      if (first_option == NULL)
        first_option = curr;
      last_option = curr;
    } else {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  show_statarea();
  return (rc);
}
short Statusline(char_t * params) {
  char_t stat_place = '?';
  short rc = RC_OK;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "top", params, 1))
    stat_place = 'T';
  else if (equal((char_t *) "bottom", params, 1))
    stat_place = 'B';
  else if (equal((char_t *) "off", params, 2)) {
    stat_place = 'O';
  } else if (equal((char_t *) "gui", params, 3)) {
    stat_place = 'G';
  } else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the setting supplied is the same as the current setting, just
   * return without doing anything.
   */
  if (stat_place == STATUSLINEx) {
    return (RC_OK);
  }
  /*
   * Now we need to move the windows around.
   */
  STATUSLINEx = stat_place;
  /*
   * Redefine the screen sizes and recreate statusline window...
   */
  set_screen_defaults();
  if (curses_started) {
    if (create_statusline_window() != RC_OK) {
      return (rc);
    }
  }
  /*
   * Recreate windows for other screen (if there is one)...
   */
  if (display_screens > 1) {
    if (curses_started) {
      if (set_up_windows((short) ((current_screen == 0) ? 1 : 0)) != RC_OK) {
        return (rc);
      }
      if (!horizontal) {
/*          redraw_window(divider); */
        touchwin(divider);
        wnoutrefresh(divider);
      }
    }
    build_screen((char_t) (other_screen));
    display_screen((char_t) (other_screen));
  }
  /*
   * Recreate windows for the current screen...
   */
  if (curses_started) {
    if (set_up_windows(current_screen) != RC_OK) {
      return (rc);
    }
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}
short Stay(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->stay, TRUE);
  return (rc);
}
short Synonym(char_t * params) {
#define SYN_PARAMS  4
  char_t *word[SYN_PARAMS + 1];
  char_t strip[SYN_PARAMS];
  char_t *rem, *newname, *def;
  char_t linend;
  unsigned short num_params = 0;
  unsigned short exp_num_params = 0;
  short rc = RC_OK, min_abbrev;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "on", params, 2)) {
    CURRENT_VIEW->synonym = TRUE;
    return (rc);
  }
  if (equal((char_t *) "off", params, 3)) {
    CURRENT_VIEW->synonym = FALSE;
    return (rc);
  }
  /*
   * We can now process commands of the second format
   */
  if (strlen((char *) params) > 6 && memcmpi(params, (char_t *) "LINEND ", 7) == 0) {
    strip[0] = STRIP_BOTH;
    strip[1] = STRIP_BOTH;
    strip[2] = STRIP_BOTH;
    strip[3] = STRIP_LEADING;
    exp_num_params = 4;
  } else {
    strip[0] = STRIP_BOTH;
    strip[1] = STRIP_LEADING;
    exp_num_params = 2;
  }
  num_params = param_split(params, word, exp_num_params, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < exp_num_params) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (exp_num_params == 4) {
    if (strlen((char *) word[1]) != 1) {
      display_error(1, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    linend = word[1][0];
    newname = my_strdup(word[2]);
    rem = my_strdup(word[3]);
  } else {
    linend = 0;
    newname = my_strdup(word[0]);
    rem = my_strdup(word[1]);
  }
  /*
   * We have parsed the line to determine the optional LINEND
   * character, and the newname. We now have to determine if an
   * abbreviation length is specified...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_LEADING;
  num_params = param_split(rem, word, 2, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    free(newname);
    free(rem);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the first word (word[0]) is numeric, then this is the
   * abbreviation length
   */
  if (valid_positive_integer(word[0])) {
    if (num_params == 1) {
      display_error(3, (char_t *) "", FALSE);
      free(newname);
      free(rem);
      return (RC_INVALID_OPERAND);
    }
    min_abbrev = atoi((char *) word[0]);
    if (min_abbrev > strlen((char *) newname)) {
      display_error(6, (char_t *) "", FALSE);
      free(newname);
      free(rem);
      return (RC_INVALID_OPERAND);
    }
    def = word[1];
  } else {
    min_abbrev = strlen((char *) newname);
    def = rem;
  }
  /*
   * Do not allow the command COMMAND to be synonymed.
   */
  if (strcasecmp((char *) newname, (char *) "COMMAND") == 0) {
    display_error(1, newname, FALSE);
    free(newname);
    free(rem);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Determine if the first word of the supplied command is REXX
   * (either case)...
   */
  if (strlen((char *) def) > 5 && memcmpi(def, (char_t *) "REXX ", 5) == 0) {
    rc = add_define(&first_synonym, &last_synonym, min_abbrev, def + 5, TRUE, newname, linend);
  } else
    rc = add_define(&first_synonym, &last_synonym, min_abbrev, def, FALSE, newname, linend);
  free(newname);
  free(rem);
  return (rc);
}
short Tabkey(char_t * params) {
#define TKY_PARAMS  3
  char_t *word[TKY_PARAMS + 1];
  char_t strip[TKY_PARAMS];
  unsigned short num_params = 0;
  char_t tabo = tabkey_overwrite;
  char_t tabi = tabkey_insert;
  short rc = RC_INVALID_OPERAND;

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, TKY_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
      /*
       * Too few parameters, error.
       */
    case 0:
    case 1:
      display_error(3, (char_t *) "", FALSE);
      break;
      /*
       * 2 parameters, validate them...
       */
    case 2:
      /*
       * Validate first parameter; overwrite mode setting...
       */
      if (equal((char_t *) "character", word[0], 1))
        tabo = 'C';
      else if (equal((char_t *) "tab", word[0], 1))
        tabo = 'T';
      else if (equal((char_t *) EQUIVCHARstr, word[0], 1));
      else {
        /*
         * If not a valid first parameter, display an error and exit.
         */
        display_error(1, word[0], FALSE);
        break;
      }
      /*
       * Validate second parameter; insert mode setting...
       */
      if (equal((char_t *) "character", word[1], 1))
        tabi = 'C';
      else if (equal((char_t *) "tab", word[1], 1))
        tabi = 'T';
      else if (equal((char_t *) EQUIVCHARstr, word[1], 1));
      else {
        /*
         * If not a valid second parameter, display an error and exit.
         */
        display_error(1, word[1], FALSE);
        break;
      }
      rc = RC_OK;
      break;
      /*
       * Too many parameters...
       */
    default:
      display_error(2, (char_t *) "", FALSE);
      break;
  }
  /*
   * If valid parameters, change the settings...
   */
  if (rc == RC_OK) {
    tabkey_insert = tabi;
    tabkey_overwrite = tabo;
  }
  return (RC_OK);
}
short Tabline(char_t * params) {
#define TBL_PARAMS  2
  char_t *word[TBL_PARAMS + 1];
  char_t strip[TBL_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  short base = CURRENT_VIEW->tab_base;
  short off = CURRENT_VIEW->tab_off;
  bool tabsts = FALSE;
  unsigned short x = 0, y = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, TBL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  rc = execute_set_on_off(word[0], &tabsts, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }
  /*
   * Parse the position parameter...
   */
  if (num_params > 1) {
    rc = execute_set_row_position(word[1], &base, &off);
    if (rc != RC_OK) {
      return (rc);
    }
  }
  /*
   * If the TABL  row is the same row as CURLINE and it is being turned
   * on, return ERROR.
   */
  if (calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE)
      && tabsts) {
    display_error(64, (char_t *) "- same line as CURLINE", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  CURRENT_VIEW->tab_base = (char_t) base;
  CURRENT_VIEW->tab_off = off;
  CURRENT_VIEW->tab_on = tabsts;
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  build_screen(current_screen);
  if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
    if (curses_started)
      getyx(CURRENT_WINDOW, y, x);
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen, CURRENT_VIEW->focus_line, y);
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (curses_started)
      wmove(CURRENT_WINDOW, y, x);
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  }
  display_screen(current_screen);
  return (RC_OK);
}
short Tabs(char_t * params) {
#define TABS_PARAMS  MAX_NUMTABS
  char_t *word[TABS_PARAMS + 1];
  char_t strip[TABS_PARAMS];
  length_t stops[TABS_PARAMS];
  register short i = 0;
  unsigned short num_params = 0;
  length_t tabinc = 0;

  for (i = 0; i < TABS_PARAMS; i++)
    strip[i] = STRIP_BOTH;
  num_params = param_split(params, word, TABS_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If the INCR option is specified...
   */
  if (equal((char_t *) "incr", word[0], 2)) {
    if (num_params != 2) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (!valid_positive_integer(word[1])) {
      display_error(4, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    tabinc = atol((char *) word[1]);
    if (tabinc < 1) {
      display_error(5, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (tabinc > 32) {
      display_error(6, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    for (i = 0; i < MAX_NUMTABS; i++)
      CURRENT_VIEW->tabs[i] = 1 + (tabinc * i);
    CURRENT_VIEW->numtabs = MAX_NUMTABS;
    CURRENT_VIEW->tabsinc = tabinc;
  } else if (equal((char_t *) "off", word[0], 3)) {
    /*
     * If the OFF option is specified...
     */
    if (num_params != 1) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    for (i = 0; i < MAX_NUMTABS; i++)
      CURRENT_VIEW->tabs[i] = 0;
    CURRENT_VIEW->numtabs = 0;
    CURRENT_VIEW->tabsinc = 0;
  } else {
    /*
     * ... individual TAB stop settings.
     */
    if (num_params > MAX_NUMTABS) {
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params == 0) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    for (i = 0; i < num_params; i++) {
      if (!valid_positive_integer(word[i])) {
        display_error(4, word[i], FALSE);
        return (RC_INVALID_OPERAND);
      }
      tabinc = atol((char *) word[i]);
      if (i > 0) {
        if (stops[i - 1] >= tabinc) {
          display_error(5, word[i], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
      stops[i] = tabinc;
    }
    CURRENT_VIEW->numtabs = num_params;
    for (i = 0; i < num_params; i++) {
      CURRENT_VIEW->tabs[i] = stops[i];
    }
    CURRENT_VIEW->tabsinc = 0;
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}
short Tabsin(char_t * params) {
/*
 * The settings for TABSIN is a global value, despite it supposedly
 * being a file level value.
 */
#define TABI_PARAMS  3
  char_t *word[TABI_PARAMS + 1];
  char_t strip[TABI_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_INVALID_OPERAND;
  char_t tabsts = TABI_ONx;
  char_t tabn = TABI_Nx;
  bool save_scope = FALSE;
  bool save_stay = FALSE;

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, TABI_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 0:
      /*
       * Too few parameters, error.
       */
      display_error(3, (char_t *) "", FALSE);
      break;
    case 1:
    case 2:
      /*
       * 1 or 2 parameters, validate them...
       *
       * Validate first parameter; on or off...
       */
      if (equal((char_t *) "on", word[0], 2)) {
        tabsts = TRUE;
        rc = RC_OK;
      }
      if (equal((char_t *) "off", word[0], 3)) {
        tabsts = FALSE;
        rc = RC_OK;
      }
      if (rc != RC_OK) {
        /*
         * If not a valid first parameter, display an error and exit.
         */
        display_error(1, word[0], FALSE);
        break;
      }
      /*
       * For 1 parameter, don't check any more.
       */
      if (num_params == 1)
        break;
      /*
       * Validate second parameter; number of spaces for a TAB...
       */
      if (!valid_positive_integer(word[1])) {
        display_error(4, word[1], FALSE);
        break;
      }
      tabn = (char_t) atoi((char *) word[1]);
      /*
       * tabn must be between 1 and 32...
       */
      if (tabn < 1) {
        display_error(5, word[1], FALSE);
        break;
      }
      if (tabn > 32) {
        display_error(6, word[1], FALSE);
        break;
      }
      rc = RC_OK;
      break;
    default:
      /*
       * Too many parameters...
       */
      display_error(2, (char_t *) "", FALSE);
      break;
  }
  /*
   * If valid parameters, change the settings...
   */
  if (rc == RC_OK) {
    TABI_ONx = tabsts;
    TABI_Nx = tabn;
    /*
     * If this command is issued from the profile file, we need to run
     * EXPAND ALL on it, as we have already read in the file.
     * We need to save the current setting of scope so that it can be
     * changed to ALL so that every line will be expanded.
     * Of course if TABSIN OFF was set we DON'T run EXPAND ALL :-)
     */
    if (in_profile && tabsts) {
      save_stay = CURRENT_VIEW->stay;
      CURRENT_VIEW->stay = TRUE;
      save_scope = CURRENT_VIEW->scope_all;
      rc = execute_expand_compress((char_t *) "ALL", TRUE, FALSE, FALSE, FALSE);
      CURRENT_VIEW->scope_all = save_scope;
      CURRENT_VIEW->stay = save_stay;
    }
  }
  return (rc);
}
short Tabsout(char_t * params) {
#define TABO_PARAMS  3
  char_t *word[TABO_PARAMS + 1];
  char_t strip[TABO_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_INVALID_OPERAND;
  bool tabsts = CURRENT_FILE->tabsout_on;
  char_t tabn = CURRENT_FILE->tabsout_num;

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, TABO_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 0:
      /*
       * Too few parameters, error.
       */
      display_error(3, (char_t *) "", FALSE);
      break;
    case 1:
    case 2:
      /*
       * 1 or 2 parameters, validate them...
       *
       * Validate first parameter; on or off...
       */
      if (equal((char_t *) "on", word[0], 2)) {
        tabsts = TRUE;
        rc = RC_OK;
      }
      if (equal((char_t *) "off", word[0], 3)) {
        tabsts = FALSE;
        rc = RC_OK;
      }
      if (rc != RC_OK) {
        /*
         * If not a valid first parameter, display an error and exit.
         */
        display_error(1, word[0], FALSE);
        break;
      }
      /*
       * For 1 parameter, don't check any more.
       */
      if (num_params == 1)
        break;
      /*
       * Validate second parameter; number of spaces for a TAB...
       */
      if (!valid_positive_integer(word[1])) {
        display_error(4, word[1], FALSE);
        break;
      }
      tabn = (char_t) atoi((char *) word[1]);
      /*
       * tabn must be between 1 and 32...
       */
      if (tabn < 1) {
        display_error(5, word[1], FALSE);
        break;
      }
      if (tabn > 32) {
        display_error(6, word[1], FALSE);
        break;
      }
      rc = RC_OK;
      break;
    default:
      /*
       * Too many parameters...
       */
      display_error(2, (char_t *) "", FALSE);
      break;
  }
  /*
   * If valid parameters, change the settings...
   */
  if (rc == RC_OK) {
    CURRENT_FILE->tabsout_on = tabsts;
    CURRENT_FILE->tabsout_num = tabn;
  }
  return (RC_OK);
}
short Targetsave(char_t * params) {
#define TARSAV_PARAMS  10
  char_t *word[TARSAV_PARAMS + 1];
  char_t strip[TARSAV_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  int targetsave = 0, i;

  /*
   * Validate the parameters that have been supplied.
   */
  for (i = 0; i < TARSAV_PARAMS; i++) {
    strip[i] = STRIP_BOTH;
  }
  num_params = param_split(params, word, TARSAV_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    rc = RC_INVALID_OPERAND;
  } else if (equal((char_t *) "all", word[0], 3)
             && num_params == 1) {
    targetsave = TARGET_ALL;
  } else if (equal((char_t *) "none", word[0], 4)
             && num_params == 1) {
    targetsave = TARGET_UNFOUND;
  } else {
    for (i = 0; i < num_params; i++) {
      if (equal((char_t *) "string", word[i], 6))
        targetsave |= TARGET_STRING;
      else if (equal((char_t *) "regexp", word[i], 6))
        targetsave |= TARGET_REGEXP;
      else if (equal((char_t *) "absolute", word[i], 8))
        targetsave |= TARGET_ABSOLUTE;
      else if (equal((char_t *) "relative", word[i], 8))
        targetsave |= TARGET_RELATIVE;
      else if (equal((char_t *) "point", word[i], 5))
        targetsave |= TARGET_POINT;
      else if (equal((char_t *) "blank", word[i], 5))
        targetsave |= TARGET_BLANK;
      else {
        display_error(1, word[i], FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      }
    }
  }
  if (rc == RC_OK)
    TARGETSAVEx = targetsave;
  return (rc);
}
short THighlight(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->thighlight_on, TRUE);
  return (rc);
}
short Timecheck(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_FILE->timecheck, TRUE);
  return (rc);
}
short Tofeof(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->tofeof, TRUE);
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}
short Trailing(char_t * params) {
  short rc = RC_OK;
  LINE *curr;
  length_t len;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "on", params, 2))
    CURRENT_FILE->trailing = TRAILING_ON;
  else if (equal((char_t *) "off", params, 3))
    CURRENT_FILE->trailing = TRAILING_OFF;
  else if (equal((char_t *) "single", params, 6))
    CURRENT_FILE->trailing = TRAILING_SINGLE;
  else if (equal((char_t *) "empty", params, 5))
    CURRENT_FILE->trailing = TRAILING_EMPTY;
  else if (equal((char_t *) "remove", params, 6))
    CURRENT_FILE->trailing = TRAILING_REMOVE;
  else {
    display_error(1, params, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  /*
   * If TRAILING is OFF, go and remove all trailing blanks...
   * Only if we didn't start THE with -u switch
   */
  if (CURRENT_FILE->trailing == TRAILING_OFF && display_length == 0) {
    curr = CURRENT_FILE->first_line->next;
    if (curr != NULL) {         /* not on EOF? */
      while (curr->next != NULL) {
        len = 1 + memrevne(curr->line, ' ', curr->length);
        curr->length = len;
        curr->line[len] = '\0';
        curr = curr->next;
      }
    }
  }
  return (rc);
}
short Trunc(char_t * params) {
  short rc = RC_OK;

  display_error(0, (char_t *) "This command not yet implemented", FALSE);
  return (rc);
}
short THETypeahead(char_t * params) {
  short rc = RC_OK;
  bool setting = FALSE;

  if (curses_started) {
    rc = execute_set_on_off(params, &setting, TRUE);
    if (rc == RC_OK) {
      if (setting) {
        typeahead(fileno(stdin));
        TYPEAHEADx = TRUE;
      } else {
        typeahead(-1);
        TYPEAHEADx = FALSE;
      }
    }
  }

  return (rc);
}
short Undoing(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_FILE->undoing, TRUE);
  return (rc);
}
short Untaa(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &UNTAAx, TRUE);
  return (rc);
}
short Verify(char_t * params) {
#define VER_PARAMS  3
  char_t *word[VER_PARAMS + 1];
  char_t strip[VER_PARAMS];
  char_t buffer[100];
  unsigned short num_params = 0;
  bool end_max = FALSE;
  length_t col1 = 0L, col2 = 0L;
  short rc;

  /*
   * Validate the parameters that have been supplied. One or two
   * parameters can be supplied. The first parameter MUST be a positive
   * integer. The second can be a positive integer or '*'. If no second
   * parameter is supplied, '*' is assumed. The second parameter MUST be
   * >= first parameter. '*' is regarded as the biggest number and is
   * literally max_line_length.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, VER_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  } else if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * First parameter must be positive integer or EQUIVCHAR
   */
  if (equal(EQUIVCHARstr, word[0], 1)) {
    col1 = CURRENT_VIEW->verify_start;
  } else {
    if ((rc = valid_positive_integer_against_maximum(word[0], MAX_WIDTH_NUM)) != 0) {
      if (rc == 4)
        sprintf((char *) buffer, "%s", word[0]);
      else
        sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
      display_error(rc, buffer, FALSE);
      return (RC_INVALID_OPERAND);
    }
    col1 = atol((char *) word[0]);
    if (col1 == 0) {
      display_error(5, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Second parameter (if present) must be positive integer or EQUIVCHAR
   */
  if (num_params == 1) {
    col2 = max_line_length;
    end_max = TRUE;
  } else {
    if (equal(EQUIVCHARstr, word[1], 1)) {
      col1 = CURRENT_VIEW->verify_end;
    } else {
      if (strcmp((char *) word[1], "*") == 0) {
        col2 = max_line_length;
        end_max = TRUE;
      } else {
        if ((rc = valid_positive_integer_against_maximum(word[1], MAX_WIDTH_NUM)) != 0) {
          if (rc == 4)
            sprintf((char *) buffer, "%s", word[1]);
          else
            sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
          display_error(rc, buffer, FALSE);
          return (RC_INVALID_OPERAND);
        } else {
          col2 = atol((char *) word[1]);
          if (col2 == 0) {
            display_error(5, word[0], FALSE);
            return (RC_INVALID_OPERAND);
          }
        }
      }
    }
  }

  if (col2 > max_line_length)
    col2 = max_line_length;
  if (col1 > col2) {
    display_error(6, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW->verify_start = (length_t) col1;
  CURRENT_VIEW->verify_col = (length_t) col1;
  CURRENT_VIEW->verify_end = (length_t) col2;
  CURRENT_VIEW->verify_end_max = end_max;

  build_screen(current_screen);
  display_screen(current_screen);

  return (RC_OK);
}
short Width(char_t * params) {
  length_t width = 0;
  length_t save_max_line_length = max_line_length;
  char_t buffer[100];
  short rc;

  if ((rc = valid_positive_integer_against_maximum(params, MAX_WIDTH_NUM)) != 0) {
    if (rc == 4)
      sprintf((char *) buffer, "%s", params);
    else
      sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
    display_error(rc, buffer, FALSE);
    return (RC_INVALID_OPERAND);
  }
  width = atol((char *) params);
  if (width < 10) {
    display_error(5, (char_t *) "- MUST be >= 10", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (width > MAX_WIDTH_NUM) {
    sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
    display_error(6, buffer, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (display_length > 0 && display_length > width) {
    display_error(6, (char_t *) "- width MUST be >= display length", FALSE);
    return (RC_INVALID_OPERAND);
  }
  max_line_length = width;
  if (allocate_working_memory() != 0) {
    /*
     * Our attempt to allocate the new memory failed, so reset the max_line_length
     * and re-allocate our original memory. Tell the user we failed
     */
    max_line_length = save_max_line_length;
    if (allocate_working_memory() != 0) {
      /*
       * If we fail again, bail out severely!!
       */
      display_error(30, (char_t *) "", FALSE);
      exit(1);
    }
    sprintf((char *) buffer, "- Unable to set WIDTH to %ld; using current WIDTH", width);
    display_error(30, buffer, FALSE);
  }
  /*
   * If VERIFY end or ZONE end was specified as '*' reset to max_line_length
   */
  if (CURRENT_VIEW->verify_end_max || CURRENT_VIEW->zone_end_max) {
    CURRENT_VIEW->verify_end = max_line_length;
    CURRENT_VIEW->zone_end = max_line_length;
    if (CURRENT_VIEW->scale_on || CURRENT_VIEW->boundmark != BOUNDMARK_OFF) {
      build_screen(current_screen);
      display_screen(current_screen);
    }
  }
  if (display_screens > 1 && (OTHER_VIEW->verify_end_max || OTHER_VIEW->zone_end_max)) {
    OTHER_VIEW->verify_end = max_line_length;
    OTHER_VIEW->zone_end = max_line_length;
    if (OTHER_VIEW->scale_on || OTHER_VIEW->boundmark != BOUNDMARK_OFF) {
      build_screen(other_screen);
      display_screen(other_screen);
    }
  }
  show_statarea();
  return (RC_OK);
}
short Word(char_t * params) {
  short rc = RC_OK;

  /*
   * Validate the the only parameter is 'NONBlank' or 'ALPHAnum'.
   */
  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "nonblank", params, 4))
    CURRENT_VIEW->word = 'N';
  else if (equal((char_t *) "alphanum", params, 5))
    CURRENT_VIEW->word = 'A';
  else {
    display_error(1, params, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}
short Wordwrap(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->wordwrap, TRUE);
  return (rc);
}
short Wrap(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->wrap, TRUE);
  return (rc);
}
short Zone(char_t * params) {
#define ZON_PARAMS  3
  char_t *word[ZON_PARAMS + 1];
  char_t strip[ZON_PARAMS];
  unsigned short num_params = 0;
  bool end_max = FALSE;
  length_t col1 = 0L, col2 = 0L;

  /*
   * Validate the parameters that have been supplied. One only
   * parameter MUST be supplied. The first parameter MUST be a positive
   * integer. The second can be a positive integer or '*'. If no second
   * parameter is supplied, ERROR.          The second parameter MUST be
   * >= first parameter. '*' is regarded as the biggest number and is
   * literally max_line_length.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, ZON_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  } else if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * First parameter must be positive integer or EQUIVCHAR
   */
  if (equal(EQUIVCHARstr, word[0], 1)) {
    col1 = CURRENT_VIEW->zone_start;
  } else {
    if (!valid_positive_integer(word[0])) {
      display_error(4, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    col1 = atol((char *) word[0]);
  }
  /*
   * Second parameter (if present) must be positive integer or EQUIVCHAR
   */
  if (num_params == 1) {
    col2 = max_line_length;
    end_max = TRUE;
  } else {
    if (equal(EQUIVCHARstr, word[1], 1)) {
      col2 = CURRENT_VIEW->zone_end;
    } else {
      if (strcmp((char *) word[1], "*") == 0) {
        col2 = max_line_length;
        end_max = TRUE;
      } else {
        if (!valid_positive_integer(word[1])) {
          display_error(4, word[1], FALSE);
          return (RC_INVALID_OPERAND);
        } else
          col2 = atol((char *) word[1]);
      }
    }
  }

  if (col2 > max_line_length)
    col2 = max_line_length;
  if (col1 > col2) {
    display_error(6, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW->zone_start = (length_t) col1;
  CURRENT_VIEW->zone_end = (length_t) col2;
  CURRENT_VIEW->zone_end_max = end_max;
  /*
   * Change the current column pointer if it is outside the new zone
   * settings...
   */
  if (CURRENT_VIEW->current_column < CURRENT_VIEW->zone_start)
    CURRENT_VIEW->current_column = max(1, CURRENT_VIEW->zone_start - 1);
  else if (CURRENT_VIEW->current_column > CURRENT_VIEW->zone_end)
    CURRENT_VIEW->current_column = min(max_line_length, CURRENT_VIEW->zone_end + 1);
  /*
   * If the SCALE line is currently displayed, display the page so that
   * any changes are reflected in the SCALE line. Also display page if
   * boundmark is not off.
   */
  if (CURRENT_VIEW->scale_on || CURRENT_VIEW->boundmark != BOUNDMARK_OFF) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (RC_OK);
}
