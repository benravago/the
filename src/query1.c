// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

#define _declare_ extern
#include "query.h"


static LINE *curr;

short extract_after_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;
  bool bool_flag = FALSE;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
  } else {
    x = getcurx(CURRENT_WINDOW);
    bool_flag = FALSE;
    switch (CURRENT_VIEW->current_window) {
      case WINDOW_FILEAREA:
        if ((x + CURRENT_VIEW->verify_col - 1) >= rec_len) {
          bool_flag = TRUE;
        }
        break;
      case WINDOW_COMMAND:
        if (x >= cmd_rec_len) {
          bool_flag = TRUE;
        }
        break;
      case WINDOW_PREFIX:
        if (x >= pre_rec_len) {
          bool_flag = TRUE;
        }
        break;
    }
    set_boolean_value(bool_flag, 1);
  }
  return 1;                     /* number of values set */
}

short extract_alt(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  sprintf((char *) query_num1, "%d", CURRENT_FILE->autosave_alt);
  sprintf((char *) query_num2, "%d", CURRENT_FILE->save_alt);
  item_values[1].value = query_num1;
  item_values[2].value = query_num2;
  item_values[1].len = strlen((char *) query_num1);
  item_values[2].len = strlen((char *) query_num2);
  return number_variables;
}

short extract_alt_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_FILE->save_alt != 0), (short) 1);
}

short extract_altkey_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int shift = 0;

  get_key_name(lastkeys[current_key], &shift);
  return set_boolean_value((bool) (shift & SHIFT_ALT), (short) 1);
}

short extract_batch_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) batch_only, (short) 1);
}

short extract_arbchar(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  set_on_off_value(CURRENT_VIEW->arbchar_status, 1);
  query_num1[0] = CURRENT_VIEW->arbchar_multiple;
  query_num1[1] = '\0';
  item_values[2].value = query_num1;
  item_values[2].len = 1;
  query_num2[0] = CURRENT_VIEW->arbchar_single;
  query_num2[1] = '\0';
  item_values[3].value = query_num2;
  item_values[3].len = 1;
  return number_variables;
}

short extract_autosave(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_FILE->autosave == 0) {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
  } else {
    sprintf((char *) query_num1, "%d", CURRENT_FILE->autosave);
    item_values[1].value = query_num1;
    item_values[1].len = strlen((char *) query_num1);
  }
  return number_variables;
}

short extract_autocolor(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_autocolour_settings(itemno, query_rsrvd, query_type, itemargs, TRUE);
}

short extract_autocolour(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_autocolour_settings(itemno, query_rsrvd, query_type, itemargs, FALSE);
}

short extract_autoscroll(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_VIEW->autoscroll == 0) {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
  } else if (CURRENT_VIEW->autoscroll == (-1)) {
    item_values[1].value = (uchar *) "HALF";
    item_values[1].len = 4;
  } else {
    sprintf((char *) query_num1, "%ld", CURRENT_VIEW->autoscroll);
    item_values[1].value = query_num1;
    item_values[1].len = strlen((char *) query_num1);
  }
  return number_variables;
}

short extract_backup(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (CURRENT_FILE->backup) {
    case BACKUP_OFF:
      item_values[1].value = (uchar *) "OFF";
      break;
    case BACKUP_TEMP:
      item_values[1].value = (uchar *) "TEMP";
      break;
    case BACKUP_KEEP:
      item_values[1].value = (uchar *) "KEEP";
      break;
    case BACKUP_INPLACE:
      item_values[1].value = (uchar *) "INPLACE";
      break;
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  strcpy((char *) query_rsrvd, (char *) BACKUP_SUFFIXx);
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) item_values[2].value);
  return number_variables;
}

short extract_beep(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(BEEPx, 1);
}

short extract_before_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;
  bool bool_flag = FALSE;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
  } else {
    x = getcurx(CURRENT_WINDOW);
    bool_flag = FALSE;
    switch (CURRENT_VIEW->current_window) {
      case WINDOW_FILEAREA:
        if ((x + CURRENT_VIEW->verify_col - 1) < memne(rec, ' ', rec_len)) {
          bool_flag = TRUE;
        }
        break;
      case WINDOW_COMMAND:
        if (x < memne(cmd_rec, ' ', cmd_rec_len)) {
          bool_flag = TRUE;
        }
        break;
      case WINDOW_PREFIX:      /* cursor can't go before 1st non-blank */
        break;
    }
    set_boolean_value((bool) bool_flag, (short) 1);
  }
  return number_variables;
}

short extract_blank_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) "0"; /* FALSE by default */
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      if (rec_len == 0) {
        item_values[1].value = (uchar *) "1";
      }
      break;
    case WINDOW_PREFIX:
      if (pre_rec_len == 0) {
        item_values[1].value = (uchar *) "1";
      }
      break;
    case WINDOW_COMMAND:
      if (cmd_rec_len == 0) {
        item_values[1].value = (uchar *) "1";
      }
      break;
  }
  item_values[1].len = 1;
  return number_variables;
}

short extract_block(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (MARK_VIEW == NULL) {
    item_values[1].value = (uchar *) "NONE";
    item_values[1].len = 4;
    number_variables = 1;
  } else {
    number_variables = 6;
    item_values[1].value = block_name[MARK_VIEW->mark_type];
    item_values[1].len = strlen((char *) item_values[1].value);
    sprintf((char *) query_num1, "%ld", MARK_VIEW->mark_start_line);
    item_values[2].value = query_num1;
    item_values[2].len = strlen((char *) query_num1);
    sprintf((char *) query_num2, "%ld", MARK_VIEW->mark_start_col);
    item_values[3].value = query_num2;
    item_values[3].len = strlen((char *) query_num2);
    sprintf((char *) query_num3, "%ld", MARK_VIEW->mark_end_line);
    item_values[4].value = query_num3;
    item_values[4].len = strlen((char *) query_num3);
    sprintf((char *) query_num4, "%ld", MARK_VIEW->mark_end_col);
    item_values[5].value = query_num4;
    item_values[5].len = strlen((char *) query_num4);
    strcpy((char *) trec, (char *) MARK_FILE->fpath);
    strcat((char *) trec, (char *) MARK_FILE->fname);
    item_values[6].value = (uchar *) trec;
    item_values[6].len = strlen((char *) trec);
  }
  return number_variables;
}

short extract_block_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_VIEW == MARK_VIEW), (short) 1);
}

short extract_bottomedge_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short y = 0;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  y = getcury(CURRENT_WINDOW);
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_FILEAREA && y == CURRENT_SCREEN.rows[WINDOW_FILEAREA] - 1), (short) 1);
}

short extract_case(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (CURRENT_VIEW->case_enter) {
    case CASE_MIXED:
      item_values[1].value = (uchar *) "MIXED";
      break;
    case CASE_UPPER:
      item_values[1].value = (uchar *) "UPPER";
      break;
    case CASE_LOWER:
      item_values[1].value = (uchar *) "LOWER";
      break;
    default:
      break;
  }
  item_values[1].len = 5;
  switch (CURRENT_VIEW->case_locate) {
    case CASE_IGNORE:
      item_values[2].value = (uchar *) "IGNORE";
      item_values[2].len = 6;
      break;
    case CASE_RESPECT:
      item_values[2].value = (uchar *) "RESPECT";
      item_values[2].len = 7;
      break;
    default:
      break;
  }
  switch (CURRENT_VIEW->case_change) {
    case CASE_IGNORE:
      item_values[3].value = (uchar *) "IGNORE";
      item_values[3].len = 6;
      break;
    case CASE_RESPECT:
      item_values[3].value = (uchar *) "RESPECT";
      item_values[3].len = 7;
      break;
    default:
      break;
  }
  switch (CURRENT_VIEW->case_sort) {
    case CASE_IGNORE:
      item_values[4].value = (uchar *) "IGNORE";
      item_values[4].len = 6;
      break;
    case CASE_RESPECT:
      item_values[4].value = (uchar *) "RESPECT";
      item_values[4].len = 7;
      break;
    default:
      break;
  }
  item_values[5].len = 5;
  switch (CURRENT_VIEW->case_enter_cmdline) {
    case CASE_MIXED:
      item_values[5].value = (uchar *) "MIXED";
      break;
    case CASE_UPPER:
      item_values[5].value = (uchar *) "UPPER";
      break;
    case CASE_LOWER:
      item_values[5].value = (uchar *) "LOWER";
      break;
    default:
      break;
  }
  item_values[6].len = 5;
  switch (CURRENT_VIEW->case_enter_prefix) {
    case CASE_MIXED:
      item_values[6].value = (uchar *) "MIXED";
      break;
    case CASE_UPPER:
      item_values[6].value = (uchar *) "UPPER";
      break;
    case CASE_LOWER:
      item_values[6].value = (uchar *) "LOWER";
      break;
    default:
      break;
  }
  return number_variables;
}

short extract_clearerrorkey(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int dummy = 0;

  if (CLEARERRORKEYx == -1) {
    item_values[1].value = (uchar *) "*";
    item_values[1].len = 1;
  } else {
    item_values[1].value = get_key_name(CLEARERRORKEYx, &dummy);
    item_values[1].len = strlen((char *) item_values[1].value);
  }
  return number_variables;
}

short extract_clearscreen(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CLEARSCREENx, 1);
}

short extract_clock(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CLOCKx, 1);
}

short extract_command_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_WINDOW_COMMAND != NULL), (short) 1);
}

short extract_cmdarrows(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CMDARROWSTABCMDx) {
    item_values[1].value = (uchar *) "TAB";
    item_values[1].len = 3;
  } else {
    item_values[1].value = (uchar *) "RETRIEVE";
    item_values[1].len = 8;
  }
  return number_variables;
}

short extract_cmdline(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (CURRENT_VIEW->cmd_line) {
    case 'B':
      item_values[1].value = (uchar *) "BOTTOM";
      item_values[1].len = 6;
      break;
    case 'T':
      item_values[1].value = (uchar *) "TOP";
      item_values[1].len = 3;
      break;
    case 'O':
      item_values[1].value = (uchar *) "OFF";
      item_values[1].len = 3;
      number_variables = 1;
      break;
  }
  if (CURRENT_VIEW->cmd_line == 'O') {
    return number_variables;
  }
  if (query_type == QUERY_EXTRACT || query_type == QUERY_FUNCTION) {
    sprintf((char *) query_num1, "%d", CURRENT_SCREEN.start_row[WINDOW_COMMAND] + 1);
    item_values[2].value = query_num1;
    item_values[2].len = strlen((char *) query_num1);
    item_values[3].value = cmd_rec;
    item_values[3].len = cmd_rec_len;
    number_variables = 3;
  } else {
    number_variables = 1;
  }
  return number_variables;
}

short extract_color(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_colour_settings(itemno, query_rsrvd, query_type, itemargs, TRUE, FALSE);
}

short extract_colour(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_colour_settings(itemno, query_rsrvd, query_type, itemargs, FALSE, FALSE);
}

short extract_coloring(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_FILE->colouring) {
    item_values[1].value = (uchar *) "ON";
    item_values[1].len = 2;
    if (CURRENT_FILE->parser) {
      item_values[3].value = CURRENT_FILE->parser->parser_name;
      item_values[3].len = strlen((char *) item_values[3].value);
    } else {
      item_values[3].value = (uchar *) "NULL";
      item_values[3].len = 4;
    }
    if (CURRENT_FILE->autocolour) {
      item_values[2].value = (uchar *) "AUTO";
      item_values[2].len = 4;
    } else {
      item_values[2].value = CURRENT_FILE->parser->parser_name;
      item_values[2].len = strlen((char *) item_values[2].value);
    }
  } else {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
    number_variables = 1;
  }
  return number_variables;
}

short extract_colouring(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_FILE->colouring) {
    item_values[1].value = (uchar *) "ON";
    item_values[1].len = 2;
    if (CURRENT_FILE->parser) {
      item_values[3].value = CURRENT_FILE->parser->parser_name;
      item_values[3].len = strlen((char *) item_values[3].value);
    } else {
      item_values[3].value = (uchar *) "NULL";
      item_values[3].len = 4;
    }
    if (CURRENT_FILE->autocolour) {
      item_values[2].value = (uchar *) "AUTO";
      item_values[2].len = 4;
    } else {
      item_values[2].value = CURRENT_FILE->parser->parser_name;
      item_values[2].len = strlen((char *) item_values[2].value);
    }
  } else {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
    number_variables = 1;
  }
  return number_variables;
}

short extract_column(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;

  if (batch_only || CURRENT_VIEW->current_window != WINDOW_FILEAREA) {
    sprintf((char *) query_num1, "%ld", CURRENT_VIEW->current_column);
  } else {
    x = getcurx(CURRENT_WINDOW);
    sprintf((char *) query_num1, "%ld", x + CURRENT_VIEW->verify_col);
  }
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_compat(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (compatible_look) {
    case COMPAT_THE:
      item_values[1].value = (uchar *) "THE";
      item_values[1].len = 3;
      break;
    case COMPAT_XEDIT:
      item_values[1].value = (uchar *) "XEDIT";
      item_values[1].len = 5;
      break;
    case COMPAT_ISPF:
      item_values[1].value = (uchar *) "ISPF";
      item_values[1].len = 4;
      break;
    case COMPAT_KEDIT:
      item_values[1].value = (uchar *) "KEDIT";
      item_values[1].len = 5;
      break;
    case COMPAT_KEDITW:
      item_values[1].value = (uchar *) "KEDITW";
      item_values[1].len = 6;
      break;
  }
  switch (compatible_feel) {
    case COMPAT_THE:
      item_values[2].value = (uchar *) "THE";
      item_values[2].len = 3;
      break;
    case COMPAT_XEDIT:
      item_values[2].value = (uchar *) "XEDIT";
      item_values[2].len = 5;
      break;
    case COMPAT_ISPF:
      item_values[2].value = (uchar *) "ISPF";
      item_values[2].len = 4;
      break;
    case COMPAT_KEDIT:
      item_values[2].value = (uchar *) "KEDIT";
      item_values[2].len = 5;
      break;
    case COMPAT_KEDITW:
      item_values[2].value = (uchar *) "KEDITW";
      item_values[2].len = 6;
      break;
  }
  switch (compatible_keys) {
    case COMPAT_THE:
      item_values[3].value = (uchar *) "THE";
      item_values[3].len = 3;
      break;
    case COMPAT_XEDIT:
      item_values[3].value = (uchar *) "XEDIT";
      item_values[3].len = 5;
      break;
    case COMPAT_ISPF:
      item_values[3].value = (uchar *) "ISPF";
      item_values[3].len = 4;
      break;
    case COMPAT_KEDIT:
      item_values[3].value = (uchar *) "KEDIT";
      item_values[3].len = 5;
      break;
    case COMPAT_KEDITW:
      item_values[3].value = (uchar *) "KEDITW";
      item_values[3].len = 6;
      break;
  }
  return number_variables;
}

short extract_ctlchar(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int i, j;
  bool found = FALSE;

  if (!CTLCHARx) {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
    number_variables = 1;
  } else {
    if (itemargs == NULL || blank_field(itemargs) || strcmp((char *) itemargs, "*") == 0) {
      item_values[1].value = (uchar *) "ON";
      item_values[1].len = 2;
      query_num2[0] = ctlchar_escape;
      query_num2[1] = '\0';
      item_values[2].value = (uchar *) query_num2;
      item_values[2].len = 1;
      memset(query_rsrvd, ' ', MAX_CTLCHARS * 2);
      for (i = 0, j = 0; i < MAX_CTLCHARS; i++) {
        if (ctlchar_char[i] != 0) {
          query_rsrvd[j * 2] = ctlchar_char[i];
          j++;
        }
      }
      query_rsrvd[(j * 2) - 1] = '\0';
      item_values[3].value = query_rsrvd;
      item_values[3].len = strlen((char *) query_rsrvd);
      number_variables = 3;
    } else {
      /*
       * Find the matching CTLCHAR
       */
      for (i = 0; i < MAX_CTLCHARS; i++) {
        if (ctlchar_char[i] == itemargs[0]) {
          uchar *attr_string;

          item_values[1].value = (ctlchar_protect[i]) ? (uchar *) "PROTECT" : (uchar *) "NOPROTECT";
          item_values[1].len = strlen((char *) item_values[1].value);
          attr_string = get_colour_strings(&ctlchar_attr[i]);
          strcpy((char *) query_rsrvd, (char *) attr_string);
          item_values[2].value = query_rsrvd;
          item_values[2].len = strlen((char *) item_values[2].value);
          free(attr_string);
          number_variables = 2;
          found = TRUE;
        }
      }
      if (found == FALSE) {
        display_error(1, (uchar *) itemargs, FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      }
    }
  }
  return number_variables;
}

short extract_ctrl_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int shift = 0;

  get_key_name(lastkeys[current_key], &shift);
  return set_boolean_value((bool) (shift & SHIFT_CTRL), (short) 1);
}

short extract_curline(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_VIEW->current_base == POSITION_MIDDLE) {
    if (CURRENT_VIEW->current_off == 0) {
      strcpy((char *) query_rsrvd, "M");
    } else {
      sprintf((char *) query_rsrvd, "M%+d", CURRENT_VIEW->current_off);
    }
  } else {
    sprintf((char *) query_rsrvd, "%d", CURRENT_VIEW->current_off);
  }
  item_values[1].value = query_rsrvd;
  item_values[1].len = strlen((char *) query_rsrvd);
  if (query_type == QUERY_EXTRACT || query_type == QUERY_FUNCTION) {
    sprintf((char *) query_num1, "%d", CURRENT_VIEW->current_row + 1);
    item_values[2].value = query_num1;
    item_values[2].len = strlen((char *) query_num1);
    curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE), CURRENT_FILE->number_lines);
    item_values[3].value = (uchar *) curr->line;
    item_values[3].len = curr->length;
    item_values[4].value = (curr->flags.new_flag || curr->flags.changed_flag) ? (uchar *) "ON" : (uchar *) "OFF";
    item_values[4].len = strlen((char *) item_values[4].value);
    if (curr->flags.new_flag) {
      if (curr->flags.changed_flag) {
        item_values[5].value = (uchar *) "NEW CHANGED";
      } else {
        item_values[5].value = (uchar *) "NEW";
      }
    } else {
      if (curr->flags.changed_flag) {
        item_values[5].value = (uchar *) "OLD CHANGED";
      } else {
        item_values[5].value = (uchar *) "OLD";
      }
    }
    item_values[5].len = strlen((char *) item_values[5].value);
    sprintf((char *) query_num2, "%d", curr->select);
    item_values[6].value = query_num2;
    item_values[6].len = strlen((char *) query_num2);
  } else {
    number_variables = 1;
  }
  return number_variables;
}

short extract_cursor(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  long current_screen_line = (-1L);
  long current_screen_column = (-1L);
  long current_file_line = (-1L);
  long current_file_column = (-1L);

  get_cursor_position(&current_screen_line, &current_screen_column, &current_file_line, &current_file_column);
  sprintf((char *) query_num1, "%ld", current_screen_line);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  sprintf((char *) query_num2, "%ld", current_screen_column);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  sprintf((char *) query_num3, "%ld", current_file_line);
  item_values[3].value = query_num3;
  item_values[3].len = strlen((char *) query_num3);
  sprintf((char *) query_num4, "%ld", current_file_column);
  item_values[4].value = query_num4;
  item_values[4].len = strlen((char *) query_num4);
  sprintf((char *) query_num5, "%ld", original_screen_line);
  item_values[5].value = query_num5;
  item_values[5].len = strlen((char *) query_num5);
  sprintf((char *) query_num6, "%ld", original_screen_column);
  item_values[6].value = query_num6;
  item_values[6].len = strlen((char *) query_num6);
  sprintf((char *) query_num7, "%ld", original_file_line);
  item_values[7].value = query_num7;
  item_values[7].len = strlen((char *) query_num7);
  sprintf((char *) query_num8, "%ld", original_file_column);
  item_values[8].value = query_num8;
  item_values[8].len = strlen((char *) query_num8);
  return number_variables;
}

short extract_cursorstay(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(scroll_cursor_stay, 1);
}

short extract_current_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_VIEW->current_window != WINDOW_COMMAND && CURRENT_VIEW->focus_line == CURRENT_VIEW->current_line), (short) 1);
}

#define DEF_MOUSE_PARAMS  4

short extract_define(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  char buf[20];
  int len, number_keys;
  uchar *keydef;
  int key;

  uchar *word[DEF_MOUSE_PARAMS + 1];
  uchar strip[DEF_MOUSE_PARAMS];
  unsigned short num_params = 0;
  bool mouse_key = FALSE;

  if (itemargs == NULL || blank_field(itemargs) || strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_EXTRACT) {
      if (set_rexx_variables_for_all_keys(KEY_TYPE_ALL, &number_keys) != RC_OK) {
        display_error(54, (uchar *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else {
        len = sprintf(buf, "%d", number_keys);
        set_rexx_variable((uchar *) "define", (uchar *) buf, len, 0);
        number_variables = EXTRACT_VARIABLES_SET;
      }
    } else {
      number_variables = EXTRACT_ARG_ERROR;
    }
  } else if (equal((uchar *) "KEY", itemargs, 3)) {
    if (query_type == QUERY_EXTRACT) {
      if (set_rexx_variables_for_all_keys(KEY_TYPE_KEY, &number_keys) != RC_OK) {
        display_error(54, (uchar *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else {
        len = sprintf(buf, "%d", number_keys);
        set_rexx_variable((uchar *) "define", (uchar *) buf, len, 0);
        number_variables = EXTRACT_VARIABLES_SET;
      }
    } else {
      number_variables = EXTRACT_ARG_ERROR;
    }
  } else if (equal((uchar *) "MOUSE", itemargs, 5)) {
    if (query_type == QUERY_EXTRACT) {
      if (set_rexx_variables_for_all_keys(KEY_TYPE_MOUSE, &number_keys) != RC_OK) {
        display_error(54, (uchar *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else {
        len = sprintf(buf, "%d", number_keys);
        set_rexx_variable((uchar *) "define", (uchar *) buf, len, 0);
        number_variables = EXTRACT_VARIABLES_SET;
      }
    } else {
      number_variables = EXTRACT_ARG_ERROR;
    }
  } else {
    /*
     * Find the key value for the named key
     */
    key = find_key_name(itemargs);
    if (key == -1) {
      /*
       * It's not a key; is it a mouse definition?
       */
      strip[0] = STRIP_BOTH;
      strip[1] = STRIP_BOTH;
      strip[2] = STRIP_BOTH;
      strip[3] = STRIP_NONE;
      num_params = param_split(itemargs, word, DEF_MOUSE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
      if (num_params < 3) {
        display_error(3, (uchar *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else if (!equal((uchar *) "in", word[1], 2)) {
        display_error(1, itemargs, FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else if ((key = find_mouse_key_value_in_window(word[0], word[2])) == (-1)) {
        display_error(1, (uchar *) itemargs, FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else {
        mouse_key = TRUE;
      }
    } else {
      mouse_key = FALSE;
    }
    if (key != -1) {
      if (query_type == QUERY_EXTRACT) {
        keydef = get_key_definition(key, THE_KEY_DEFINE_SHOW, FALSE, mouse_key);
      } else {
        keydef = get_key_definition(key, THE_KEY_DEFINE_QUERY, FALSE, mouse_key);
      }
      strcpy((char *) query_rsrvd, (char *) keydef);
      item_values[1].value = query_rsrvd;
      item_values[1].len = strlen((char *) item_values[1].value);
      number_variables = 1;
    } else {
      display_error(1, (uchar *) itemargs, FALSE);
      number_variables = EXTRACT_ARG_ERROR;
    }
  }
  return number_variables;
}

short extract_defsort(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (DEFSORTx) {
    case DIRSORT_DIR:
      item_values[1].value = (uchar *) "DIRECTORY";
      break;
    case DIRSORT_NAME:
      item_values[1].value = (uchar *) "NAME";
      break;
    case DIRSORT_SIZE:
      item_values[1].value = (uchar *) "SIZE";
      break;
    case DIRSORT_TIME:
      item_values[1].value = (uchar *) "TIME";
      break;
    case DIRSORT_DATE:
      item_values[1].value = (uchar *) "DATE";
      break;
    case DIRSORT_NONE:
      item_values[1].value = (uchar *) "OFF";
      break;
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  if (DIRORDERx == DIRSORT_ASC) {
    item_values[2].value = (uchar *) "ASCENDING";
    item_values[2].len = 9;
  } else {
    item_values[2].value = (uchar *) "DESCENDING";
    item_values[2].len = 10;
  }
  return number_variables;
}

short extract_dir_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_FILE->pseudo_file == PSEUDO_DIR), (short) 1);
}

short extract_dirfileid(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  long true_line = (-1L);

  if (CURRENT_FILE->pseudo_file == PSEUDO_DIR) {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      if (CURRENT_TOF || CURRENT_BOF) {
        true_line = (-1L);
      } else {
        true_line = CURRENT_VIEW->current_line;
      }
    } else {
      if (FOCUS_TOF || FOCUS_BOF) {
        true_line = (-1L);
      } else {
        true_line = CURRENT_VIEW->focus_line;
      }
    }
  }
  if (true_line == (-1L)) {
    item_values[1].value = (uchar *) dir_path;
    item_values[1].len = strlen((char *) dir_path);
    number_variables = 1;
  } else {
    curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
    item_values[1].value = (uchar *) dir_path;
    item_values[1].len = strlen((char *) dir_path);
    if (curr->length < FILE_START) {
      item_values[2].value = (uchar *) "";
      item_values[2].len = 0;
    } else {
      item_values[2].value = (uchar *) curr->line + FILE_START;
      item_values[2].len = strlen((char *) curr->line + FILE_START);
    }
  }
  return number_variables;
}

short extract_ecolor(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_colour_settings(itemno, query_rsrvd, query_type, itemargs, TRUE, TRUE);
}

short extract_ecolour(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return extract_colour_settings(itemno, query_rsrvd, query_type, itemargs, FALSE, TRUE);
}

short extract_end_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  item_values[1].value = (uchar *) "0"; /* set FALSE by default */
  x = getcurx(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      if (x + CURRENT_VIEW->verify_col == rec_len) {
        item_values[1].value = (uchar *) "1";
      }
      break;
    case WINDOW_PREFIX:
      if (pre_rec_len > 0 && pre_rec_len - 1 == x) {
        item_values[1].value = (uchar *) "1";
      }
      break;
    case WINDOW_COMMAND:
      if (cmd_rec_len > 0 && cmd_rec_len - 1 == x) {
        item_values[1].value = (uchar *) "1";
      }
      break;
  }
  item_values[1].len = 1;
  return number_variables;
}

short extract_display(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  sprintf((char *) query_num1, "%d", CURRENT_VIEW->display_low);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  sprintf((char *) query_num2, "%d", CURRENT_VIEW->display_high);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  return number_variables;
}

short extract_eof(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value((bool) CURRENT_BOF, 1);
}

short extract_eof_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (FOCUS_BOF && CURRENT_VIEW->current_window != WINDOW_COMMAND), (short) 1);
}

short extract_eolout(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (CURRENT_FILE->eolout) {
    case EOLOUT_LF:
      item_values[1].value = (uchar *) "LF";
      item_values[1].len = 2;
      break;
    case EOLOUT_CR:
      item_values[1].value = (uchar *) "CR";
      item_values[1].len = 2;
      break;
    case EOLOUT_CRLF:
      item_values[1].value = (uchar *) "CRLF";
      item_values[1].len = 4;
      break;
    case EOLOUT_NONE:
      item_values[1].value = (uchar *) "NONE";
      item_values[1].len = 4;
      break;
  }
  return number_variables;
}

short extract_equivchar(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = EQUIVCHARstr;
  item_values[1].len = strlen((char *) EQUIVCHARstr);
  return number_variables;
}

short extract_errorformat(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (ERRORFORMATx) {
    case 'N':
      item_values[1].value = (uchar *) "NORMAL";
      item_values[1].len = 6;
      break;
    case 'E':
      item_values[1].value = (uchar *) "EXTENDED";
      item_values[1].len = 8;
      break;
    default:
      break;
  }
  return number_variables;
}

short extract_erroroutput(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(ERROROUTPUTx, 1);
}

short extract_etmode(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  get_etmode(query_num1, query_rsrvd);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_field(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int i = 0;
  unsigned short x = 0;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  x = getcurx(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      item_values[1].value = rec;
      item_values[1].len = rec_len;
      i = x + CURRENT_VIEW->verify_col;
      query_num1[0] = rec[i - 1];
      item_values[4].value = (uchar *) "TEXT";
      item_values[4].len = 4;
      break;
    case WINDOW_PREFIX:
      item_values[1].value = pre_rec;
      item_values[1].len = pre_rec_len;
      i = x + 1;
      query_num1[0] = pre_rec[i - 1];
      item_values[4].value = (uchar *) "PREFIX";
      item_values[4].len = 6;
      break;
    case WINDOW_COMMAND:
      item_values[1].value = cmd_rec;
      item_values[1].len = cmd_rec_len;
      i = x + 1;
      query_num1[0] = cmd_rec[i - 1];
      item_values[4].value = (uchar *) "COMMAND";
      item_values[4].len = 7;
      break;
  }
  query_num1[1] = '\0';
  item_values[2].value = query_num1;
  item_values[2].len = 1;
  sprintf((char *) query_num2, "%d", i);
  item_values[3].value = query_num2;
  item_values[3].len = strlen((char *) query_num2);
  return number_variables;
}

short extract_fieldword(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0, rc;
  long real_col = 0;
  long first_col, last_col, len = 0;
  uchar *ptr = NULL, save_word;
  long word_len;
  uchar *tmpbuf;

  x = getcurx(CURRENT_WINDOW);
  switch (CURRENT_VIEW->current_window) {
    case WINDOW_FILEAREA:
      ptr = rec;
      len = rec_len;
      real_col = x + CURRENT_VIEW->verify_col;
      break;
    case WINDOW_PREFIX:
      ptr = pre_rec;
      len = pre_rec_len;
      real_col = x + 1;
      break;
    case WINDOW_COMMAND:
      ptr = cmd_rec;
      len = cmd_rec_len;
      real_col = x + 1;
      break;
  }
  save_word = CURRENT_VIEW->word;
  /*
   * Get the word based on an alphanumeric word setting...
   */
  CURRENT_VIEW->word = 'A';
  if (len == 0 || get_fieldword(ptr, len, real_col - 1, &first_col, &last_col) == 0) {
    rc = set_rexx_variable(query_item[itemno].name, (uchar *) "", 0, 1);
  } else {
    word_len = (last_col - first_col) + 1;
    tmpbuf = (uchar *) malloc(sizeof(uchar) * (word_len + 1));
    if (tmpbuf == (uchar *) NULL) {
      display_error(30, (uchar *) "", FALSE);
      CURRENT_VIEW->word = save_word;
      return (EXTRACT_ARG_ERROR);
    }
    memcpy((char *) tmpbuf, (char *) ptr + first_col, word_len);
    tmpbuf[word_len] = '\0';
    rc = set_rexx_variable(query_item[itemno].name, tmpbuf, word_len, 1);
    free(tmpbuf);
  }
  if (rc == RC_SYSTEM_ERROR) {
    display_error(54, (uchar *) "", FALSE);
    CURRENT_VIEW->word = save_word;
    return (EXTRACT_ARG_ERROR);
  }
  /*
   * Get the word based on a non-blank word setting...
   */
  CURRENT_VIEW->word = 'N';
  if (len == 0 || get_fieldword(ptr, len, real_col - 1, &first_col, &last_col) == 0) {
    rc = set_rexx_variable(query_item[itemno].name, (uchar *) "", 0, 2);
  } else {
    word_len = (last_col - first_col) + 1;
    tmpbuf = (uchar *) malloc(sizeof(uchar) * (word_len + 1));
    if (tmpbuf == (uchar *) NULL) {
      display_error(30, (uchar *) "", FALSE);
      CURRENT_VIEW->word = save_word;
      return (EXTRACT_ARG_ERROR);
    }
    memcpy((char *) tmpbuf, (char *) ptr + first_col, word_len);
    tmpbuf[word_len] = '\0';
    rc = set_rexx_variable(query_item[itemno].name, tmpbuf, word_len, 2);
    free(tmpbuf);
  }
  if (rc == RC_SYSTEM_ERROR) {
    display_error(54, (uchar *) "", FALSE);
    CURRENT_VIEW->word = save_word;
    return (EXTRACT_ARG_ERROR);
  }
  /*
   * Set the starting column
   */
  word_len = sprintf((char *) query_num1, "%ld", first_col + 1);
  rc = set_rexx_variable(query_item[itemno].name, query_num1, word_len, 3);
  if (rc == RC_SYSTEM_ERROR) {
    display_error(54, (uchar *) "", FALSE);
    CURRENT_VIEW->word = save_word;
    return (EXTRACT_ARG_ERROR);
  }
  /*
   * Set the 0 tail
   */
  rc = set_rexx_variable(query_item[itemno].name, (uchar *) "3", 1, 0);
  if (rc == RC_SYSTEM_ERROR) {
    display_error(54, (uchar *) "", FALSE);
    CURRENT_VIEW->word = save_word;
    return (EXTRACT_ARG_ERROR);
  }
  CURRENT_VIEW->word = save_word;
  return EXTRACT_VARIABLES_SET;
}

short extract_first_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  x = getcurx(CURRENT_WINDOW);
  return set_boolean_value((bool) (x == 0 && CURRENT_VIEW->verify_col == 1), (short) 1);
}

short extract_focuseof_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  bool bool_flag;

  if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    bool_flag = CURRENT_BOF;
  } else {
    bool_flag = FOCUS_BOF;
  }
  return set_boolean_value((bool) (bool_flag), (short) 1);
}

short extract_focustof_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  bool bool_flag;

  if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
    bool_flag = CURRENT_TOF;
  } else {
    bool_flag = FOCUS_TOF;
  }
  return set_boolean_value((bool) (bool_flag), (short) 1);
}

short extract_filename(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) CURRENT_FILE->fname;
  item_values[1].len = strlen((char *) CURRENT_FILE->fname);
  return number_variables;
}

short extract_filestatus(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) "NONE";
  item_values[1].len = 4;
  if (ISREADONLY(CURRENT_FILE)) {
    item_values[2].value = (uchar *) "READONLY";
    item_values[2].len = 8;
  } else {
    item_values[2].value = (uchar *) "READWRITE";
    item_values[2].len = 9;
  }
  switch (CURRENT_FILE->eolfirst) {
    case EOLOUT_LF:
      item_values[3].value = (uchar *) "LF";
      item_values[3].len = 2;
      break;
    case EOLOUT_CR:
      item_values[3].value = (uchar *) "CR";
      item_values[3].len = 2;
      break;
    case EOLOUT_CRLF:
      item_values[3].value = (uchar *) "CRLF";
      item_values[3].len = 4;
      break;
    case EOLOUT_NONE:
      item_values[3].value = (uchar *) "NONE";
      item_values[3].len = 4;
      break;
  }
  return number_variables;
}

short extract_filetabs(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value((bool) FILETABSx, 1);
}

short extract_fname(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  short x = 0;

  strcpy((char *) query_rsrvd, (char *) CURRENT_FILE->fname);
  x = strzreveq(query_rsrvd, '.');
  if (x != (-1)) {
    query_rsrvd[x] = '\0';
  }
  item_values[1].value = query_rsrvd;
  item_values[1].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_efileid(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) CURRENT_FILE->efileid;
  item_values[1].len = strlen((char *) CURRENT_FILE->efileid);
  item_values[2].value = (uchar *) CURRENT_FILE->actualfname;
  item_values[2].len = strlen((char *) CURRENT_FILE->actualfname);
  return number_variables;
}

short extract_fpath(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) CURRENT_FILE->fpath;
  item_values[1].len = strlen((char *) CURRENT_FILE->fpath);
  return number_variables;
}

short extract_fdisplay(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  item_values[1].value = (uchar *) CURRENT_FILE->display_name;
  item_values[1].len = strlen((char *) CURRENT_FILE->display_name);
  return number_variables;
}

short extract_ftype(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  short x = 0;

  x = strzreveq(CURRENT_FILE->fname, '.');
  if (x == (-1)) {
    item_values[1].value = (uchar *) "";
    item_values[1].len = 0;
  } else {
    item_values[1].value = (uchar *) CURRENT_FILE->fname + x + 1;
    item_values[1].len = strlen((char *) CURRENT_FILE->fname + x + 1);
  }
  return number_variables;
}

short extract_fullfname(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CURRENT_FILE->display_actual_filename, 1);
}

short extract_getenv(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  uchar *tmpbuf = NULL;

  if (itemargs == NULL || strlen((char *) itemargs) == 0) {
    item_values[1].value = (uchar *) "***invalid***";
    item_values[1].len = strlen((char *) item_values[1].value);
    return number_variables;
  }
  if (query_type == QUERY_FUNCTION) {
    tmpbuf = (uchar *) getenv((char *) arg);
  } else {
    tmpbuf = (uchar *) getenv((char *) itemargs);
  }
  if (tmpbuf == NULL) {
    item_values[1].value = (uchar *) "***invalid***";
  } else {
    item_values[1].value = tmpbuf;
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  return number_variables;
}

short extract_header(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  short rc = RC_OK;
  int i = 0, off = 0;
  ushort save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;

  if (query_type == QUERY_QUERY) {
    for (i = 0; thm[i].the_header != HEADER_ALL; i++) {
      // no-op
    }
    CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, i);
    CURRENT_VIEW->msgmode_status = TRUE;
  }
  for (i = 0; thm[i].the_header != HEADER_ALL; i++) {
    sprintf((char *) query_rsrvd, "%s %s", thm[i].the_header_name, (CURRENT_VIEW->syntax_headers & thm[i].the_header) ? "ON" : "OFF");
    if (query_type == QUERY_QUERY) {
      display_error(0, query_rsrvd, TRUE);
    } else {
      number_variables++;
      item_values[number_variables].len = strlen((char *) query_rsrvd);
      memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
      item_values[number_variables].value = trec + off;
      off += (item_values[number_variables].len) + 1;
    }
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = save_msgline_rows;
    CURRENT_VIEW->msgmode_status = save_msgmode_status;
    rc = EXTRACT_VARIABLES_SET;
  } else {
    rc = number_variables;
  }
  return (rc);
}

short extract_hex(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CURRENT_VIEW->hex, 1);
}

short extract_hexdisplay(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(HEXDISPLAYx, 1);
}

short extract_hexshow(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  set_on_off_value(CURRENT_VIEW->hexshow_on, 1);
  if (CURRENT_VIEW->hexshow_base == POSITION_MIDDLE) {
    sprintf((char *) query_rsrvd, "M%+d", CURRENT_VIEW->hexshow_off);
  } else {
    sprintf((char *) query_rsrvd, "%d", CURRENT_VIEW->hexshow_off);
  }
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_highlight(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  switch (CURRENT_VIEW->highlight) {
    case HIGHLIGHT_NONE:
      item_values[1].value = (uchar *) "OFF";
      break;
    case HIGHLIGHT_ALT:
      item_values[1].value = (uchar *) "ALTERED";
      break;
    case HIGHLIGHT_TAG:
      item_values[1].value = (uchar *) "TAGGED";
      break;
    case HIGHLIGHT_SELECT:
      item_values[1].value = (uchar *) "SELECT";
      sprintf((char *) query_num1, "%d", CURRENT_VIEW->highlight_low);
      item_values[2].value = query_num1;
      item_values[2].len = strlen((char *) query_num1);
      sprintf((char *) query_num2, "%d", CURRENT_VIEW->highlight_high);
      item_values[3].value = query_num2;
      item_values[3].len = strlen((char *) query_num2);
      number_variables = 3;
      break;
    default:
      break;
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  return number_variables;
}

short extract_idline(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (query_type == QUERY_EXTRACT && !blank_field(itemargs)) {
    prepare_idline(current_screen);
    /*
     * Only time we supply 2 values; 'EXTRACT /IDLINE *'
     */
    if (CURRENT_VIEW->id_line) {
      item_values[1].value = (uchar *) "ON";
      item_values[1].len = 2;
    } else {
      item_values[1].value = (uchar *) "OFF";
      item_values[1].len = 3;
    }
    item_values[2].value = linebuf;
    item_values[2].len = strlen((char *) item_values[2].value);
    number_variables = 2;
  } else {
    return set_on_off_value(CURRENT_VIEW->id_line, 1);
  }
  return number_variables;
}

short extract_impmacro(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CURRENT_VIEW->imp_macro, 1);
}

short extract_impos(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(CURRENT_VIEW->imp_os, 1);
}

short extract_inblock_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;
  bool bool_flag = FALSE;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  bool_flag = FALSE;
  if (CURRENT_VIEW == MARK_VIEW && CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    x = getcurx(CURRENT_WINDOW_FILEAREA);
    switch (MARK_VIEW->mark_type) {
      case M_LINE:
        if ((CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line) && (CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)) {
          bool_flag = TRUE;
        }
        break;
      case M_BOX:
      case M_WORD:
      case M_COLUMN:
        if ((CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line) && (CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line) && (x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col) && (x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col)) {
          bool_flag = TRUE;
        }
        break;
      case M_STREAM:
        if ((CURRENT_VIEW->focus_line < MARK_VIEW->mark_start_line) || (CURRENT_VIEW->focus_line > MARK_VIEW->mark_end_line)) {
          bool_flag = FALSE;
          break;
        }
        if (MARK_VIEW->mark_start_line == MARK_VIEW->mark_end_line) {
          /*
           * Single-line block
           */
          if (x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col && x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col) {
            bool_flag = TRUE;
            break;
          } else {
            bool_flag = FALSE;
            break;
          }
        }
        /*
         * Multi-line stream block
         */
        if (CURRENT_VIEW->focus_line == MARK_VIEW->mark_start_line && x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col) {
          bool_flag = TRUE;
          break;
        }
        if (CURRENT_VIEW->focus_line == MARK_VIEW->mark_end_line && x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col) {
          bool_flag = TRUE;
          break;
        }
        if (CURRENT_VIEW->focus_line != MARK_VIEW->mark_start_line && CURRENT_VIEW->focus_line != MARK_VIEW->mark_end_line) {
          bool_flag = TRUE;
          break;
        }
        bool_flag = FALSE;
        break;
      default:
        break;
    }
  }
  return set_boolean_value((bool) bool_flag, (short) 1);
}

short extract_incommand_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_COMMAND), (short) 1);
}

short extract_initial_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) in_profile, (short) 1);
}

short extract_inprefix_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_PREFIX), (short) 1);
}

short extract_inputmode(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (CURRENT_VIEW->inputmode == INPUTMODE_OFF) {
    item_values[1].value = (uchar *) "OFF";
    item_values[1].len = 3;
  } else if (CURRENT_VIEW->inputmode == INPUTMODE_FULL) {
    item_values[1].value = (uchar *) "FULL";
    item_values[1].len = 4;
  } else if (CURRENT_VIEW->inputmode == INPUTMODE_LINE) {
    item_values[1].value = (uchar *) "LINE";
    item_values[1].len = 4;
  }
  return number_variables;
}

short extract_insertmode(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_on_off_value(INSERTMODEx, 1);
}

short extract_interface(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (INTERFACEx == INTERFACE_CLASSIC) {
    item_values[1].value = (uchar *) "CLASSIC";
    item_values[1].len = 7;
  } else if (INTERFACEx == INTERFACE_CUA) {
    item_values[1].value = (uchar *) "CUA";
    item_values[1].len = 3;
  }
  return number_variables;
}

short extract_insertmode_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  return set_boolean_value((bool) (INSERTMODEx), (short) 1);
}

short extract_lastkey(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  int keynum = 0;

  if (blank_field(itemargs)) {
    set_key_values((current_key == -1) ? -1 : lastkeys[current_key], lastkeys_is_mouse[current_key]);
  } else {
    if (valid_positive_integer(itemargs)) {
      keynum = atoi((char *) itemargs);
      if (keynum > 8) {
        return (EXTRACT_ARG_ERROR);
      }
      if (current_key == -1) {
        set_key_values(-1, FALSE);
      } else {
        keynum--;               /* 0 base the number */
        if (keynum <= current_key) {
          keynum = current_key - keynum;
        } else {
          keynum = 8 - (keynum - current_key);
        }
        set_key_values(lastkeys[keynum], lastkeys_is_mouse[keynum]);
      }
    } else {
      return (EXTRACT_ARG_ERROR);
    }
  }
  return number_variables;
}

short extract_lastmsg(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  if (last_message == NULL) {
    item_values[1].value = (uchar *) "";
    item_values[1].len = 0;
  } else {
    item_values[1].value = (uchar *) last_message;
    item_values[1].len = strlen((char *) last_message);
  }
  return number_variables;
}

short extract_lastop(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  short rc = RC_OK;
  int i = 0;
  bool found = FALSE;
  ushort save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
  uchar *ptr_lastop = NULL;
  int off = 0;

  if (itemargs == NULL || blank_field(itemargs) || strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, LASTOP_MAX);
      CURRENT_VIEW->msgmode_status = TRUE;
    } else {
      number_variables = 0;
    }
    for (i = 0; i < LASTOP_MAX; i++) {
      sprintf((char *) query_rsrvd, "%s%s %s", (query_type == QUERY_QUERY) ? (char *) "lastop " : "", lastop[i].command, (lastop[i].value) ? lastop[i].value : (uchar *) "");
      if (query_type == QUERY_QUERY) {
        display_error(0, query_rsrvd, TRUE);
      } else {
        number_variables++;
        item_values[number_variables].len = strlen((char *) query_rsrvd);
        memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
        item_values[number_variables].value = trec + off;
        off += (item_values[number_variables].len) + 1;
      }
    }
  } else {
    if (query_type == QUERY_QUERY) {
      CURRENT_VIEW->msgline_rows = 1;
      CURRENT_VIEW->msgmode_status = TRUE;
    }
    /*
     * Find a match for the supplied lastop operand
     */
    for (i = 0; i < LASTOP_MAX; i++) {
      if (equal(lastop[i].command, (uchar *) itemargs, lastop[i].min_len)) {
        ptr_lastop = (lastop[i].value) ? lastop[i].value : (uchar *) "";
        found = TRUE;
        break;
      }
    }
    if (query_type == QUERY_QUERY) {
      if (!found) {
        display_error(1, itemargs, TRUE);
        return EXTRACT_ARG_ERROR;
      }
      sprintf((char *) query_rsrvd, "%s%s %s", (query_type == QUERY_QUERY) ? (char *) "lastop " : "", lastop[i].command, ptr_lastop);
      display_error(0, query_rsrvd, TRUE);
    } else {
      if (!found) {
        return EXTRACT_ARG_ERROR;
      }
      item_values[1].value = lastop[i].command;
      item_values[1].len = strlen((char *) itemargs);
      item_values[2].value = ptr_lastop;
      item_values[2].len = strlen((char *) ptr_lastop);
      number_variables = 2;
    }
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = save_msgline_rows;
    CURRENT_VIEW->msgmode_status = save_msgmode_status;
    rc = EXTRACT_VARIABLES_SET;
  } else {
    rc = number_variables;
  }
  return rc;
}

short extract_lastrc(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  sprintf((char *) query_num1, "%d", lastrc);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_leftedge_function(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  unsigned short x = 0;

  if (batch_only) {
    item_values[1].value = (uchar *) "0";
    item_values[1].len = 1;
    return 1;
  }
  x = getcurx(CURRENT_WINDOW);
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_FILEAREA && x == 0), (short) 1);
}

short extract_length(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, CURRENT_VIEW->current_line, CURRENT_FILE->number_lines);
  sprintf((char *) query_num1, "%ld", curr->length);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_line(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  sprintf((char *) query_num1, "%ld", (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE));
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_lineflag(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE), CURRENT_FILE->number_lines);
  if (curr->flags.new_flag) {
    item_values[1].value = (uchar *) "NEW";
  } else {
    item_values[1].value = (uchar *) "NONEW";
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  if (curr->flags.changed_flag) {
    item_values[2].value = (uchar *) "CHANGE";
  } else {
    item_values[2].value = (uchar *) "NOCHANGE";
  }
  item_values[2].len = strlen((char *) item_values[2].value);
  if (curr->flags.tag_flag) {
    item_values[3].value = (uchar *) "TAG";
  } else {
    item_values[3].value = (uchar *) "NOTAG";
  }
  item_values[3].len = strlen((char *) item_values[3].value);
  return number_variables;
}

short extract_linend(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  set_on_off_value(CURRENT_VIEW->linend_status, 1);
  query_num1[0] = CURRENT_VIEW->linend_value;
  query_num1[1] = '\0';
  item_values[2].value = query_num1;
  item_values[2].len = 1;
  return number_variables;
}

short extract_lscreen(short number_variables, short itemno, uchar *itemargs, uchar query_type, long argc, uchar *arg, long arglen) {
  sprintf((char *) query_num1, "%d", CURRENT_SCREEN.screen_rows);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  sprintf((char *) query_num2, "%d", CURRENT_SCREEN.screen_cols);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  sprintf((char *) query_num3, "%d", CURRENT_SCREEN.screen_start_row + 1);
  item_values[3].value = query_num3;
  item_values[3].len = strlen((char *) query_num3);
  sprintf((char *) query_num4, "%d", CURRENT_SCREEN.screen_start_col + 1);
  item_values[4].value = query_num4;
  item_values[4].len = strlen((char *) query_num4);
  sprintf((char *) query_num5, "%d", terminal_lines);
  item_values[5].value = query_num5;
  item_values[5].len = strlen((char *) query_num5);
  sprintf((char *) query_num6, "%d", COLS);
  item_values[6].value = query_num6;
  item_values[6].len = strlen((char *) query_num6);
  return number_variables;
}
