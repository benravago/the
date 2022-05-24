/* COMMSET1.C - SET commands A-N                                       */
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

the_header_mapping thm[] = {
  { "NUMBER", 6, HEADER_NUMBER },
  { "COMMENT", 7, HEADER_COMMENT },
  { "STRING", 6, HEADER_STRING },
  { "KEYWORD", 7, HEADER_KEYWORD },
  { "FUNCTION", 8, HEADER_FUNCTION },
  { "HEADER", 6, HEADER_HEADER },
  { "LABEL", 5, HEADER_LABEL },
  { "MATCH", 5, HEADER_MATCH },
  { "COLUMN", 6, HEADER_COLUMN },
  { "POSTCOMPARE", 11, HEADER_POSTCOMPARE },
  { "MARKUP", 6, HEADER_MARKUP },
  { "DIRECTORY", 3, HEADER_DIRECTORY },
  { "*", 1, HEADER_ALL },       /* this should be last */
  { NULL, 0, 0 },
};

/*#define DEBUG 1*/

static short set_active_colour(short area) {
  int i;
  COLOUR_ATTR attr;
  chtype ch = 0L, nondisp_attr = 0L;

  memcpy(&attr, CURRENT_FILE->attr + area, sizeof(COLOUR_ATTR));
  /*
   * Special handling required for NONDISP...
   */
  if (area == ATTR_NONDISP) {
    nondisp_attr = set_colour(&attr);
    for (i = 0; i < 256; i++) {
      if (etmode_flag[i]) {
        ch = etmode_table[i] & A_CHARTEXT;
        etmode_table[i] = ch | nondisp_attr;
      }
    }
  }
  /*
   * If we haven't started curses (in profile first time) exit now...
   */
  if (!curses_started) {
    return (RC_OK);
  }
  /*
   * Update the appropriate window with the new colour combination...
   */
  switch (valid_areas[area].area_window) {
    case WINDOW_FILEAREA:
      if (area == ATTR_FILEAREA)
        wattrset(CURRENT_WINDOW_FILEAREA, set_colour(CURRENT_FILE->attr + area));
      build_screen(current_screen);
      display_screen(current_screen);
      break;
    case WINDOW_PREFIX:
      if (CURRENT_WINDOW_PREFIX != NULL) {
        wattrset(CURRENT_WINDOW_PREFIX, set_colour(CURRENT_FILE->attr + area));
        build_screen(current_screen);
        display_screen(current_screen);
      }
      break;
    case WINDOW_COMMAND:
      if (CURRENT_WINDOW_COMMAND != NULL) {
        wattrset(CURRENT_WINDOW_COMMAND, set_colour(CURRENT_FILE->attr + area));
        redraw_window(CURRENT_WINDOW_COMMAND);
        touchwin(CURRENT_WINDOW_COMMAND);
        wnoutrefresh(CURRENT_WINDOW_COMMAND);
      }
      break;
    case WINDOW_ARROW:
      if (CURRENT_WINDOW_ARROW != NULL) {
        wattrset(CURRENT_WINDOW_ARROW, set_colour(CURRENT_FILE->attr + area));
        redraw_window(CURRENT_WINDOW_ARROW);
        touchwin(CURRENT_WINDOW_ARROW);
        wnoutrefresh(CURRENT_WINDOW_ARROW);
      }
      break;
    case WINDOW_IDLINE:
      if (CURRENT_WINDOW_IDLINE != NULL) {
        wattrset(CURRENT_WINDOW_IDLINE, set_colour(CURRENT_FILE->attr + area));
        redraw_window(CURRENT_WINDOW_IDLINE);
        touchwin(CURRENT_WINDOW_IDLINE);
        wnoutrefresh(CURRENT_WINDOW_IDLINE);
      }
      break;
    case WINDOW_STATAREA:
      if (statarea != NULL) {
        wattrset(statarea, set_colour(CURRENT_FILE->attr + area));
        redraw_window(statarea);
        touchwin(statarea);
        wnoutrefresh(statarea);
      }
      break;
    case WINDOW_FILETABS:
      if (filetabs != NULL) {
        wattrset(filetabs, set_colour(CURRENT_FILE->attr + area));
        redraw_window(filetabs);
        touchwin(filetabs);
        wnoutrefresh(filetabs);
      }
      break;
    case WINDOW_DIVIDER:
      if (divider != (WINDOW *) NULL) {
        wattrset(divider, set_colour(CURRENT_FILE->attr + area));
        if (display_screens > 1 && !horizontal) {
          draw_divider();
          touchwin(divider);
          wnoutrefresh(divider);
        }
      }
      break;
    case WINDOW_SLK:
      if (max_slk_labels) {
        slk_attrset(set_colour(CURRENT_FILE->attr + area));
        slk_touch();
        slk_noutrefresh();
      }
      break;
    default:
      break;
  }
  return (RC_OK);
}

short Alt(char_t * params) {
#define ALT_PARAMS  2
  char_t strip[ALT_PARAMS];
  char_t *word[ALT_PARAMS + 1];
  unsigned short num_params = 0;
  unsigned short autosave_alt = CURRENT_FILE->autosave_alt;
  unsigned short save_alt = CURRENT_FILE->save_alt;

  strip[0] = STRIP_BOTH;
  num_params = param_split(params, word, ALT_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal(word[0], EQUIVCHARstr, 1))
    autosave_alt = CURRENT_FILE->autosave_alt;
  else {
    if (!valid_positive_integer(word[0])) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    autosave_alt = atoi((char *) word[0]);
  }
  if (num_params == 2) {
    if (equal(word[1], EQUIVCHARstr, 1))
      save_alt = CURRENT_FILE->save_alt;
    else {
      if (!valid_positive_integer(word[1])) {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      save_alt = atoi((char *) word[1]);
    }
  }

  CURRENT_FILE->autosave_alt = autosave_alt;
  CURRENT_FILE->save_alt = save_alt;

  return (RC_OK);
}
short Arbchar(char_t * params) {
#define ARB_PARAMS  4
  char_t *word[ARB_PARAMS + 1];
  char_t strip[ARB_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_INVALID_OPERAND;
  bool arbsts = CURRENT_VIEW->arbchar_status;
  char_t arbchr_single = CURRENT_VIEW->arbchar_single;
  char_t arbchr_multiple = CURRENT_VIEW->arbchar_multiple;

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  num_params = param_split(params, word, ARB_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 0:
      /*
       * No parameters, error.
       */
      display_error(3, (char_t *) "", FALSE);
      break;
    case 1:
      /*
       * 1 or 2 parameters, validate them...
       */
      if (equal(word[0], EQUIVCHARstr, 1));
      else
        rc = execute_set_on_off(word[0], &arbsts, TRUE);
      break;
    case 2:
    case 3:
      if (equal(word[0], EQUIVCHARstr, 1));
      else {
        rc = execute_set_on_off(word[0], &arbsts, TRUE);
        if (rc != RC_OK)
          break;
      }
      rc = RC_INVALID_OPERAND;
      /*
       * For 2 parameters, check that a single character has been supplied...
       */
      if (equal(word[1], EQUIVCHARstr, 1));
      else {
        if (strlen((char *) word[1]) != 1) {
          display_error(1, word[1], FALSE);
          break;
        }
        arbchr_multiple = word[1][0];
      }
      rc = RC_OK;
      /*
       * For 2 parameters, don't check any more.
       */
      if (num_params == 2)
        break;
      rc = RC_INVALID_OPERAND;
      /*
       * For 3 parameters, check that a single character has been supplied...
       */
      if (equal(word[2], EQUIVCHARstr, 1));
      else {
        if (strlen((char *) word[2]) != 1) {
          display_error(1, word[2], FALSE);
          break;
        }
        arbchr_single = word[2][0];
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
    CURRENT_VIEW->arbchar_single = arbchr_single;
    CURRENT_VIEW->arbchar_multiple = arbchr_multiple;
    CURRENT_VIEW->arbchar_status = arbsts;
  }
  return (rc);
}
short Autocolour(char_t * params) {
#define AUCO_PARAMS  3
  char_t *word[AUCO_PARAMS + 1];
  char_t strip[AUCO_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;
  PARSER_DETAILS *parser = NULL;
  PARSER_MAPPING *mapping = NULL, *curr = NULL, *tmp_mapping;
  char_t *filemask = NULL, *magic_number = NULL;
  VIEW_DETAILS *curr_view = vd_first;
  bool redisplay_current = FALSE, redisplay_other = FALSE;
  int i, change = 0;

  /*
   * Validate the parameters that have been supplied.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, AUCO_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  filemask = word[0];
  if (num_params == 3) {
    if (equal((char_t *) "magic", word[2], 5)) {
      magic_number = word[0];
      filemask = NULL;
    } else {
      display_error(1, word[2], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Find a parser equal to the first parameter...
   */
  if (!equal((char_t *) "*null", word[1], 5)) {
    parser = parserll_find(first_parser, word[1]);
    if (parser == NULL) {
      display_error(199, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Now check if we already have a mapping for the mask/magic number
   */
  mapping = mappingll_find(first_parser_mapping, filemask, magic_number);
  if (mapping)
    curr = mapping;
  /*
   * Add the new mapping if it is a "real" parser.
   */
  if (parser) {
    curr = last_parser_mapping = mappingll_add(first_parser_mapping, last_parser_mapping, sizeof(PARSER_MAPPING));
    if (first_parser_mapping == NULL)
      first_parser_mapping = curr;
    if (filemask) {
      curr->filemask = (char_t *) malloc(1 + strlen((char *) filemask) * sizeof(char_t));
      if (curr->filemask == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) curr->filemask, (char *) filemask);
    }
    if (magic_number) {
      curr->magic_number = (char_t *) malloc(1 + strlen((char *) magic_number) * sizeof(char_t));
      if (curr->magic_number == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) curr->magic_number, (char *) magic_number);
      curr->magic_number_length = strlen((char *) magic_number);
    }
    curr->parser = parser;
  }
  /*
   * Check all files in the ring and apply the new mapping. If the current
   * file or the file in the other screen now match the new mapping,
   * redisplay them.
   */
  for (i = 0; i < number_of_files;) {
    if (curr && find_parser_mapping(curr_view->file_for_view, curr)) {
      curr_view->file_for_view->parser = parser;
      if (curr_view->file_for_view == SCREEN_FILE(current_screen))
        redisplay_current = TRUE;
      if (display_screens > 1 && curr_view->file_for_view == SCREEN_FILE((char_t) (other_screen)))
        redisplay_other = TRUE;
    }
    curr_view = curr_view->next;
    if (curr_view == NULL)
      break;
  }
  /*
   * Now delete the old mapping if we found one earlier...
   */
  if (mapping) {
    mappingll_del(&first_parser_mapping, &last_parser_mapping, mapping, DIRECTION_FORWARD);
    change--;
  }
  if (parser) {
    change++;
  }
  if (rexx_support) {
    if (change > 0) {
      char_t tmp[30];

      /*
       * As this is a new mapping, then register another implied extract
       * function for the number of mappings we now have.
       */
      for (i = 0, tmp_mapping = first_parser_mapping; tmp_mapping != NULL; tmp_mapping = tmp_mapping->next, i++);
      sprintf((char *) tmp, "autocolour.%d", i);
      MyRexxRegisterFunctionExe(tmp);
      sprintf((char *) tmp, "autocolor.%d", i);
      MyRexxRegisterFunctionExe(tmp);
    }
    if (change < 0) {
      char_t tmp[30];

      /*
       * As this is a removal of a mapping, then deregister the implied extract
       * function for the number of mappings we had before.
       */
      for (i = 0, tmp_mapping = first_parser_mapping; tmp_mapping != NULL; tmp_mapping = tmp_mapping->next, i++);
      sprintf((char *) tmp, "autocolour.%d", i - 1);
      MyRexxDeregisterFunction(tmp);
      sprintf((char *) tmp, "autocolor.%d", i - 1);
      MyRexxDeregisterFunction(tmp);
    }
  }

  if (redisplay_other)
    display_screen((char_t) (other_screen));
  if (redisplay_current)
    display_screen(current_screen);

  return (rc);
}
short Autosave(char_t * params) {
#define AUS_PARAMS  1
  char_t strip[AUS_PARAMS];
  char_t *word[AUS_PARAMS + 1];
  unsigned short num_params = 0;

  strip[0] = STRIP_BOTH;
  num_params = param_split(params, word, AUS_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params != 1) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "off", word[0], 3)) {
    CURRENT_FILE->autosave = 0;
    return (RC_OK);
  }
  if (!valid_positive_integer(word[0])) {
    display_error(4, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_FILE->autosave = (char_t) atoi((char *) word[0]);
  return (RC_OK);
}
short Autoscroll(char_t * params) {
#define AUL_PARAMS  1
  char_t strip[AUL_PARAMS];
  char_t *word[AUL_PARAMS + 1];
  unsigned short num_params = 0;

  strip[0] = STRIP_BOTH;
  num_params = param_split(params, word, AUL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params != 1) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "off", word[0], 3)) {
    CURRENT_VIEW->autoscroll = 0;
    return (RC_OK);
  }
  if (equal((char_t *) "half", word[0], 1)) {
    CURRENT_VIEW->autoscroll = (-1);
    return (RC_OK);
  }
  if (!valid_positive_integer(word[0])) {
    display_error(4, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW->autoscroll = (char_t) atol((char *) word[0]);
  return (RC_OK);
}
short Backup(char_t * params) {
  short rc = RC_OK;

#define BAC_PARAMS  2
  char_t *word[BAC_PARAMS + 1];
  char_t strip[BAC_PARAMS];
  short num_params = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, BAC_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate the first parameter
   */
  if (equal((char_t *) "off", word[0], 3))
    CURRENT_FILE->backup = BACKUP_OFF;
  else if (equal((char_t *) "on", word[0], 2))
    CURRENT_FILE->backup = BACKUP_ON;
  else if (equal((char_t *) "keep", word[0], 4))
    CURRENT_FILE->backup = BACKUP_KEEP;
  else if (equal((char_t *) "temp", word[0], 4))
    CURRENT_FILE->backup = BACKUP_TEMP;
  else if (equal((char_t *) "inplace", word[0], 2))
    CURRENT_FILE->backup = BACKUP_INPLACE;
  else {
    display_error(1, word[0], FALSE);
    rc = RC_INVALID_OPERAND;
  }
  if (num_params == 2) {
    /*
     * Save the second arg as the backup suffix
     */
    if (strlen((char *) word[1]) > 100) {
      display_error(37, word[1], FALSE);
      rc = RC_INVALID_OPERAND;
    } else {
      strcpy((char *) BACKUP_SUFFIXx, (char *) word[1]);
    }
  }
  return (rc);
}
short BeepSound(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &BEEPx, TRUE);
  return (rc);
}
short Boundmark(char_t * params) {
#define BND_PARAMS  2
  char_t save_boundmark = CURRENT_VIEW->boundmark;
  char_t *word[BND_PARAMS + 1];
  char_t strip[BND_PARAMS];
  short num_params = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, BND_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 1) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate the first and only parameter
   */
  if (equal((char_t *) "zone", word[0], 1))
    CURRENT_VIEW->boundmark = BOUNDMARK_ZONE;
  else if (equal((char_t *) "trunc", word[0], 2))
    CURRENT_VIEW->boundmark = BOUNDMARK_TRUNC;
  else if (equal((char_t *) "margins", word[0], 2))
    CURRENT_VIEW->boundmark = BOUNDMARK_MARGINS;
  else if (equal((char_t *) "tabs", word[0], 3))
    CURRENT_VIEW->boundmark = BOUNDMARK_TABS;
  else if (equal((char_t *) "verify", word[0], 1))
    CURRENT_VIEW->boundmark = BOUNDMARK_VERIFY;
  else if (equal((char_t *) "off", word[0], 3))
    CURRENT_VIEW->boundmark = BOUNDMARK_OFF;
  else {
    display_error(1, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the value has changed, display the screen
   */
  if (CURRENT_VIEW->boundmark != save_boundmark) {
    build_screen(current_screen);
    display_screen(current_screen);
  }

  return (RC_OK);
}
short Case(char_t * params) {
#define CAS_PARAMS  6
  char_t parm[CAS_PARAMS];
  char_t *word[CAS_PARAMS + 1];
  char_t strip[CAS_PARAMS];

  /*
   * Type 0 is Mixed|Upper|Lower
   * Type 1 is Respect|Ignore
   */
  char arg_types[CAS_PARAMS + 1] = "011100";
  register short i = 0;
  short num_params = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(params, word, CAS_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Save the current values of each optional case setting.
   */
  parm[1] = CURRENT_VIEW->case_locate;
  parm[2] = CURRENT_VIEW->case_change;
  parm[3] = CURRENT_VIEW->case_sort;
  parm[4] = CURRENT_VIEW->case_enter_cmdline;
  parm[5] = CURRENT_VIEW->case_enter_prefix;
  /*
   * Validate all arguments.
   */
  for (i = 0; i < num_params; i++) {
    if (strcmp((char *) word[i], "") != 0) {
      if (arg_types[i] == '0') {
        if (equal((char_t *) "mixed", word[i], 1))
          parm[i] = CASE_MIXED;
        else if (equal((char_t *) "upper", word[i], 1))
          parm[i] = CASE_UPPER;
        else if (equal((char_t *) "lower", word[i], 1))
          parm[i] = CASE_LOWER;
        else if (equal((char_t *) EQUIVCHARstr, word[i], 1))
          parm[i] = CURRENT_VIEW->case_enter;
        else {
          display_error(1, (char_t *) word[i], FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        if (equal((char_t *) "respect", word[i], 1))
          parm[i] = CASE_RESPECT;
        else if (equal((char_t *) "ignore", word[i], 1))
          parm[i] = CASE_IGNORE;
        else if (equal((char_t *) EQUIVCHARstr, word[i], 1))
          parm[i] = parm[i];
        else {
          display_error(1, (char_t *) word[i], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
    }
  }
  /*
   * Set the new values of case settings for the view.
   */
  CURRENT_VIEW->case_enter = parm[0];
  CURRENT_VIEW->case_locate = parm[1];
  CURRENT_VIEW->case_change = parm[2];
  CURRENT_VIEW->case_sort = parm[3];
  CURRENT_VIEW->case_enter_cmdline = parm[4];
  CURRENT_VIEW->case_enter_prefix = parm[5];

  return (RC_OK);
}
short Clearerrorkey(char_t * params) {
  short rc = RC_OK;
  int key = 0;

  if (strcmp((char *) params, "*") == 0) {
    CLEARERRORKEYx = -1;
  } else {
    key = find_key_name(params);
    if (key == -1) {
      display_error(13, params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    CLEARERRORKEYx = key;
  }
  return (rc);
}
short Clearscreen(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CLEARSCREENx, TRUE);
  return (rc);
}
short Clock(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CLOCKx, TRUE);
  if (rc == RC_OK && curses_started)
    clear_statarea();
  return (rc);
}
short Cmdarrows(char_t * params) {
  short rc = RC_OK;

  /*
   * Determine values for first parameter
   */
  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "tab", params, 1))
    CMDARROWSTABCMDx = TRUE;
  else if (equal((char_t *) "retrieve", params, 1))
    CMDARROWSTABCMDx = FALSE;
  else {
    display_error(1, params, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}
short Cmdline(char_t * params) {
  char_t cmd_place = '?';
  short rc = RC_OK;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "top", params, 1)) {
    cmd_place = 'T';
  } else if (equal((char_t *) "bottom", params, 1)
             || equal((char_t *) "on", params, 2)) {
    cmd_place = 'B';
  } else if (equal((char_t *) "off", params, 3)) {
    cmd_place = 'O';
  } else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the setting supplied is the same as the current setting, just
   * return without doing anything.
   */
  if (cmd_place == CURRENT_VIEW->cmd_line) {
    return (RC_OK);
  }
  /*
   * Now we need to move the windows around.
   */
  CURRENT_VIEW->cmd_line = cmd_place;
  /*
   * Rebuild the windows and display...
   */
  set_screen_defaults();
  if (curses_started) {
    if (set_up_windows(current_screen) != RC_OK) {
      return (rc);
    }
  }
  if (CURRENT_VIEW->cmd_line == 'O')
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
  build_screen(current_screen);
  if (curses_started)
    display_screen(current_screen);

  return (rc);
}
short Colour(char_t * params) {
#define COL_PARAMS_DEF 2
#define COL_PARAMS_COLOUR 4
#define COL_MODIFIER_NO_SET  0
#define COL_MODIFIER_SET_ON  1
#define COL_MODIFIER_SET_OFF 2
  char_t *word[COL_PARAMS_COLOUR + 1];
  char_t strip[COL_PARAMS_COLOUR];
  char_t parm[COL_PARAMS_COLOUR];
  register short i = 0;
  unsigned short num_params = 0;
  short area = -1;
  COLOUR_ATTR attr, tmp_attr;
  char_t *dummy = NULL;
  bool any_colours = FALSE;
  short word1_len, modifier_set = COL_MODIFIER_NO_SET;
  bool window_set[MAX_THE_WINDOWS];
  int clr;
  int cont[3];

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  num_params = param_split(params, word, COL_PARAMS_DEF, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Check which format of this command we are running.
   * If the last word is ON or OFF then we are executing the
   * second format.
   * If the first word is a colour, it is format three.
   */
  word1_len = strlen((char *) word[1]);
  clr = is_valid_colour(word[0]);
  if (clr == (-1)) {
    {
      if (strcasecmp((char *) word[1] + word1_len - 3, " ON") == 0)
        modifier_set = COL_MODIFIER_SET_ON;
      else if (strcasecmp((char *) word[1] + word1_len - 4, " OFF") == 0)
        modifier_set = COL_MODIFIER_SET_OFF;
      if (modifier_set) {
        /*
         * Check that first parameter is an area or '*'
         */
        parm[0] = FALSE;
        if (strcmp((char *) word[0], "*") == 0) {
          area = -1;
          parm[0] = TRUE;
        } else {
          for (i = 0; i < ATTR_MAX; i++) {
            if (equal(valid_areas[i].area, word[0], valid_areas[i].area_min_len)) {
              parm[0] = TRUE;
              area = i;
              break;
            }
          }
        }
        if (parm[0] == FALSE) {
          display_error(1, (char_t *) word[0], FALSE);
          return (RC_INVALID_OPERAND);
        }
        /*
         * Check that each subsequent parameter (except the last) is
         * a modifier.
         */
        if (parse_modifiers(word[1], &tmp_attr) != RC_OK) {
          return (RC_INVALID_OPERAND);
        }
        /*
         * For each area, turn off the modifiers and redraw the affected part of
         * the screen
         */
        if (area == (-1)) {
          for (i = 0; i < MAX_THE_WINDOWS; i++) {
            window_set[i] = FALSE;
          }
          for (i = 0; i < ATTR_MAX; i++) {
            attr = CURRENT_FILE->attr[i];
            if (modifier_set == COL_MODIFIER_SET_ON) {
              if (colour_support)
                attr.mod |= tmp_attr.mod;
              else
                attr.mono |= tmp_attr.mono;
            } else {
              if (colour_support)
                attr.mod &= ~tmp_attr.mod;
              else
                attr.mono &= ~tmp_attr.mono;
            }
            CURRENT_FILE->attr[i] = attr;
            if (i == ATTR_BOUNDMARK || i == ATTR_NONDISP || window_set[valid_areas[i].area_window] == FALSE) {
              set_active_colour(i);
              window_set[valid_areas[i].area_window] = TRUE;
            }
          }
        } else {
          attr = CURRENT_FILE->attr[area];
          if (modifier_set == COL_MODIFIER_SET_ON) {
            if (colour_support)
              attr.mod |= tmp_attr.mod;
            else
              attr.mono |= tmp_attr.mono;
          } else {
            if (colour_support)
              attr.mod &= ~tmp_attr.mod;
            else
              attr.mono &= ~tmp_attr.mono;
          }
          CURRENT_FILE->attr[area] = attr;
          set_active_colour(area);
        }
      } else {
        /*
         * Check that the supplied area matches one of the values in the area
         * array and that the length is at least as long as the minimum.
         */
        parm[0] = FALSE;
        for (i = 0; i < ATTR_MAX; i++) {
          if (equal(valid_areas[i].area, word[0], valid_areas[i].area_min_len)) {
            parm[0] = TRUE;
            area = i;
            break;
          }
        }
        if (parm[0] == FALSE) {
          display_error(1, (char_t *) word[0], FALSE);
          return (RC_INVALID_OPERAND);
        }
        attr = CURRENT_FILE->attr[area];
        /*
         * Determine colours and modifiers.
         */
        if (parse_colours(word[1], &attr, &dummy, FALSE, &any_colours) != RC_OK) {
          return (RC_INVALID_OPERAND);
        }
        /*
         * Now we have the new colours, save them with the current file...
         */
        CURRENT_FILE->attr[area] = attr;
        set_active_colour(area);
      }
    }
  } else {
    /* can the terminal support changing colour? */
    if (!can_change_color()) {
      display_error(61, (char_t *) "Changing colors unsupported.", FALSE);
      return (RC_INVALID_ENVIRON);
    }
    /* SET COLOUR colour red green blue */
    num_params = param_split(params, word, COL_PARAMS_COLOUR, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
    if (num_params != 4) {
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
    }
    /*
     * Validate the colour contents
     */
    for (i = 1; i < 4; i++) {
      if (!valid_positive_integer(word[i])) {
        display_error(4, (char_t *) word[i], FALSE);
        return (RC_INVALID_OPERAND);
      }
      cont[i - 1] = atoi((char *) word[i]);
      if (cont[i - 1] < 0) {
        display_error(5, (char_t *) word[i], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if (cont[i - 1] > 1000) {
        display_error(6, (char_t *) word[i], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
    init_color(clr, cont[0], cont[1], cont[2]);
  }
  return (RC_OK);
}

short Colouring(char_t * params) {
#define COLG_PARAMS  2
  char_t *word[COLG_PARAMS + 1];
  char_t strip[COLG_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  bool new_colouring = FALSE;
  PARSER_DETAILS *new_parser = NULL;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, COLG_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  rc = execute_set_on_off(word[0], &new_colouring, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }
  if (num_params == 1 && new_colouring == TRUE) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }

  if (new_colouring) {
    /*
     * This is only applicable when turning colouring ON
     */
    if (equal((char_t *) "AUTO", word[1], 4)) {
      /*
       * Set the parser to the parser for the file extension or
       * to NULL if no parser is set up for this extension.
       */
      new_parser = find_auto_parser(CURRENT_FILE);
      CURRENT_FILE->autocolour = TRUE;
    } else {
      /*
       * Look for a parser with the specified name
       */
      new_parser = parserll_find(first_parser, word[1]);
      if (new_parser == NULL) { /* no parser by that name... */
        display_error(199, word[1], FALSE);
        return RC_INVALID_OPERAND;
      }
      CURRENT_FILE->autocolour = FALSE;
    }
  }
  CURRENT_FILE->parser = new_parser;
  CURRENT_FILE->colouring = new_colouring;
  /*
   * If all is OK, redisplay the screen to get the new colouring
   */
  if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
    display_screen((char_t) (other_screen));
  }
  display_screen(current_screen);
  return (RC_OK);
}

short Compat(char_t * params) {
#define COM_PARAMS  4
  char_t *word[COM_PARAMS + 1];
  char_t strip[COM_PARAMS];
  short num_params = 0;
  int rc = RC_OK;
  int prey = 0, prex = 0;
  short save_look = compatible_look;
  short save_feel = compatible_feel;
  short save_keys = compatible_keys;
  short new_look = 0;
  short new_feel = 0;
  short new_keys = 0;
  unsigned short save_autosave_alt = 0;
  unsigned short save_save_alt = 0;

  /*
   * Parse the parameters...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_NONE;
  num_params = param_split(params, word, COM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 3) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "the", word[0], 1))
    new_look = COMPAT_THE;
  else if (equal((char_t *) "xedit", word[0], 1))
    new_look = COMPAT_XEDIT;
  else if (equal((char_t *) "kedit", word[0], 1))
    new_look = COMPAT_KEDIT;
  else if (equal((char_t *) "keditw", word[0], 6))
    new_look = COMPAT_KEDITW;
  else if (equal((char_t *) "ispf", word[0], 1))
    new_look = COMPAT_ISPF;
  else if (equal((char_t *) EQUIVCHARstr, word[0], 1))
    new_look = save_look;
  else {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params == 1) {
    new_feel = save_feel;
    new_keys = save_keys;
  } else {
    if (equal((char_t *) "the", word[1], 1))
      new_feel = COMPAT_THE;
    else if (equal((char_t *) "xedit", word[1], 1))
      new_feel = COMPAT_XEDIT;
    else if (equal((char_t *) "kedit", word[1], 1))
      new_feel = COMPAT_KEDIT;
    else if (equal((char_t *) "keditw", word[1], 6))
      new_feel = COMPAT_KEDITW;
    else if (equal((char_t *) "ispf", word[1], 1))
      new_feel = COMPAT_ISPF;
    else if (equal((char_t *) EQUIVCHARstr, word[1], 1))
      new_feel = save_feel;
    else {
      display_error(1, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params == 2)
      new_keys = save_keys;
    else {
      if (equal((char_t *) "the", word[2], 1))
        new_keys = COMPAT_THE;
      else if (equal((char_t *) "xedit", word[2], 1))
        new_keys = COMPAT_XEDIT;
      else if (equal((char_t *) "kedit", word[2], 1))
        new_keys = COMPAT_KEDIT;
      else if (equal((char_t *) "keditw", word[2], 6))
        new_keys = COMPAT_KEDITW;
      else if (equal((char_t *) "ispf", word[2], 1))
        new_keys = COMPAT_ISPF;
      else if (equal((char_t *) EQUIVCHARstr, word[2], 1))
        new_keys = save_keys;
      else {
        display_error(1, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
  }
  compatible_look = new_look;
  compatible_feel = new_feel;
  compatible_keys = new_keys;
  /*
   * If the FEEL has changed, change the default feel...
   */
  set_global_feel_defaults();
  /*
   * If the KEYS has changed, change the default key definitions...
   */
  if (save_keys != compatible_keys) {
    switch (compatible_keys) {
      case COMPAT_THE:
        rc = set_THE_key_defaults(prey, prex);
        break;
      case COMPAT_XEDIT:
        rc = set_XEDIT_key_defaults(prey, prex);
        break;
      case COMPAT_ISPF:
        rc = set_ISPF_key_defaults(prey, prex);
        break;
      case COMPAT_KEDIT:
      case COMPAT_KEDITW:
        rc = set_KEDIT_key_defaults(prey, prex);
        break;
    }
    if (rc != RC_OK) {
      return (rc);
    }
  }
  /*
   * Now we have to change the LOOK of the current view...
   */
  if (curses_started) {
    if (CURRENT_WINDOW_PREFIX != NULL)
      getyx(CURRENT_WINDOW_PREFIX, prey, prex);
  }
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  /*
   * Reset common settings to defaults for THE...
   */
  set_global_look_defaults();

  save_autosave_alt = CURRENT_FILE->autosave_alt;
  save_save_alt = CURRENT_FILE->save_alt;
  set_file_defaults(CURRENT_FILE);
  CURRENT_FILE->autosave_alt = save_autosave_alt;
  CURRENT_FILE->save_alt = save_save_alt;
  set_view_defaults(CURRENT_VIEW);
  /*
   * Determine the size of each window in each screen in case any changes
   * in defaults caused some settings to include/exclude some windows...
   */
  set_screen_defaults();
  /*
   * For the common windows, set their attributes to match the new values
   */
  if (curses_started && statarea != NULL) {
    wattrset(statarea, set_colour(CURRENT_FILE->attr + ATTR_STATAREA));
    clear_statarea();
  }
  /*
   * If more than one screen displayed, redisplay the 'other' screen...
   */
  if (display_screens > 1) {
    OTHER_SCREEN.screen_view->current_row = calculate_actual_row(OTHER_SCREEN.screen_view->current_base, OTHER_SCREEN.screen_view->current_off, OTHER_SCREEN.rows[WINDOW_FILEAREA], TRUE);
    pre_process_line(OTHER_SCREEN.screen_view, OTHER_SCREEN.screen_view->focus_line, (LINE *) NULL);
    if (OTHER_SCREEN.screen_view->cmd_line == 'O')
      OTHER_SCREEN.screen_view->current_window = WINDOW_FILEAREA;
    if (curses_started) {
      if (set_up_windows(current_screen) != RC_OK) {
        return (rc);
      }
      if (!horizontal) {
        wattrset(divider, set_colour(OTHER_SCREEN.screen_view->file_for_view->attr + ATTR_DIVIDER));
        touchwin(divider);
        wnoutrefresh(divider);
      }
    }
    redraw_screen((char_t) ((current_screen == 0) ? 1 : 0));
    build_screen((char_t) (other_screen));
    display_screen((char_t) (other_screen));
  }
  /*
   * Redisplay the current screen...
   */
  CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  if (CURRENT_VIEW->cmd_line == 'O')
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
  if (curses_started) {
    if (set_up_windows(current_screen) != RC_OK) {
      return (rc);
    }
  }
  redraw_screen(current_screen);
  build_screen(current_screen);
  display_screen(current_screen);

  return (rc);
}
short Ctlchar(char_t * params) {
#define CTL_PARAMS  3
  char_t *word[CTL_PARAMS + 1];
  char_t strip[CTL_PARAMS];
  short num_params = 0;
  COLOUR_ATTR attr;
  char_t *dummy = NULL;
  bool any_colours = FALSE, protect, found;
  int i;
  bool have_ctlchar = TRUE;
  RESERVED *curr;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, CTL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 3) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params == 1) {
    if (equal((char_t *) "off", word[0], 1)) {
      if (num_params != 1) {
        display_error(2, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      }
      have_ctlchar = FALSE;
    } else {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  } else {
    if (strlen((char *) word[0]) > 1) {
      display_error(37, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num_params == 2) {
      if (equal((char_t *) "escape", word[1], 1)) {
        /*
         * Sets the value in word[0] to be the escape character
         */
        ctlchar_escape = word[0][0];
      } else if (equal((char_t *) "off", word[1], 3)) {
        /*
         * Turns off the escape character in word[0]
         * Find the entry in
         */
        for (i = 0; i < MAX_CTLCHARS; i++) {
          if (ctlchar_char[i] == word[0][0]) {
            ctlchar_char[i] = 0;
            break;
          }
        }
        /*
         * Find the first spare CTLCHAR spec for the supplied character
         * and add it.
         */
        found = FALSE;
        for (i = 0; i < MAX_CTLCHARS; i++) {
          if (ctlchar_char[i] == 0) {
            ctlchar_char[i] = word[0][0];
            ctlchar_attr[i].pair = -1;
            found = TRUE;
            break;
          }
        }
        if (!found) {
          display_error(80, (char_t *) "", FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else if (equal((char_t *) "protect", word[1], 1)) {
        display_error(3, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      } else if (equal((char_t *) "noprotect", word[1], 1)) {
        display_error(3, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      } else {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
    } else {
      /*
       * Now should be parsing colours to set the ctlchar colour for
       * the character in word[0][0]
       */
      if (equal((char_t *) "protect", word[1], 1))
        protect = TRUE;
      else if (equal((char_t *) "noprotect", word[1], 1))
        protect = FALSE;
      else {
        display_error(1, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      memset(&attr, 0, sizeof(COLOUR_ATTR));
      if (parse_colours(word[2], &attr, &dummy, FALSE, &any_colours) != RC_OK) {
        return (RC_INVALID_OPERAND);
      }
      /*
       * Find any existing CTLCHAR spec for the supplied character
       * and turn it off...
       */
      for (i = 0; i < MAX_CTLCHARS; i++) {
        if (ctlchar_char[i] == word[0][0]) {
          ctlchar_char[i] = 0;
          ctlchar_protect[i] = FALSE;
          break;
        }
      }
      /*
       * Find the first spare CTLCHAR spec for the supplied character
       * and add it.
       */
      found = FALSE;
      for (i = 0; i < MAX_CTLCHARS; i++) {
        if (ctlchar_char[i] == 0) {
          ctlchar_char[i] = word[0][0];
          ctlchar_attr[i] = attr;
          ctlchar_protect[i] = protect;
          found = TRUE;
          break;
        }
      }
      if (!found) {
        display_error(80, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
  }
  CTLCHARx = have_ctlchar;
  /*
   * For each current reserved line, reparse it to ensure the changes made
   * here are reflected correctly.
   */
  curr = CURRENT_FILE->first_reserved;
  while (curr) {
    parse_reserved_line(curr);
    curr = curr->next;
  }
  if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
    build_screen((char_t) (other_screen));
    display_screen((char_t) (other_screen));
  }
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}

short Curline(char_t * params) {
#define CUR_PARAMS  2
  char_t *word[CUR_PARAMS + 1];
  char_t strip[CUR_PARAMS];
  short num_params = 0;
  short rc = 0;
  short base = CURRENT_VIEW->current_base;
  short off = CURRENT_VIEW->current_off;
  short hexshow_row = 0, curline_row = 0;
  bool onoff = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, CUR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 0:
      display_error(3, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
      break;
    case 1:
      /*
       * Can only be a valid position
       */
      /*
       * Is the first argument ON ?
       */
      rc = execute_set_on_off(word[0], &onoff, FALSE);
      if (rc != RC_OK) {
        /*
         * If not ON or OFF, it must be a position
         */
        rc = execute_set_row_position(word[0], &base, &off);
        if (rc != RC_OK) {
          return (rc);
        }
        /*
         * A valid position, so it must be ON
         */
        onoff = TRUE;
        break;
      }
      /*
       * If its ON, error: too few operands
       */
      if (onoff == TRUE) {
        display_error(3, (char_t *) "", FALSE);
        return (RC_INVALID_OPERAND);
      } else {
        display_error(1, (char_t *) word[0], FALSE);
        return (RC_INVALID_OPERAND);
      }
      break;
    case 2:
      /*
       * First argument MUST be ON, 2nd a valid position
       */
      /*
       * Is the first argument ON or OFF ?
       */
      rc = execute_set_on_off(word[0], &onoff, TRUE);
      if (rc != RC_OK) {
        return (RC_INVALID_OPERAND);
      }
      /*
       * If its OFF, error
       */
      if (onoff == FALSE) {
        display_error(1, (char_t *) word[1], FALSE);
        return (RC_INVALID_OPERAND);
      }
      /*
       * Is the position correct ?
       */
      rc = execute_set_row_position(word[1], &base, &off);
      if (rc != RC_OK) {
        return (rc);
      }
      break;
    default:
      display_error(2, (char_t *) "", FALSE);
      return (RC_INVALID_OPERAND);
      break;
  }
  /*
   * If we have set the CURLINE OFF, redisplay the current screen???
   */
  if (onoff == FALSE) {
  } else {
    /*
     * If the CURLINE is the same line as HEXSHOW, SCALE, TABLE or has a
     * RESERVED line on it, return ERROR.
     */
    curline_row = calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
    if (calculate_actual_row(CURRENT_VIEW->scale_base, CURRENT_VIEW->scale_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == curline_row && CURRENT_VIEW->scale_on) {
      display_error(64, (char_t *) "- same as SCALE", FALSE);
      return (rc);
    }
    if (calculate_actual_row(CURRENT_VIEW->tab_base, CURRENT_VIEW->tab_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == curline_row && CURRENT_VIEW->tab_on) {
      display_error(64, (char_t *) "- same as TABLINE", FALSE);
      return (rc);
    }
    hexshow_row = calculate_actual_row(CURRENT_VIEW->hexshow_base, CURRENT_VIEW->hexshow_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
    if ((hexshow_row == curline_row || hexshow_row + 1 == curline_row)
        && CURRENT_VIEW->hexshow_on) {
      display_error(64, (char_t *) "- same as HEXSHOW", FALSE);
      return (rc);
    }
    if (find_reserved_line(current_screen, TRUE, curline_row, 0, 0) != NULL) {
      display_error(64, (char_t *) "- same as RESERVED line", FALSE);
      return (rc);
    }
    /*
     * If the "real" row for CURLINE is not the same as the generated one,
     * set the base and offset to reflect the generated row.
     */
    if (calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], FALSE) != curline_row) {
      CURRENT_VIEW->current_base = POSITION_MIDDLE;
      CURRENT_VIEW->current_off = 0;
    } else {
      CURRENT_VIEW->current_base = base;
      CURRENT_VIEW->current_off = off;
    }
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    CURRENT_VIEW->current_row = curline_row;
    build_screen(current_screen);
    display_screen(current_screen);
  }

  return (RC_OK);
}
short CursorStay(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &scroll_cursor_stay, TRUE);
  return (rc);
}
short Defsort(char_t * params) {
#define DIR_PARAMS  2
  char_t *word[DIR_PARAMS + 1];
  char_t strip[DIR_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  int defsort = 0;
  int dirorder = DIRSORT_ASC;

  /*
   * Here's a real hack! If we have REPROFILE ON and we call DEFSORT
   * in the profile, we go into infinite recursion, so we have to
   * check a special global variable; DONT_CALL_DEFSORTx, which is
   * only set to TRUE when EditFile() is called from this function.
   */
  if (DONT_CALL_DEFSORTx) {
    return (RC_OK);
  }
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, DIR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (equal((char_t *) "directory", word[0], 3))
    defsort = DIRSORT_DIR;
  else if (equal((char_t *) "name", word[0], 1))
    defsort = DIRSORT_NAME;
  else if (equal((char_t *) "time", word[0], 1))
    defsort = DIRSORT_TIME;
  else if (equal((char_t *) "size", word[0], 1))
    defsort = DIRSORT_SIZE;
  else if (equal((char_t *) "date", word[0], 1))
    defsort = DIRSORT_DATE;
  else if (equal((char_t *) "off", word[0], 3))
    defsort = DIRSORT_NONE;
  else {
    display_error(1, (char_t *) word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params == 2) {
    if (equal((char_t *) "ascending", word[1], 1))
      dirorder = DIRSORT_ASC;
    else if (equal((char_t *) "descending", word[1], 1))
      dirorder = DIRSORT_DESC;
    else {
      display_error(1, (char_t *) word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  DEFSORTx = defsort;
  DIRORDERx = dirorder;
  /*
   * If we are in DIR.DIR, then reload the directory
   */
  if (CURRENT_FILE->pseudo_file == PSEUDO_DIR) {
    DONT_CALL_DEFSORTx = TRUE;
    strcpy((char *) temp_cmd, (char *) dir_path);
    strcat((char *) temp_cmd, (char *) dir_files);
    /*
     * Edit the DIR.DIR file
     */
    rc = Directory(temp_cmd);
    DONT_CALL_DEFSORTx = FALSE;
  }
  return (rc);
}

short Display(char_t * params) {
  short rc = RC_OK;
  short col1 = 0, col2 = 0;

  if ((rc = validate_n_m(params, &col1, &col2)) != RC_OK) {
    return (rc);
  }
  CURRENT_VIEW->display_low = col1;
  CURRENT_VIEW->display_high = col2;
  /*
   * If we are on the command line and the result of this statement means
   * that the current line is no longer in scope, we need to make the
   * current line and possibly the focus line the next line in scope.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, NULL, get_true_line(TRUE), DIRECTION_FORWARD);
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
short Ecolour(char_t * params) {
#define ECOL_PARAMS 2
  char_t *word[ECOL_PARAMS + 1];
  char_t strip[ECOL_PARAMS];
  unsigned short num_params = 0;
  short area = 0, off;
  register short i = 0;
  COLOUR_ATTR attr, tmp_attr;
  char_t *dummy = NULL;
  bool any_colours = FALSE;
  char_t ch;
  short word1_len, modifier_set = COL_MODIFIER_NO_SET;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, ECOL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Check which format of this command we are running.
   * If the last word is ON or OFF then we are executing the
   * second format.
   */
  word1_len = strlen((char *) word[1]);
  if (strcasecmp((char *) word[1] + word1_len - 2, "ON") == 0)
    modifier_set = COL_MODIFIER_SET_ON;
  else if (strcasecmp((char *) word[1] + word1_len - 3, "OFF") == 0)
    modifier_set = COL_MODIFIER_SET_OFF;
  if (modifier_set) {
    /*
     * Check that the supplied area matches one of the values in the area
     * array and that the length is at least as long as the minimum.
     */
    if (strlen((char *) word[0]) != 1) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    ch = word[0][0];
    if (ch >= 'A' && ch <= 'Z')
      off = 'A';
    else if (ch >= 'a' && ch <= 'z')
      off = 'a';
    else if (ch >= '1' && ch <= '9')
      off = '1' - 26;           /* Beware: --x == +x */
    else if (ch == '*')
      off = -1;
    else {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    /*
     * Check that each subsequent parameter (except the last) is
     * a modifier.
     */
    if (parse_modifiers(word[1], &tmp_attr) != RC_OK) {
      return (RC_INVALID_OPERAND);
    }
    if (off == (-1)) {
      for (i = 0; i < ECOLOUR_MAX; i++) {
        attr = CURRENT_FILE->ecolour[i];
        if (modifier_set == COL_MODIFIER_SET_ON) {
          if (colour_support)
            attr.mod |= tmp_attr.mod;
          else
            attr.mono |= tmp_attr.mono;
        } else {
          if (colour_support)
            attr.mod &= ~tmp_attr.mod;
          else
            attr.mono &= ~tmp_attr.mono;
        }
        CURRENT_FILE->ecolour[i] = attr;
      }
    } else {
      area = ch - off;
      attr = CURRENT_FILE->ecolour[area];
      if (modifier_set == COL_MODIFIER_SET_ON) {
        if (colour_support)
          attr.mod |= tmp_attr.mod;
        else
          attr.mono |= tmp_attr.mono;
      } else {
        if (colour_support)
          attr.mod &= ~tmp_attr.mod;
        else
          attr.mono &= ~tmp_attr.mono;
      }
      CURRENT_FILE->ecolour[area] = attr;
    }
  } else {
    /*
     * Check that the supplied area matches one of the values in the area
     * array and that the length is at least as long as the minimum.
     */
    if (strlen((char *) word[0]) != 1) {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    ch = word[0][0];
    if (ch >= 'A' && ch <= 'Z')
      off = 'A';
    else if (ch >= 'a' && ch <= 'z')
      off = 'a';
    else if (ch >= '1' && ch <= '9')
      off = '1' - 26;           /* Beware: --x == +x */
    else {
      display_error(1, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
    area = ch - off;
    attr = CURRENT_FILE->ecolour[area];
    /*
     * Determine colours and modifiers.
     */
    if (parse_colours(word[1], &attr, &dummy, FALSE, &any_colours) != RC_OK) {
      return (RC_INVALID_OPERAND);
    }
    /*
     * Now we have the new colours, save them with the current file...
     */
    CURRENT_FILE->ecolour[area] = attr;
  }
  /*
   * If we haven't started curses (in profile first time) exit now...
   */
  if (!curses_started) {
    return (RC_OK);
  }
  /*
   * Update the appropriate window with the new colour combination...
   */
  if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
    display_screen((char_t) (other_screen));
  }
  display_screen(current_screen);
  return (RC_OK);
}
short Eolout(char_t * params) {
  char_t eolchar = 0;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "lf", params, 2))
    eolchar = EOLOUT_LF;
  else if (equal((char_t *) "cr", params, 2))
    eolchar = EOLOUT_CR;
  else if (equal((char_t *) "crlf", params, 4))
    eolchar = EOLOUT_CRLF;
  else if (equal((char_t *) "none", params, 4))
    eolchar = EOLOUT_NONE;
  else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (display_length != 0 && eolchar != EOLOUT_NONE) {
    display_error(0, (char_t *) "Warning: Setting EOLOUT will result in extra characters appended to each line on SAVE/FILE!", FALSE);
  }

  EOLx = CURRENT_FILE->eolout = eolchar;
  return (RC_OK);
}
short Equivchar(char_t * params) {
  short rc = RC_OK;

  /*
   * Must supply a parameter...
   */
  if (blank_field(params)) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * ... and it must be a single character
   */
  if (strlen((char *) params) > 1) {
    display_error(37, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Save it.
   */
  EQUIVCHARstr[0] = EQUIVCHARx = *(params);
  return (rc);
}
short Errorformat(char_t * params) {
  char_t errformat = 'N';

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "normal", params, 1))
    errformat = 'N';
  else if (equal((char_t *) "extended", params, 1))
    errformat = 'E';
  else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  ERRORFORMATx = errformat;
  return (RC_OK);
}
short Erroroutput(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &ERROROUTPUTx, TRUE);
  return (rc);
}
short Etmode(char_t * params) {
#define ETM_PARAMS  21
  char_t *word[ETM_PARAMS + 1];
  char_t strip[ETM_PARAMS];
  short num_params = 0;
  register short i = 0, j = 0;
  short rc = RC_OK;
  bool tmp_mode = FALSE;
  chtype attr = 0L;
  COLOUR_ATTR curr_attr;
  bool flags[256];
  int num = 0, num1 = 0;
  char_t *wptr = NULL, *wptr1 = NULL;

  for (i = 0; i < ETM_PARAMS; i++)
    strip[i] = STRIP_BOTH;
  num_params = param_split(params, word, ETM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  rc = execute_set_on_off(word[0], &tmp_mode, TRUE);
  if (rc != RC_OK) {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL)
    set_up_default_colours((FILE_DETAILS *) NULL, &curr_attr, ATTR_NONDISP);
  else
    memcpy(&curr_attr, CURRENT_FILE->attr + ATTR_NONDISP, sizeof(COLOUR_ATTR));
  attr = set_colour(&curr_attr);
  if (num_params == 1) {        /* absolute ON or OFF */
    if (tmp_mode) {             /* ETMODE ON */
      for (i = 0; i < 256; i++) {
        etmode_table[i] = i;
        etmode_flag[i] = FALSE;
      }
    } else {
      for (i = 0; i < 256; i++) {
        if (isprint(i)) {
          etmode_table[i] = i;
          etmode_flag[i] = FALSE;
        } else if (iscntrl(i)) {
          etmode_table[i] = ('@' + i) | attr;
          etmode_flag[i] = (attr) ? TRUE : FALSE;
        } else {
          etmode_table[i] = NONDISPx | attr;
          etmode_flag[i] = (attr) ? TRUE : FALSE;
        }
      }
    }
    if (number_of_files != 0) {
      build_screen(current_screen);
      display_screen(current_screen);
    }
    return (RC_OK);
  }
  memset(flags, FALSE, sizeof(flags));
  for (i = 1; i < num_params; i++) {
    if (valid_positive_integer(word[i])) {
      num = atoi((char *) word[i]);
      if (num > 255) {
        display_error(6, word[i], FALSE);
        return (RC_INVALID_OPERAND);
      }
      flags[num] = TRUE;
      continue;
    }
    num = strzeq(word[i], (char_t) '-');
    num1 = strzreveq(word[i], (char_t) '-');
    if (num != num1 || num == (-1)) {
      display_error(1, word[i], FALSE);
      return (RC_INVALID_OPERAND);
    }
    wptr = word[i];
    *(wptr + num) = '\0';
    wptr1 = wptr + num + 1;
    if (!valid_positive_integer(wptr)) {
      display_error(1, wptr, FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (!valid_positive_integer(wptr)) {
      display_error(1, wptr1, FALSE);
      return (RC_INVALID_OPERAND);
    }
    num = atoi((char *) wptr);
    num1 = atoi((char *) wptr1);
    if (num > num1) {
      display_error(1, word[i], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num > 255) {
      display_error(6, wptr, FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (num1 > 255) {
      display_error(6, wptr1, FALSE);
      return (RC_INVALID_OPERAND);
    }
    for (j = num; j < num1 + 1; j++) {
      flags[j] = TRUE;
    }
  }
  for (i = 0; i < 256; i++) {
    if (flags[i]) {
      etmode_table[i] = i;
      etmode_flag[i] = FALSE;
    } else {
      if (iscntrl(i)) {
        etmode_table[i] = ('@' + i) | attr;
        etmode_flag[i] = TRUE;
      } else {
        etmode_table[i] = NONDISPx | attr;
        etmode_flag[i] = TRUE;
      }
    }
  }
  if (number_of_files != 0) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}
short Fext(char_t * params) {
  char_t tmp_name[MAX_FILE_NAME + 1];
  short rc = RC_OK;
  int last_period = 0;

  /*
   * If a pseudo file is being changed, then error...
   */
  if (CURRENT_FILE->pseudo_file) {
    display_error(8, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  strcpy((char *) tmp_name, (char *) CURRENT_FILE->fpath);
  strcat((char *) tmp_name, (char *) CURRENT_FILE->fname);
  last_period = strzreveq(CURRENT_FILE->fname, (char_t) '.');
  if (last_period == (-1)) {    /* no period */
    if (blank_field(params)) {  /* and no extension, return... */
      return (RC_OK);
    }
    strcat((char *) tmp_name, ".");  /* add a period */
  } else {
    tmp_name[strlen((char *) CURRENT_FILE->fpath) + last_period + 1] = '\0';
  }
  strcat((char *) tmp_name, (char *) params);
  /*
   * Split the new path supplied...
   */
  if ((rc = splitpath(strrmdup(strtrans(tmp_name, OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    display_error(10, tmp_name, FALSE);
    return (rc);
  }
  /*
   * If the path is NOT the same as already assigned, error...
   */
  if (strcmp((char *) sp_path, (char *) CURRENT_FILE->fpath) != 0) {
    display_error(1, params, FALSE);
    return (RC_OK);
  }
  /*
   * Check we don't already have this filename in the ring
   */
  if (is_file_in_ring(CURRENT_FILE->fpath, sp_fname)) {
    display_error(76, tmp_name, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the length of the new path is > the existing one,
   * free up any memory for the existing path and allocate some
   * more. Save the new path.
   */
  if (strlen((char *) sp_fname) > strlen((char *) CURRENT_FILE->fname)) {
    free(CURRENT_FILE->fname);
    if ((CURRENT_FILE->fname = (char_t *) malloc(strlen((char *) sp_fname))) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
  }
  strcpy((char *) CURRENT_FILE->fname, (char *) sp_fname);
  /*
   * Re-display the IDLINE
   */
  if (curses_started) {
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      show_heading((char_t) (other_screen));
    }
    show_heading(current_screen);
  }
  return (rc);
}
short Filename(char_t * params) {
  char_t tmp_name[MAX_FILE_NAME + 1];
  short rc = RC_OK;
  int i = 0, cnt = 0, len_params = 0;

  /*
   * Must supply a parameter...
   */
  if (blank_field(params)) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If a pseudo file is being changed, then error...
   */
  if (CURRENT_FILE->pseudo_file) {
    display_error(8, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If a = is specified...
   */
  if (equal(params, EQUIVCHARstr, 1)) {
    return (RC_OK);
  }
  /*
   * Find out how many = are specified...
   */
  len_params = strlen((char *) params);
  for (i = 0, cnt = 0; i < len_params; i++) {
    if (params[i] == EQUIVCHARx)
      cnt++;
  }
  if (cnt > 1) {
    display_error(1, params, FALSE);
    return (RC_OK);
  }
  /*
   * The filename can be quoted; so strip leading and trailing
   * double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  /*
   * If we do have a leading or trailing = then call the equivalent
   * SET FEXT or FNAME command...
   */
  if (cnt == 1) {
    if (params[0] == EQUIVCHARx) {
      strcpy((char *) tmp_name, (char *) params + 1);
      rc = Fext(tmp_name);
      return (rc);
    } else {
      if (params[len_params - 1] == EQUIVCHARx) {
        strcpy((char *) tmp_name, (char *) params);
        tmp_name[len_params - 1] = '\0';
        rc = Fname(tmp_name);
        return (rc);
      } else {
        display_error(1, params, FALSE);
        return (rc);
      }
    }
  }
  /*
   * To get here, no = was in the parameter...
   */
  strcpy((char *) tmp_name, (char *) CURRENT_FILE->fpath);
  strcat((char *) tmp_name, (char *) params);
  if ((rc = splitpath(strrmdup(strtrans(tmp_name, OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    display_error(10, tmp_name, FALSE);
    return (rc);
  }
  /*
   * If the resulting path is different to the current one, error.
   */
  if (strcmp((char *) sp_path, (char *) CURRENT_FILE->fpath) != 0) {
    display_error(8, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the file name is the same as already assigned, exit...
   */
  if (strcmp((char *) sp_fname, (char *) CURRENT_FILE->fname) == 0) {
    return (RC_OK);
  }
  /*
   * Check we don't already have this filename in the ring
   */
  if (is_file_in_ring(CURRENT_FILE->fpath, sp_fname)) {
    display_error(76, tmp_name, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the length of the new filename is > the existing one,
   * free up any memory for the existing name and allocate some
   * more. Save the new name.
   */
  if (strlen((char *) sp_fname) > strlen((char *) CURRENT_FILE->fname)) {
    free(CURRENT_FILE->fname);
    if ((CURRENT_FILE->fname = (char_t *) malloc(strlen((char *) sp_fname) + 1)) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
  }
  strcpy((char *) CURRENT_FILE->fname, (char *) sp_fname);
  /*
   * Re-display the IDLINE
   */
  if (curses_started) {
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      show_heading((char_t) (other_screen));
    }
    show_heading(current_screen);
  }
  return (rc);
}
short THEFiletabs(char_t * params) {
  short rc = RC_OK;
  bool save_filetabs = FILETABSx;

  rc = execute_set_on_off(params, &FILETABSx, TRUE);
  if (save_filetabs != FILETABSx) {
    if (FILETABSx && filetabs == (WINDOW *) NULL) {
      create_filetabs_window();
    } else if (!FILETABSx && filetabs != (WINDOW *) NULL) {
      delwin(filetabs);
      filetabs = (WINDOW *) NULL;
    }
    /*
     * To get here something has changed, so rebuild the windows and
     * display the screen.
     */
    set_screen_defaults();
    if (set_up_windows(current_screen) != RC_OK) {
      return (RC_OK);
    }
    /*
     * We now have CURRENT_VIEW and real screen sizes set, we can
     * calculate the CURLINE value.
     */
    CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      build_screen((char_t) (other_screen));
      if (curses_started)
        display_screen((char_t) (other_screen));
    }
    build_screen(current_screen);
    if (curses_started)
      display_screen(current_screen);
  }
  return (rc);
}
short Fmode(char_t * params) {
  short rc = RC_OK;

  /*
   * Not valid for Unix...
   */
  display_error(82, (char_t *) "FMODE", FALSE);
  rc = RC_INVALID_OPERAND;
  return (rc);
}
short Fname(char_t * params) {
  char_t tmp_name[MAX_FILE_NAME + 1];
  short rc = RC_OK;
  int last_period = 0;

  /*
   * Must supply a parameter...
   */
  if (blank_field(params)) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If a pseudo file is being changed, then error...
   */
  if (CURRENT_FILE->pseudo_file) {
    display_error(8, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * The filename can be quoted; so strip leading and trailing
   * double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  strcpy((char *) tmp_name, (char *) CURRENT_FILE->fpath);
  last_period = strzreveq(CURRENT_FILE->fname, (char_t) '.');
  if (last_period == (-1)) {    /* no period */
    strcat((char *) tmp_name, (char *) params);
  } else {
    int len = strlen((char *) CURRENT_FILE->fpath);
    int lenext = strlen((char *) CURRENT_FILE->fname) - last_period;

    strcat((char *) tmp_name, (char *) CURRENT_FILE->fname + last_period);
    meminsmem(tmp_name, params, strlen((char *) params), len, MAX_FILE_NAME + 1, len + lenext + 1);
  }
  /*
   * Split the new path supplied...
   */
  if ((rc = splitpath(strrmdup(strtrans(tmp_name, OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    display_error(10, tmp_name, FALSE);
    return (rc);
  }
  /*
   * If the resulting path is different to the current one, error.
   */
  if (strcmp((char *) sp_path, (char *) CURRENT_FILE->fpath) != 0) {
    display_error(8, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the file name is the same as already assigned, exit...
   */
  if (strcmp((char *) sp_fname, (char *) CURRENT_FILE->fname) == 0) {
    return (RC_OK);
  }
  /*
   * Check we don't already have this filename in the ring
   */
  if (is_file_in_ring(CURRENT_FILE->fpath, sp_fname)) {
    display_error(76, tmp_name, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the length of the new path is > the existing one,
   * free up any memory for the existing path and allocate some
   * more. Save the new path.
   */
  if (strlen((char *) sp_fname) > strlen((char *) CURRENT_FILE->fname)) {
    free(CURRENT_FILE->fname);
    if ((CURRENT_FILE->fname = (char_t *) malloc(strlen((char *) sp_fname))) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
  }
  strcpy((char *) CURRENT_FILE->fname, (char *) sp_fname);
  /*
   * Re-display the IDLINE
   */
  if (curses_started) {
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      show_heading((char_t) (other_screen));
    }
    show_heading(current_screen);
  }
  return (rc);
}
short Fpath(char_t * params) {
  char_t tmp_name[MAX_FILE_NAME + 1];
  short rc = RC_OK;

  /*
   * Must supply a parameter...
   */
  if (blank_field(params)) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If a pseudo file is being changed, then error...
   */
  if (CURRENT_FILE->pseudo_file) {
    display_error(8, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * The filename can be quoted; so strip leading and trailing
   * double quotes
   */
  params = MyStrip(params, STRIP_BOTH, '"');
  /*
   * Split the new path supplied...
   */
  if ((rc = splitpath(strrmdup(strtrans(params, OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    display_error(10, params, FALSE);
    return (rc);
  }
  /*
   * If a filename results, then the path name specified would conflict
   * with an existing file.
   */
  if (!blank_field(sp_fname)) {
    display_error(8, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the path is the same as already assigned, exit...
   */
  if (strcmp((char *) sp_path, (char *) CURRENT_FILE->fpath) == 0) {
    return (RC_OK);
  }
  /*
   * Check we don't already have this filename in the ring
   */
  if (is_file_in_ring(sp_path, CURRENT_FILE->fname)) {
    sprintf((char *) tmp_name, "%s%s", (char_t *) sp_path, CURRENT_FILE->fname);
    display_error(76, tmp_name, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If the length of the new path is > the existing one,
   * free up any memory for the existing path and allocate some
   * more. Save the new path.
   */
  if (strlen((char *) sp_path) > strlen((char *) CURRENT_FILE->fpath)) {
    free(CURRENT_FILE->fpath);
    if ((CURRENT_FILE->fpath = (char_t *) malloc(strlen((char *) sp_path))) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
  }
  strcpy((char *) CURRENT_FILE->fpath, (char *) sp_path);
  /*
   * Re-display the IDLINE
   */
  if (curses_started) {
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      show_heading((char_t) (other_screen));
    }
    show_heading(current_screen);
  }
  return (rc);
}

short Fullfname(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_FILE->display_actual_filename, TRUE);
  if (curses_started) {
    if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
      show_heading((char_t) (other_screen));
    }
    show_heading(current_screen);
  }
  return (rc);
}
short THEHeader(char_t * params) {
  short rc = RC_OK;

#define HEA_PARAMS  2
  char_t *word[HEA_PARAMS + 1];
  char_t strip[HEA_PARAMS];
  short num_params = 0;
  line_t save_syntax_headers = CURRENT_VIEW->syntax_headers, val = 0;
  bool on_or_off;
  int i;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, HEA_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Validate first parameter...
   */
  for (i = 0; thm[i].the_header_name != NULL; i++) {
    if (equal((char_t *) thm[i].the_header_name, word[0], thm[i].the_header_name_len)) {
      val = thm[i].the_header;
      break;
    }
  }
  if (val == 0) {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }

  rc = execute_set_on_off(word[1], &on_or_off, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }

  /*
   * Have a valid header, and a valid ON|OFF...
   */
  if (on_or_off)
    CURRENT_VIEW->syntax_headers |= val;
  else
    CURRENT_VIEW->syntax_headers &= ~val;

  if (CURRENT_VIEW->syntax_headers != save_syntax_headers) {
    build_screen(current_screen);
    display_screen(current_screen);
  }

  return (rc);
}
short Hex(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->hex, TRUE);
  return (rc);
}
short Hexdisplay(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &HEXDISPLAYx, TRUE);
  if (rc == RC_OK && curses_started)
    clear_statarea();
  return (rc);
}
short Hexshow(char_t * params) {
#define HEXS_PARAMS  2
  char_t *word[HEXS_PARAMS + 1];
  char_t strip[HEXS_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  short base = CURRENT_VIEW->hexshow_base;
  short off = CURRENT_VIEW->hexshow_off;
  bool hexshowsts = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, HEXS_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  rc = execute_set_on_off(word[0], &hexshowsts, TRUE);
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
   * If the HEXSHOW row (or the next row) is the same row as CURLINE and
   * it is being turned on, return ERROR.
   */
  if ((calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE)
       || calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) == calculate_actual_row(base, off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE) + 1)
      && hexshowsts) {
    display_error(64, (char_t *) "- same line as CURLINE", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  CURRENT_VIEW->hexshow_base = base;
  CURRENT_VIEW->hexshow_off = off;
  CURRENT_VIEW->hexshow_on = hexshowsts;
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  build_screen(current_screen);
  display_screen(current_screen);
  return (rc);
}
short Highlight(char_t * params) {
#define HIGH_PARAMS  2
  char_t *word[HIGH_PARAMS + 1];
  char_t strip[HIGH_PARAMS];
  short num_params = 0;
  short col1 = 0, col2 = 0;
  short rc = RC_OK;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, HIGH_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  switch (num_params) {
    case 1:
      if (equal((char_t *) "off", word[0], 3)) {
        CURRENT_VIEW->highlight = HIGHLIGHT_NONE;
        break;
      }
      if (equal((char_t *) "tagged", word[0], 3)) {
        CURRENT_VIEW->highlight = HIGHLIGHT_TAG;
        break;
      }
      if (equal((char_t *) "altered", word[0], 3)) {
        CURRENT_VIEW->highlight = HIGHLIGHT_ALT;
        break;
      }
      display_error(1, word[0], FALSE);
      rc = RC_INVALID_OPERAND;
      break;
    case 2:
    case 3:
      if (!equal((char_t *) "select", word[0], 3)) {
        display_error(1, word[0], FALSE);
        return (RC_INVALID_OPERAND);
      }
      if ((rc = validate_n_m(word[1], &col1, &col2)) != RC_OK) {
        return (rc);
      }
      CURRENT_VIEW->highlight = HIGHLIGHT_SELECT;
      CURRENT_VIEW->highlight_low = col1;
      CURRENT_VIEW->highlight_high = col2;
      break;
    default:
      display_error(1, word[0], FALSE);
      rc = RC_INVALID_OPERAND;
      break;
  }
  if (rc == RC_OK) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}
short Idline(char_t * params) {
  short rc = RC_OK;
  bool save_id_line = FALSE;

  save_id_line = CURRENT_VIEW->id_line;
  rc = execute_set_on_off(params, &CURRENT_VIEW->id_line, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }
  /*
   * If the new value of id_line is the same as before, exit now.
   */
  if (save_id_line == CURRENT_VIEW->id_line) {
    return (rc);
  }
  /*
   * Redefine the screen sizes...
   */
  set_screen_defaults();
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

short Impmacro(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->imp_macro, TRUE);
  return (rc);
}
short Impos(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->imp_os, TRUE);
  return (rc);
}
short Inputmode(char_t * params) {
  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "off", params, 3))
    CURRENT_VIEW->inputmode = INPUTMODE_OFF;
  else if (equal((char_t *) "full", params, 2))
    CURRENT_VIEW->inputmode = INPUTMODE_FULL;
  else if (equal((char_t *) "line", params, 2))
    CURRENT_VIEW->inputmode = INPUTMODE_LINE;
  else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  return (RC_OK);
}
short Insertmode(char_t * params) {
  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "off", params, 3))
    INSERTMODEx = FALSE;
  else if (equal((char_t *) "on", params, 2))
    INSERTMODEx = TRUE;
  else if (equal((char_t *) "toggle", params, 6))
    INSERTMODEx = (INSERTMODEx) ? FALSE : TRUE;
  else {
    display_error(1, (char_t *) params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (curses_started)
    draw_cursor(TRUE);
  return (RC_OK);
}
short THEInterface(char_t * params) {
  short rc = RC_OK;

  params = MyStrip(params, STRIP_BOTH, ' ');
  if (equal((char_t *) "classic", params, 7))
    INTERFACEx = INTERFACE_CLASSIC;
  else if (equal((char_t *) "cua", params, 3))
    INTERFACEx = INTERFACE_CUA;
  else {
    display_error(3, (char_t *) "", FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}
short Lastop(char_t * params) {
#define LOP_PARAMS  2
  char_t *word[LOP_PARAMS + 1];
  char_t strip[LOP_PARAMS];
  unsigned short num_params = 0;
  bool found = FALSE;
  int i;
  short rc = RC_OK;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, LOP_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return RC_INVALID_OPERAND;
  } else if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Find a match for the supplied lastop operand
   */
  for (i = 0; i < LASTOP_MAX; i++) {
    if (equal(lastop[i].command, word[0], lastop[i].min_len)) {
      found = TRUE;
      break;
    }
  }
  if (!found) {
    display_error(1, word[0], FALSE);
    rc = RC_INVALID_OPERAND;
  } else {
    rc = save_lastop(i, word[1]);
    if (rc != RC_OK) {
      display_error(30, (char_t *) "", FALSE);
      rc = RC_OUT_OF_MEMORY;
    }
  }
  return (rc);
}

short Lineflag(char_t * params) {
#define LF_PARAMS  4
  char_t *word[LF_PARAMS + 1];
  char_t strip[LF_PARAMS];
  line_t num_lines = 0L, true_line = 0L;
  char_t *save_params;
  short num_params = 0, num_flags;
  short rc = RC_OK;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  long save_target_type = TARGET_UNFOUND;
  TARGET target;
  bool num_lines_based_on_scope = FALSE, no_flag = FALSE;
  unsigned int new_flag = 2;
  unsigned int changed_flag = 2;
  unsigned int tag_flag = 2;
  int i, j;

  save_params = my_strdup(params);
  if (save_params == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_NONE;
  num_params = param_split(params, word, LF_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {        /* no params */
    display_error(3, (char_t *) "", FALSE);
    free(save_params);
    return (RC_INVALID_OPERAND);
  }
  for (i = 0; i < num_params; i++) {
    if (!equal((char_t *) "change", word[i], 3)
        && !equal((char_t *) "nochange", word[i], 4)
        && !equal((char_t *) "new", word[i], 3)
        && !equal((char_t *) "nonew", word[i], 5)
        && !equal((char_t *) "tag", word[i], 3)
        && !equal((char_t *) "notag", word[i], 5)) {
      no_flag = TRUE;
      break;
    }
  }
  /*
   * If we broke out of the above loop the first time, then
   * there are no valid flags specified
   */
  if (i == 0) {
    display_error(3, (char_t *) "", FALSE);
    free(save_params);
    return (RC_INVALID_OPERAND);
  }
  /*
   * If no_flag is set to FALSE, all parameters are flags; therefore
   * the target will be 1
   */
  if (no_flag == FALSE) {
    num_lines = 1L;
    true_line = get_true_line(TRUE);
    num_lines_based_on_scope = TRUE;
    num_flags = num_params;
  } else {
    if (i + 1 != num_params) {
      for (j = 0; j < i; j++) {
        strip[0] = STRIP_BOTH;
      }
      strip[i] = STRIP_NONE;
      num_params = param_split(params, word, i + 1, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
    }
    initialise_target(&target);
    if ((rc = validate_target(word[num_params - 1], &target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
      free_target(&target);
      free(save_params);
      return (rc);
    }
    switch (target.rt[0].target_type) {
      case TARGET_BLOCK_CURRENT:
        switch (MARK_VIEW->mark_type) {
          case M_STREAM:
          case M_WORD:
          case M_COLUMN:
            display_error(49, (char_t *) "", FALSE);
            free_target(&target);
            free(save_params);
            return (RC_INVALID_OPERAND);
            break;
          case M_BOX:
            display_error(48, (char_t *) "", FALSE);
            free_target(&target);
            free(save_params);
            return (RC_INVALID_OPERAND);
            break;
          default:
            break;
        }
        break;
      case TARGET_BLOCK_ANY:
        display_error(45, (char_t *) "", FALSE);
        free_target(&target);
        free(save_params);
        return (RC_INVALID_OPERAND);
        break;
      default:
        break;
    }
    num_lines = target.num_lines;
    true_line = target.true_line;
    save_target_type = target.rt[0].target_type;
    num_lines_based_on_scope = (save_target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
    free_target(&target);
    num_flags = num_params - 1;
  }
  for (i = 0; i < num_flags; i++) {
    if (equal((char_t *) "change", word[i], 3))
      changed_flag = 1;
    else if (equal((char_t *) "nochange", word[i], 4))
      changed_flag = 0;
    else if (equal((char_t *) "new", word[i], 3))
      new_flag = 1;
    else if (equal((char_t *) "nonew", word[i], 5))
      new_flag = 0;
    else if (equal((char_t *) "tag", word[i], 3))
      tag_flag = 1;
    else if (equal((char_t *) "notag", word[i], 5))
      tag_flag = 0;
    else;
  }
  /*
   * Now we are here, everything's OK, do the actual modification...
   */
  rc = execute_set_lineflag(new_flag, changed_flag, tag_flag, true_line, num_lines, num_lines_based_on_scope, save_target_type);
  free(save_params);
  return (rc);
}
short Linend(char_t * params) {
#define LE_PARAMS  2
  char_t *word[LE_PARAMS + 1];
  char_t strip[LE_PARAMS];
  unsigned short num_params = 0;
  bool le_status = CURRENT_VIEW->linend_status;
  char_t le_value = CURRENT_VIEW->linend_value;
  short rc = RC_OK;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, LE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
    case 1:
    case 2:
      rc = execute_set_on_off(word[0], &le_status, TRUE);
      if (rc != RC_OK) {
        display_error(1, word[0], FALSE);
        rc = RC_INVALID_OPERAND;
        break;
      }
      if (num_params == 1)
        break;
      if ((int) strlen((char *) word[1]) > (int) 1) {
        display_error(1, word[1], FALSE);
        break;
      }
      le_value = word[1][0];
      break;
    case 0:
      display_error(3, (char_t *) "", FALSE);
      rc = RC_INVALID_OPERAND;
      break;
    default:
      display_error(2, (char_t *) "", FALSE);
      rc = RC_INVALID_OPERAND;
      break;
  }
  if (rc == RC_OK) {
    CURRENT_VIEW->linend_status = le_status;
    CURRENT_VIEW->linend_value = le_value;
  }
  return (rc);
}
short SetMacro(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->macro, TRUE);
  return (rc);
}
short Macroext(char_t * params) {
  /*
   * If no value is specified for ext, set the value of macro_suffix to
   * "", otherwise set it to the supplied value, prefixed with '.'
   */
  if (strlen((char *) params) == 0)
    strcpy((char *) macro_suffix, "");
  else {
    if ((int) strlen((char *) params) > (int) 10) {
      display_error(85, (char_t *) params, FALSE);
      return (RC_INVALID_OPERAND);
    }
    strcpy((char *) macro_suffix, ".");
    strcat((char *) macro_suffix, (char *) params);
  }
  return (RC_OK);
}
short Macropath(char_t * params) {
#define PATH_DELIM ':'
  register int len = 0;
  int num_dirs, i;
  char *ptr = NULL;
  char_t *src;

  /*
   * No checking is done on macro path supplied other than it contains a
   * value. Path delimiters are translated if necessary.
   */
  if (strlen((char *) params) == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (strcasecmp((char *) params, "PATH") == 0)
    src = (char_t *) getenv("PATH");
  else
    src = params;
  strcpy((char *) the_macro_path_buf, (char *) src);
  strrmdup(strtrans(the_macro_path_buf, OSLASH, ISLASH), ISLASH, TRUE);
  strrmdup(the_macro_path_buf, PATH_DELIM, FALSE);
  len = strlen((char *) the_macro_path_buf);
  if (the_macro_path_buf[len - 1] == PATH_DELIM) {
    the_macro_path_buf[len - 1] = '\0';
    len--;
  }
  if (len == 0) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  strcpy((char *) the_macro_path, (char *) the_macro_path_buf);
  /*
   * Count the number of PATH_DELIM in the buffer to determine
   * how many pointers to allocate.
   */
  for (num_dirs = 1, i = 0; i < len; i++) {
    if (*(the_macro_path + i) == PATH_DELIM)
      num_dirs++;
  }
  /*
   * If we have already allocated enough pointer memory
   * don't bother allocating more
   */
  if (num_dirs > total_macro_dirs) {
    if (the_macro_dir == NULL)
      the_macro_dir = (char_t **) malloc(num_dirs * sizeof(char *));
    else
      the_macro_dir = (char_t **) realloc(the_macro_dir, num_dirs * sizeof(char *));
    if (the_macro_dir == NULL) {
      max_macro_dirs = total_macro_dirs = 0;
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    total_macro_dirs = num_dirs;
  }
  the_macro_dir[0] = the_macro_path_buf;
  max_macro_dirs = 0;
  for (ptr = (char *) the_macro_path_buf; *ptr != '\0'; ptr++) {
    if (*ptr == PATH_DELIM) {
      *ptr = '\0';
      the_macro_dir[++max_macro_dirs] = (char_t *)++ ptr;
    }
  }
  max_macro_dirs++;
  return (RC_OK);
}
short Margins(char_t * params) {
#define MAR_PARAMS  3
  char_t *word[MAR_PARAMS + 1];
  char_t strip[MAR_PARAMS];
  short num_params = 0;
  length_t left = 0, right = 0, indent = 0;
  bool offset = FALSE, consistancy_error = FALSE;

  /*
   * Two parameters are mandatory, the third is optional.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, MAR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 3) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the parameters...
   */
  if (equal(EQUIVCHARstr, word[0], 1)) {
    left = CURRENT_VIEW->margin_left;
  } else {
    left = atol((char *) word[0]);
    if (left < 1) {
      display_error(5, word[0], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Right margin value can be *, set to maximum line length.
   */
  if (equal(EQUIVCHARstr, word[1], 1)) {
    right = CURRENT_VIEW->margin_right;
  } else if (equal((char_t *) "*", word[1], 1)) {
    right = max_line_length;
  } else {
    right = atol((char *) word[1]);
    if (right > max_line_length) {
      display_error(6, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
    if (right < 1) {
      display_error(5, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * Left margin must be less than right margin.
   */
  if (right < left) {
    display_error(5, word[1], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Obtain current values for indent, in case they aren't changed by
   * the current command. (i.e. no third parameter)
   */
  indent = CURRENT_VIEW->margin_indent;
  offset = CURRENT_VIEW->margin_indent_offset_status;
  /*
   * Determine the type of offset for the indent value. If a sign is
   * specified, then the number supplied is relative to the left margin
   * otherwise it is an absolute column value.
   * Do the following processing only if the indent parameter is NOT
   * the EQUIVCHAR.
   */
  if (num_params == 3 && !equal(EQUIVCHARstr, word[2], 1)) {
    if (*(word[2]) == '-' || *(word[2]) == '+') {
      offset = TRUE;
      if ((indent = atol((char *) word[2])) == 0) {
        if (strcmp((char *) word[2], "+0") != 0) {
          display_error(1, word[2], FALSE);
          return (RC_INVALID_OPERAND);
        }
      }
    } else {
      offset = FALSE;
      /*
       * Absolute indent cannot be negative.
       */
      if ((indent = atol((char *) word[2])) < 0) {
        display_error(1, word[2], FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
  }
  /*
   * Once all values are determined, validate the relationship between
   * the margins and the indent values.
   * Rules:
   *       o If indent is a negative offset, the resultant column value
   *         cannot be negative.
   *       o If indent is a positive offset, the resultant column value
   *         cannot be > max_line_length or right margin
   *       o If indent is an absolute value, it cannot be > right margin
   */
  consistancy_error = FALSE;
  if (offset && indent < 0 && indent + left < 0)
    consistancy_error = TRUE;
  if (offset && indent > 0 && indent + left > right)
    consistancy_error = TRUE;
  if (offset && indent > 0 && (length_t) (indent + left) > max_line_length)
    consistancy_error = TRUE;
  if (!offset && indent > right)
    consistancy_error = TRUE;
  if (consistancy_error) {
    if (offset)
      sprintf((char *) temp_cmd, "%ld %ld %+ld", left, right, indent);
    else
      sprintf((char *) temp_cmd, "%ld %ld %ld", left, right, indent);
    display_error(12, temp_cmd, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * All OK, so save the values...
   */
  CURRENT_VIEW->margin_left = left;
  CURRENT_VIEW->margin_right = right;
  CURRENT_VIEW->margin_indent = indent;
  CURRENT_VIEW->margin_indent_offset_status = offset;
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
short Mouse(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &MOUSEx, TRUE);
  mousemask((MOUSEx) ? ALL_MOUSE_EVENTS : 0, (mmask_t *) NULL);
  return (rc);
}
short Msgline(char_t * params) {
#define MSG_PARAMS  5
  char_t *word[MSG_PARAMS + 1];
  char_t strip[MSG_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  short base = CURRENT_VIEW->msgline_base;
  short off = CURRENT_VIEW->msgline_off;
  int start_row;
  bool msgsts = FALSE;
  int num_lines = CURRENT_VIEW->msgline_rows;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_NONE;
  num_params = param_split(params, word, MSG_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  /*
   * If only 1 parameter, it must be CLEAR...
   */
  if (num_params == 1) {
    if (equal((char_t *) "CLEAR", word[0], 5)) {
      clear_msgline(-1);
      return (RC_OK);
    }
  }
  /*
   * more than 1 parameter or only parameter not CLEAR
   */
  if (num_params < 2) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 4) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  if (equal(EQUIVCHARstr, word[0], 1)) {
    msgsts = TRUE;
  } else {
    rc = execute_set_on_off(word[0], &msgsts, TRUE);
    if (rc != RC_OK) {
      return (rc);
    }
  }
  /*
   * ... only "ON" is allowed...
   */
  if (!msgsts) {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the position parameter...
   */
  if (num_params > 1) {
    if (equal(EQUIVCHARstr, word[1], 1)) {
      base = CURRENT_VIEW->msgline_base;
      off = CURRENT_VIEW->msgline_off;
    } else {
      rc = execute_set_row_position(word[1], &base, &off);
      if (rc != RC_OK) {
        return (rc);
      }
    }
  }
  /*
   * To get here we have either two arguments or one. If two, the first
   * is the number of lines, and the second MUST be Overlay.
   * If one argument, it is either Overlay or number of lines.
   */
  switch (num_params) {
    case 3:
      if (equal((char_t *) "overlay", word[2], 1))
        num_lines = 1;
      else if (equal((char_t *) EQUIVCHARstr, word[2], 1))
        num_lines = CURRENT_VIEW->msgline_rows;
      else if (equal((char_t *) "*", word[2], 1))
        num_lines = 0;
      else {
        num_lines = atoi((char *) word[2]);
        if (num_lines < 1) {
          display_error(5, word[2], FALSE);
          return (rc);
        }
        start_row = calculate_actual_row(base, off, CURRENT_SCREEN.screen_rows, TRUE);
        if (base == POSITION_BOTTOM) {
          if (num_lines > start_row)
            rc = RC_INVALID_OPERAND;
        } else {
          if (start_row + num_lines > CURRENT_SCREEN.screen_rows)
            rc = RC_INVALID_OPERAND;
        }
        if (rc == RC_INVALID_OPERAND) {
          display_error(6, word[2], FALSE);
          return (rc);
        }
      }
      break;
    case 4:
      if (equal((char_t *) EQUIVCHARstr, word[2], 1))
        num_lines = CURRENT_VIEW->msgline_rows;
      else if (equal((char_t *) "*", word[2], 1))
        num_lines = 0;
      else {
        num_lines = atoi((char *) word[2]);
        if (num_lines < 1) {
          display_error(5, word[2], FALSE);
          return (rc);
        }
        start_row = calculate_actual_row(base, off, CURRENT_SCREEN.screen_rows, TRUE);
        if (base == POSITION_BOTTOM) {
          if (num_lines > start_row)
            rc = RC_INVALID_OPERAND;
        } else {
          if (start_row + num_lines > CURRENT_SCREEN.screen_rows)
            rc = RC_INVALID_OPERAND;
        }
        if (rc == RC_INVALID_OPERAND) {
          display_error(6, word[2], FALSE);
          return (rc);
        }
      }
      if (!equal((char_t *) "overlay", word[3], 1)
          && !equal((char_t *) EQUIVCHARstr, word[3], 1)) {
        display_error(1, word[3], FALSE);
        return (rc);
      }
      break;
    default:
      num_lines = 1;
      break;
  }
  CURRENT_VIEW->msgline_base = base;
  CURRENT_VIEW->msgline_off = off;
  CURRENT_VIEW->msgline_rows = num_lines;
  return (rc);
}
short Msgmode(char_t * params) {
#define MSGM_PARAMS  2
  char_t *word[MSGM_PARAMS + 1];
  char_t strip[MSGM_PARAMS];
  short num_params = 0;
  short rc = RC_OK;
  bool new_msgmode = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, MSGM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Parse the status parameter...
   */
  rc = execute_set_on_off(word[0], &new_msgmode, TRUE);
  if (rc != RC_OK) {
    return (rc);
  }
  /*
   * If present the second argument is validated but ignored...
   */
  if (num_params > 1) {
    if (equal((char_t *) "SHORT", word[1], 1)) {
      /*
       * Ignore
       */
    } else if (equal((char_t *) "LONG", word[1], 1)) {
      /*
       * Ignore
       */
    } else {
      display_error(1, word[1], FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  CURRENT_VIEW->msgmode_status = new_msgmode;
  return (RC_OK);
}
short Newlines(char_t * params) {
#define NEW_PARAMS  1
  char_t parm[NEW_PARAMS];
  char_t *word[NEW_PARAMS + 1];
  char_t strip[NEW_PARAMS];
  unsigned short num_params = 0;

  strip[0] = STRIP_BOTH;
  num_params = param_split(params, word, NEW_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params > 1) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }

  parm[0] = (char_t) UNDEFINED_OPERAND;
  if (equal((char_t *) "aligned", word[0], 1))
    parm[0] = TRUE;
  if (equal((char_t *) "left", word[0], 1))
    parm[0] = FALSE;
  if (parm[0] == (char_t) UNDEFINED_OPERAND) {
    display_error(1, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  CURRENT_VIEW->newline_aligned = parm[0];
  return (RC_OK);
}
short Nondisp(char_t * params) {
  if (strlen((char *) params) != 1) {
    display_error(1, params, FALSE);
    return (RC_INVALID_OPERAND);
  }
  NONDISPx = *params;
  build_screen(current_screen);
  display_screen(current_screen);
  return (RC_OK);
}
short Number(char_t * params) {
  short rc = RC_OK;

  rc = execute_set_on_off(params, &CURRENT_VIEW->number, TRUE);
  if (rc == RC_OK) {
    build_screen(current_screen);
    display_screen(current_screen);
  }
  return (rc);
}
