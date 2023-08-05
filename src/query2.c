// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* Functions related to QUERY,STATUS and EXTRACT - M-Z */

#include "the.h"
#include "proto.h"

#include "query.h"

extern char_t  *block_name[];

extern char_t query_num1[20];
extern char_t query_num2[20];
extern char_t query_num3[40];
extern char_t query_num4[40];
extern char_t query_num5[10];
extern char_t query_num6[10];
extern char_t query_num7[10];
extern char_t query_num8[10];

extern char_t query_rsrvd[MAX_FILE_NAME + 100];

static LINE *curr;

short extract_macro(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->macro, 1);
}

short extract_macroext(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (strlen((char *) macro_suffix) == 0) {
    item_values[1].value = (char_t *) macro_suffix;
  } else {
    item_values[1].value = (char_t *) macro_suffix + 1;
  }
  item_values[1].len = strlen((char *) item_values[1].value);
  return number_variables;
}

short extract_macropath(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].value = (char_t *) the_macro_path;
  item_values[1].len = strlen((char *) the_macro_path);
  return number_variables;
}

short extract_margins(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%ld", CURRENT_VIEW->margin_left);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  sprintf((char *) query_num2, "%ld", CURRENT_VIEW->margin_right);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  if (CURRENT_VIEW->margin_indent_offset_status) {
    sprintf((char *) query_num3, "%+ld", CURRENT_VIEW->margin_indent);
  } else {
    sprintf((char *) query_num3, "%ld", CURRENT_VIEW->margin_indent);
  }
  item_values[3].value = query_num3;
  item_values[3].len = strlen((char *) query_num3);
  return number_variables;
}

short extract_modifiable_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  bool bool_flag = FALSE;
  short y = 0;

  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      if (batch_only) {
        bool_flag = FALSE;
        break;
      }
      y = getcury(CURRENT_WINDOW);
      if (FOCUS_TOF || FOCUS_BOF || CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW) {
        bool_flag = FALSE;
      } else {
        bool_flag = TRUE;
      }
      break;

    default:
      bool_flag = TRUE;
      break;
  }
  return set_boolean_value((bool) bool_flag, (short) 1);
}

short extract_monitor(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (colour_support) {
    item_values[1].value = (char_t *) "COLOR";
    item_values[1].len = 5;
  } else {
    item_values[1].value = (char_t *) "MONO";
    item_values[1].len = 4;
  }
  item_values[2].value = (char_t *) "COLOR";
  item_values[2].len = 5;
  return number_variables;
}

short extract_mouse(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(MOUSEx, 1);
}

short extract_mouseclick(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int mouseclick = -1;

  mouseclick = mouseinterval(-1);
  sprintf((char *) query_num3, "%d", mouseclick);
  item_values[1].value = query_num3;
  item_values[1].len = strlen((char *) query_num3);
  return number_variables;
}

short extract_msgline(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].value = (char_t *) "ON";
  item_values[1].len = 2;
  if (CURRENT_VIEW->msgline_base == POSITION_MIDDLE) {
    sprintf((char *) query_rsrvd, "M%+d", CURRENT_VIEW->msgline_off);
  } else {
    sprintf((char *) query_rsrvd, "%d", CURRENT_VIEW->msgline_off);
  }
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) query_rsrvd);
  if (CURRENT_VIEW->msgline_rows) {
    sprintf((char *) query_num1, "%d", CURRENT_VIEW->msgline_rows);
    item_values[3].value = query_num1;
  } else {
    item_values[3].value = (char_t *) "*";
  }
  item_values[3].len = strlen((char *) query_num1);
  item_values[4].value = (char_t *) "OVERLAY";
  item_values[4].len = 7;
  return number_variables;
}

short extract_msgmode(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  set_on_off_value(CURRENT_VIEW->msgmode_status, 1);
  item_values[2].value = (char_t *) "LONG";
  item_values[2].len = 4;
  return number_variables;
}

short extract_nbfile(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%ld", number_of_files);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_nbscope(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  line_t number_lines = 0;

  if (CURRENT_VIEW->scope_all) {
    sprintf((char *) query_num1, "%ld", CURRENT_FILE->number_lines);
  } else {
    for (number_lines = 0L, curr = CURRENT_FILE->first_line; curr != NULL; curr = curr->next) {
      if (curr->prev == NULL || curr->next == NULL) {
        continue;
      }
      if (IN_SCOPE(CURRENT_VIEW, curr)) {
        number_lines++;
      }
    }
    sprintf((char *) query_num1, "%ld", number_lines);
  }
  sprintf((char *) query_num2, "%ld", get_true_line(TRUE));
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  return number_variables;
}

short extract_newlines(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (CURRENT_VIEW->newline_aligned) {
    item_values[1].value = (char_t *) "ALIGNED";
    item_values[1].len = 7;
  } else {
    item_values[1].value = (char_t *) "LEFT";
    item_values[1].len = 4;
  }
  return number_variables;
}

short extract_nondisp(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  query_num1[0] = NONDISPx;
  query_num1[1] = '\0';
  item_values[1].value = query_num1;
  item_values[1].len = 1;
  return number_variables;
}

short extract_number(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->number, 1);
}

short extract_pagewrap(short pagewrap_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(PAGEWRAPx, 1);
}

short extract_parser(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  short rc = RC_OK;
  register int i = 0;
  int off = 0;
  bool found = FALSE;
  row_t save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
  char_t *ptr_filename = NULL;
  PARSER_DETAILS *curr;

  if (itemargs == NULL || blank_field(itemargs) || strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      for (i = 0, curr = first_parser; curr != NULL; curr = curr->next, i++);
      CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, i);
      CURRENT_VIEW->msgmode_status = TRUE;
    } else {
      number_variables = 0;
    }
    for (curr = first_parser; curr != NULL; curr = curr->next) {
      sprintf((char *) query_rsrvd, "%s%s %s", (query_type == QUERY_QUERY) ? (char *) "parser " : "", curr->parser_name, curr->filename);
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
     * Find a match for the supplied mask or magic number
     */
    for (curr = first_parser; curr != NULL; curr = curr->next) {
      if (strcasecmp((char *) itemargs, (char *) curr->parser_name) == 0) {
        ptr_filename = curr->filename;
        found = TRUE;
        break;
      }
    }
    if (!found) {
      ptr_filename = (char_t *) "";
    }
    if (query_type == QUERY_QUERY) {
      sprintf((char *) query_rsrvd, "%s%s %s", (query_type == QUERY_QUERY) ? (char *) "parser " : "", itemargs, ptr_filename);
      display_error(0, query_rsrvd, TRUE);
    } else {
      item_values[1].value = itemargs;
      item_values[1].len = strlen((char *) itemargs);
      item_values[2].value = ptr_filename;
      item_values[2].len = strlen((char *) ptr_filename);
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

/*
                                 +-- * ---+
<---+---------+--+-----------+---+- name -+---+---------------------------------+--->
    +- BLOCK -+  +- OLDNAME -+                |   +--- :1 ---+   +--- * ----+   |
                                              +---+- target -+---+- target -+---+
*/

#define PEN_PARAMS  6

#define STATE_START    0
#define STATE_OLDNAME  1
#define STATE_NAME     2
#define STATE_TARGET1  3
#define STATE_TARGET2  4

short extract_pending(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  char_t *word[PEN_PARAMS + 1];
  char_t strip[PEN_PARAMS];
  unsigned short num_params = 0;
  bool find_block = FALSE;
  bool find_oldname = FALSE;
  char_t *name = NULL;
  THE_PPC *curr_ppc = NULL;
  LINE *curr;
  line_t first_in_range = 0L;
  line_t last_in_range = CURRENT_FILE->number_lines + 1;
  THE_PPC *found_ppc = NULL;
  short target_type = TARGET_ABSOLUTE | TARGET_RELATIVE;
  TARGET target;
  short rc;
  short state = STATE_START;
  int i, j;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(itemargs, word, PEN_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    /*
     * No parameters, error.
     */
    display_error(3, (char_t *) "", FALSE);
    return EXTRACT_ARG_ERROR;
  }
  i = 0;
  for (;;) {
    if (i == num_params) {
      break;
    }
    switch (state) {

      case STATE_START:
        if (equal((char_t *) "block", word[i], 5)) {
          find_block = TRUE;
          state = STATE_OLDNAME;
          i++;
          break;
        }
        if (equal((char_t *) "oldname", word[i], 7)) {
          find_oldname = TRUE;
          state = STATE_NAME;
          i++;
          break;
        }
        state = STATE_NAME;
        break;

      case STATE_OLDNAME:
        if (equal((char_t *) "oldname", word[i], 7)) {
          find_oldname = TRUE;
          state = STATE_NAME;
          i++;
          break;
        }
        state = STATE_NAME;
        break;

      case STATE_NAME:
        name = word[i];
        state = STATE_TARGET1;
        i++;
        break;

      case STATE_TARGET1:
        initialise_target(&target);
        rc = validate_target(word[i], &target, target_type, 0L, FALSE, FALSE);
        if (rc == RC_OK) {
          first_in_range = target.rt[0].numeric_target;
          i++;
          state = STATE_TARGET2;
        } else {
          /* error */
          number_variables = EXTRACT_ARG_ERROR;
        }
        free_target(&target);
        break;

      case STATE_TARGET2:
        if (equal((char_t *) "*", word[i], 1)) {
          /*
           * If the second target is *, then default last_in_range to be
           * 1 more than the number of lines in the file; same as if no
           * second target was specified.
           */
          last_in_range = CURRENT_FILE->number_lines + 1;
          i++;
          state = STATE_TARGET2;
        } else {
          initialise_target(&target);
          rc = validate_target(word[i], &target, target_type, first_in_range, FALSE, FALSE);
          if (rc == RC_OK) {
            last_in_range = target.rt[0].numeric_target;
            i++;
            state = STATE_TARGET2;
          } else {
            /* error */
            number_variables = EXTRACT_ARG_ERROR;
          }
          free_target(&target);
        }
        break;
    }
    if (number_variables == EXTRACT_ARG_ERROR) {
      break;
    }
  }
  /*
   * If the validation of parameters is successful...
   */
  if (number_variables >= 0) {
    /*
     * No pending prefix commands, return 0.
     */
    if (CURRENT_FILE->first_ppc == NULL) {
      number_variables = 0;
    } else {
      /*
       * If we are to look for OLDNAME, find a synonym for it if one exists..
       */
      if (find_oldname) {
        name = find_prefix_oldname(name);
      }
      /*
       * For each pending prefix command...
       */
      curr_ppc = CURRENT_FILE->first_ppc;
      for (;;) {
        if (curr_ppc == NULL) {
          break;
        }
        /*
         * Ignore the pending prefix if we have already processed it
         */
        if (curr_ppc->ppc_processed || curr_ppc->ppc_current_command) {
          curr_ppc = curr_ppc->next;
          continue;
        }
        /*
         * To imitate XEDIT behaviour, ignore a prefix command if the line on which the prefix command
         * has been entered is not in scope.
         */
        curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, curr_ppc->ppc_line_number, CURRENT_FILE->number_lines);
        if (!(IN_SCOPE(CURRENT_VIEW, curr) || CURRENT_VIEW->scope_all || curr_ppc->ppc_shadow_line)) {
          curr_ppc = curr_ppc->next;
          continue;
        }
        /*
         * If we want to match on any name...
         */
        if (strcmp((char *) name, "*") == 0) {
          /*
           * Are we matching on any BLOCK command...
           */
          if (find_block) {
            if (curr_ppc->ppc_block_command) {
              /*
               * We have found the first BLOCK command with any name.
               */
              if (found_ppc == NULL) {
                found_ppc = curr_ppc;
              } else {
                if (curr_ppc->ppc_line_number < found_ppc->ppc_line_number) {
                  found_ppc = curr_ppc;
                }
              }
            }
            /*
             * Go back and look for another either because we didn't
             * find a block command, or because we did, but it may not
             * be the one with the smallest line number.
             */
            curr_ppc = curr_ppc->next;
            continue;
          } else {
            /*
             * We have found the first command with any name.
             */
            found_ppc = in_range(found_ppc, curr_ppc, first_in_range, last_in_range);
            /*
             * Go back and look for another because it may not
             * be the one with the smallest line number.
             */
            curr_ppc = curr_ppc->next;
            continue;
          }
        }
        /*
         * We want to find a specific command...
         */
        if (strcasecmp((char *) curr_ppc->ppc_command, (char *) name) == 0) {
          /*
           * Are we looking for a specific BLOCK command...
           */
          if (find_block) {
            if (curr_ppc->ppc_block_command) {
              /*
               * We have found the first specific BLOCK command.
               */
              found_ppc = in_range(found_ppc, curr_ppc, first_in_range, last_in_range);
            }
          } else {
            /*
             * We have found the first specific command.
             */
            found_ppc = in_range(found_ppc, curr_ppc, first_in_range, last_in_range);
          }
          /*
           * Go back and look for another because it may not
           * be the one with the smallest line number.
           */
          curr_ppc = curr_ppc->next;
          continue;
        }
        curr_ppc = curr_ppc->next;
      }
      /*
       * Did we find a matching pending prefix command ?
       */
      if (found_ppc == NULL) {
        number_variables = 0;
      } else {
        /*
         * Yes we did. Set all of the REXX variables to the correct values...
         */
        sprintf((char *) query_num1, "%ld", found_ppc->ppc_line_number);
        item_values[1].value = query_num1;
        item_values[1].len = strlen((char *) query_num1);
        item_values[2].value = found_ppc->ppc_command;
        item_values[2].len = strlen((char *) item_values[2].value);
        item_values[3].value = find_prefix_synonym(found_ppc->ppc_command);
        item_values[3].len = strlen((char *) item_values[3].value);
        if (found_ppc->ppc_block_command) {
          item_values[4].value = (char_t *) "BLOCK";
        } else {
          item_values[4].value = (char_t *) "";
        }
        item_values[4].len = strlen((char *) item_values[4].value);
        for (i = 0; i < PPC_OPERANDS; i++) {
          j = i + 5;
          item_values[j].value = found_ppc->ppc_op[i];
          item_values[j].len = strlen((char *) item_values[j].value);
        }
        number_variables = PPC_OPERANDS + 4;
      }
    }
  }
  return number_variables;
}

short extract_point(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  line_t true_line = 0;
  int len, total_len = 0;
  THELIST *curr_name;

  switch (query_type) {

    case QUERY_QUERY:
    case QUERY_MODIFY:
      true_line = (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE);
      curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
      if (curr->first_name == NULL) {   /* line not named */
        item_values[1].value = (char_t *) "";
        item_values[1].len = 0;
      } else {
        strcpy((char *) query_rsrvd, "");
        curr_name = curr->first_name;
        while (curr_name) {
          len = strlen((char *) curr_name->data);
          if (total_len + len + 1 > sizeof(query_rsrvd)) {
            break;
          }
          total_len += len + 1;
          strcat((char *) query_rsrvd, " ");
          strcat((char *) query_rsrvd, (char *) curr_name->data);
          curr_name = curr_name->next;
        }
        item_values[1].value = query_rsrvd;
        item_values[1].len = total_len;
      }
      break;

    default:
      number_variables = extract_point_settings(itemno, itemargs);
      break;
  }
  return number_variables;
}

short extract_position(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  line_t true_line = 0;
  length_t col = 0;

  set_on_off_value(CURRENT_VIEW->position_status, 1);
  if ((query_type == QUERY_EXTRACT || query_type == QUERY_FUNCTION) && !batch_only) {
    get_current_position(current_screen, &true_line, &col);
    sprintf((char *) query_num1, "%ld", true_line);
    item_values[2].value = query_num1;
    item_values[2].len = strlen((char *) query_num1);
    sprintf((char *) query_num2, "%ld", col);
    item_values[3].value = query_num2;
    item_values[3].len = strlen((char *) query_num2);
  } else {
    number_variables = 1;
  }
  return number_variables;
}

short extract_prefix(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  /*
   * Simply handle QUERY PREFIX here...
   */
  if (strcmp((char *) itemargs, "") == 0) {
    if (CURRENT_VIEW->prefix == PREFIX_OFF) {
      item_values[1].value = (char_t *) "OFF";
      item_values[1].len = 3;
      item_values[2].value = (char_t *) "";   /* this set to empty deliberately */
      item_values[2].len = 0;
      return 1;
    }
    if ((CURRENT_VIEW->prefix & PREFIX_STATUS_MASK) == PREFIX_ON) {
      item_values[1].value = (char_t *) "ON";
      item_values[1].len = 2;
    } else {
      item_values[1].value = (char_t *) "NULLS";
      item_values[1].len = 5;
    }
    if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
      item_values[2].value = (char_t *) "LEFT";
      item_values[2].len = 4;
    } else {
      item_values[2].value = (char_t *) "RIGHT";
      item_values[2].len = 5;
    }
    sprintf((char *) query_num1, "%d", CURRENT_VIEW->prefix_width);
    item_values[3].value = query_num1;
    item_values[3].len = strlen((char *) query_num1);
    sprintf((char *) query_num2, "%d", CURRENT_VIEW->prefix_gap);
    item_values[4].value = query_num2;
    item_values[4].len = strlen((char *) query_num2);
    number_variables = 4;
  } else {
    /*
     * ...but for QUERY PREFIX SYNONYM, its more complicated...
     */
    number_variables = extract_prefix_settings(itemno, itemargs, query_type);
  }
  return number_variables;
}

short extract_printer(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].value = (char_t *) spooler_name;
  item_values[1].len = strlen((char *) spooler_name);
  return number_variables;
}

short extract_profile(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (local_prf == NULL) {
    item_values[1].value = (char_t *) "";
    item_values[1].len = 0;
  } else {
    item_values[1].value = (char_t *) local_prf;
    item_values[1].len = strlen((char *) local_prf);
  }
  return number_variables;
}

short extract_pscreen(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].len = sprintf((char *) query_num1, "%d", LINES);
  item_values[1].value = query_num1;
  item_values[2].len = sprintf((char *) query_num2, "%d", COLS);
  item_values[2].value = query_num2;
  return number_variables;
}

short extract_reprofile(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(REPROFILEx, 1);
}

short extract_readonly(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (READONLYx == READONLY_FORCE) {
    item_values[1].value = (char_t *) "FORCE";
    item_values[1].len = 5;
  } else if (ISREADONLY(CURRENT_FILE)) {
    item_values[1].value = (char_t *) "ON";
    item_values[1].len = 2;
  } else {
    item_values[1].value = (char_t *) "OFF";
    item_values[1].len = 3;
  }
  return number_variables;
}

short extract_readv(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int key = 0;
  bool mouse_key = FALSE;

  if (batch_only) {
    item_values[1].value = (char_t *) "0";
    item_values[1].len = 1;
    return 1;
  }
  for (;;) {
    if (is_termresized()) {
      (void) THE_Resize(0, 0);
      (void) THERefresh((char_t *) "");
    }
    key = wgetch(CURRENT_WINDOW);
    if (is_termresized()) {
      continue;
    }
    if (key == KEY_MOUSE) {
      mouse_key = TRUE;
    } else {
      mouse_key = FALSE;
    }
    break;
  }
  if (current_key == -1) {
    current_key = 0;
  } else {
    if (current_key == 7) {
      current_key = 0;
    } else {
      current_key++;
    }
  }
  lastkeys[current_key] = key;
  set_key_values(key, mouse_key);
  return number_variables;
}

short extract_reserved(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  bool line_numbers_only = TRUE;
  RESERVED *curr_rsrvd;
  char_t *tmpbuf = NULL, *attr_string = NULL;
  short rc = RC_OK;
  short y = 0, x = 0;

  if (query_type == QUERY_EXTRACT && strcmp((char *) itemargs, "*") == 0) {
    line_numbers_only = FALSE;
  }
  number_variables = 0;
  curr_rsrvd = CURRENT_FILE->first_reserved;
  strcpy((char *) query_rsrvd, "");
  while (curr_rsrvd != NULL) {
    if (line_numbers_only) {
      y = strlen((char *) curr_rsrvd->spec) + 1;
      if ((x + y) > sizeof(query_rsrvd)) {
        break;
      }
      strcat((char *) query_rsrvd, (char *) curr_rsrvd->spec);
      strcat((char *) query_rsrvd, " ");
      x += y;
    } else {
      attr_string = get_colour_strings(curr_rsrvd->attr);
      if (attr_string == (char_t *) NULL) {
        return (EXTRACT_ARG_ERROR);
      }
      tmpbuf = (char_t *) malloc (sizeof(char_t) * (strlen((char *) attr_string) + strlen((char *) curr_rsrvd->line) + strlen((char *) curr_rsrvd->spec) + 13));
      if (tmpbuf == (char_t *) NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (EXTRACT_ARG_ERROR);
      }
      if (curr_rsrvd->autoscroll) {
        strcpy((char *) tmpbuf, "AUTOSCROLL ");
        strcat((char *) tmpbuf, (char *) curr_rsrvd->spec);
      } else {
        strcpy((char *) tmpbuf, (char *) curr_rsrvd->spec);
      }
      strcat((char *) tmpbuf, " ");
      strcat((char *) tmpbuf, (char *) attr_string);
      free (attr_string);
      strcat((char *) tmpbuf, (char *) curr_rsrvd->line);
      rc = set_rexx_variable(query_item[itemno].name, tmpbuf, strlen((char *) tmpbuf), ++number_variables);
      free (tmpbuf);
      if (rc == RC_SYSTEM_ERROR) {
        display_error(54, (char_t *) "", FALSE);
        return (EXTRACT_ARG_ERROR);
      }
    }
    curr_rsrvd = curr_rsrvd->next;
  }
  if (line_numbers_only) {
    if (x == 0) {
      number_variables = 0;
    } else {
      number_variables = 1;
      item_values[1].value = query_rsrvd;
      item_values[1].len = strlen((char *) query_rsrvd);
    }
  } else {
    sprintf((char *) query_rsrvd, "%d", number_variables);
    rc = set_rexx_variable(query_item[itemno].name, query_rsrvd, strlen((char *) query_rsrvd), 0);
    if (rc == RC_SYSTEM_ERROR) {
      display_error(54, (char_t *) "", FALSE);
      number_variables = EXTRACT_ARG_ERROR;
    } else {
      number_variables = EXTRACT_VARIABLES_SET;
    }
  }
  return number_variables;
}

short extract_rexxhalt(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (COMMANDCALLSx == 0) {
    item_values[1].value = (char_t *) "OFF";
    item_values[1].len = 3;
  } else {
    item_values[1].len = sprintf((char *) query_num1, "%d", COMMANDCALLSx);
    item_values[1].value = query_num1;
  }
  if (FUNCTIONCALLSx == 0) {
    item_values[2].value = (char_t *) "OFF";
    item_values[2].len = 3;
  } else {
    item_values[2].len = sprintf((char *) query_num1, "%d", FUNCTIONCALLSx);
    item_values[2].value = query_num1;
  }
  return number_variables;
}

short extract_rexxoutput(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (CAPREXXOUTx) {
    item_values[1].value = (char_t *) "FILE";
    item_values[1].len = 4;
  } else {
    item_values[1].value = (char_t *) "DISPLAY";
    item_values[1].len = 7;
  }
  sprintf((char *) query_num1, "%ld", CAPREXXMAXx);
  item_values[2].value = query_num1;
  item_values[2].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_rightedge_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  short x = 0;

  if (batch_only) {
    item_values[1].value = (char_t *) "0";
    item_values[1].len = 1;
    return number_variables;
  }
  x = getcurx(CURRENT_WINDOW);
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_FILEAREA && x == getmaxx(CURRENT_WINDOW) - 1), (short) 1);
}

short extract_ring(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  VIEW_DETAILS *curr;
  FILE_DETAILS *first_view_file = NULL;
  bool process_view = FALSE;
  line_t lineno = 0L;
  length_t col = 0;
  register int i = 0, j = 0;
  int offset = 0, off = 0;
  bool view_being_displayed = FALSE;
  row_t save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;

  if (compatible_feel == COMPAT_XEDIT) {
    offset = 1;
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, number_of_files);
    CURRENT_VIEW->msgmode_status = TRUE;
  } else {
    number_variables = offset;
  }
  curr = vd_current;
  for (j = 0; j < number_of_files;) {
    process_view = TRUE;
    if (curr->file_for_view->file_views > 1) {
      if (first_view_file == curr->file_for_view) {
        process_view = FALSE;
      } else {
        first_view_file = curr->file_for_view;
      }
    }
    if (process_view) {
      j++;
      view_being_displayed = FALSE;
      for (i = 0; i < display_screens; i++) {
        if (SCREEN_VIEW(i) == curr) {
          view_being_displayed = TRUE;
          get_current_position((char_t) i, &lineno, &col);
        }
      }
      if (!view_being_displayed) {
        lineno = (curr->current_window == WINDOW_COMMAND) ? curr->current_line : curr->focus_line;
        col = curr->current_column;
      }
      if (compatible_look == COMPAT_XEDIT) {
        sprintf((char *) query_rsrvd, "%s%s Size=%ld Line=%ld Col=%ld Alt=%d,%d", curr->file_for_view->fpath, curr->file_for_view->fname, curr->file_for_view->number_lines, lineno, col, curr->file_for_view->autosave_alt, curr->file_for_view->save_alt);
      } else {
        sprintf((char *) query_rsrvd, "%s%s Line=%ld Col=%ld Size=%ld Alt=%d,%d", curr->file_for_view->fpath, curr->file_for_view->fname, lineno, col, curr->file_for_view->number_lines, curr->file_for_view->autosave_alt, curr->file_for_view->save_alt);
      }
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
    curr = curr->next;
    if (curr == NULL) {
      curr = vd_first;
    }
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = save_msgline_rows;
    CURRENT_VIEW->msgmode_status = save_msgmode_status;
    number_variables = EXTRACT_VARIABLES_SET;
  } else {
    if (offset) {
      sprintf((char *) query_num1, "%d", number_variables - 1);
      item_values[1].value = query_num1;
      item_values[1].len = strlen((char *) query_num1);
    }
  }
  return number_variables;
}

short extract_scale(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  set_on_off_value(CURRENT_VIEW->scale_on, 1);
  if (CURRENT_VIEW->scale_base == POSITION_MIDDLE) {
    sprintf((char *) query_rsrvd, "M%+d", CURRENT_VIEW->scale_off);
  } else {
    sprintf((char *) query_rsrvd, "%d", CURRENT_VIEW->scale_off);
  }
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_scope(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (CURRENT_VIEW->scope_all) {
    item_values[1].value = (char_t *) "ALL";
    item_values[1].len = 3;
  } else {
    item_values[1].value = (char_t *) "DISPLAY";
    item_values[1].len = 7;
  }
  return number_variables;
}

short extract_screen(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%d", display_screens);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  if (horizontal) {
    item_values[2].value = (char_t *) "HORIZONTAL";
    item_values[2].len = 10;
  } else {
    item_values[2].value = (char_t *) "VERTICAL";
    item_values[2].len = 8;
  }
  return number_variables;
}

short extract_select(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  LINE *curr = NULL;
  unsigned short x = 0;

  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE), CURRENT_FILE->number_lines);
  sprintf((char *) query_num1, "%d", curr->select);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  x = 0;
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, 1L, CURRENT_FILE->number_lines);
  while (curr->next != NULL) {
    if (curr->select > x) {
      x = curr->select;
    }
    curr = curr->next;
  }
  item_values[2].len = sprintf((char *) query_num2, "%d", x);
  item_values[2].value = query_num2;
  return number_variables;
}

short extract_shadow(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->shadow, 1);
}

short extract_shadow_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  bool bool_flag = FALSE;
  short y = 0;

  switch (CURRENT_VIEW->current_window) {

    case WINDOW_FILEAREA:
      if (batch_only) {
        bool_flag = FALSE;
        break;
      }
      y = getcury(CURRENT_WINDOW);
      if (CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW) {
        bool_flag = TRUE;
      } else {
        bool_flag = FALSE;
      }
      break;

    default:
      bool_flag = FALSE;
      break;
  }
  return set_boolean_value((bool) bool_flag, (short) 1);
}

short extract_shift_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int shift = 0;

  get_key_name(lastkeys[current_key], &shift);
  return set_boolean_value((bool) (shift & SHIFT_SHIFT), (short) 1);
}

short extract_showkey(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int key = 0;

  if (strcmp((char *) itemargs, "") == 0 || ((key = find_key_name(itemargs)) == -1)) {
    item_values[1].value = (char_t *) "INVALID KEY";
    item_values[1].len = strlen((char *) item_values[1].value);
  } else {
    function_key(key, OPTION_EXTRACT, FALSE);
    number_variables = EXTRACT_VARIABLES_SET;
  }
  return number_variables;
}

short extract_size(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%ld", CURRENT_FILE->number_lines);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_slk(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int off = 0;
  int item;

  if (blank_field(itemargs)) {
    if (max_slk_labels == 0 && query_type != QUERY_MODIFY) {
      item_values[1].value = (char_t *) "UNAVAILABLE";
      item_values[1].len = 11;
      number_variables = 2;
    } else if (SLKx) {
      item_values[1].value = (char_t *) "ON";
      item_values[1].len = 2;
      number_variables = 2;
    } else {
      item_values[1].value = (char_t *) "OFF";
      item_values[1].len = 3;
      number_variables = 2;
    }
    if (query_type == QUERY_MODIFY || query_type == QUERY_STATUS) {
      number_variables = 1;
    } else {
      item_values[2].len = sprintf((char *) query_num1, "%d", slk_format_switch);
      item_values[2].value = query_num1;
    }
    return (number_variables);
  } else if (max_slk_labels == 0) {
    display_error(82, (char_t *) "- use -k command line switch to enable", FALSE);
    number_variables = EXTRACT_ARG_ERROR;
  } else if (strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      display_error(1, (char_t *) itemargs, FALSE);
      return EXTRACT_ARG_ERROR;
    }
    for (number_variables = 0; number_variables < max_slk_labels;) {
      number_variables++;
      strcpy((char *) query_rsrvd, slk_label(number_variables));
      item_values[number_variables].len = strlen((char *) query_rsrvd);
      memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
      item_values[number_variables].value = trec + off;
      off += (item_values[number_variables].len) + 1;
    }
  } else {
    if (valid_positive_integer(itemargs)) {
      /*
       * Try and find a matching slk
       */
      item = atoi((char *) itemargs);
      if (item < 1) {
        display_error(5, (char_t *) itemargs, FALSE);
        return EXTRACT_ARG_ERROR;
      }
      if (item > max_slk_labels) {
        display_error(6, (char_t *) itemargs, FALSE);
        return EXTRACT_ARG_ERROR;
      }
      if (query_type == QUERY_QUERY) {
        strcpy((char *) query_rsrvd, slk_label(item));
        sprintf((char *) trec, "slk %s", query_rsrvd);
        display_error(0, trec, TRUE);
        number_variables = EXTRACT_VARIABLES_SET;
      } else {
        /*
         * EXTRACT
         */
        strcpy((char *) query_rsrvd, slk_label(item));
        item_values[1].value = query_rsrvd;
        item_values[1].len = strlen((char *) query_rsrvd);
        number_variables = 1;
      }
    } else {
      display_error(4, (char_t *) itemargs, FALSE);
      number_variables = EXTRACT_ARG_ERROR;
    }
  }
  return (number_variables);
}

short extract_spacechar_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  char_t cursor_char = 0;

  if (batch_only) {
    item_values[1].value = (char_t *) "0";
    item_values[1].len = 1;
    return 1;
  }
  cursor_char = (char_t) (winch(CURRENT_WINDOW) & A_CHARTEXT);
  return set_boolean_value((bool) (cursor_char == ' '), (short) 1);
}

short extract_statopt(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  short rc = RC_OK;
  register int i = 0;
  int off = 0;
  row_t save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
  LINE *curr;

  if (itemargs == NULL || blank_field(itemargs) || strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      for (i = 0, curr = first_option; curr != NULL; curr = curr->next, i++);
      CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, i);
      CURRENT_VIEW->msgmode_status = TRUE;
    } else {
      number_variables = 0;
    }
    for (curr = first_option; curr != NULL; curr = curr->next) {
      sprintf((char *) query_rsrvd, "%sON %s %d %d %s", (query_type == QUERY_QUERY) ? (char *) "statopt " : "", curr->name, curr->select + 1 + STATAREA_OFFSET, curr->save_select, (char *) ((curr->line != NULL) ? (char *) curr->line : ""));
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
     * Find a match for the supplied option
     */
    curr = lll_locate(first_option, make_upper(itemargs));
    if (curr) {
      /*
       * We found it
       */
      sprintf((char *) query_rsrvd, "%sON %s %d %d %s", (query_type == QUERY_QUERY) ? (char *) "statopt " : "", curr->name, curr->select, curr->save_select, (char *) ((curr->line != NULL) ? (char *) curr->line : ""));
    } else {
      /*
       * We didn't find it
       */
      sprintf((char *) query_rsrvd, "%sOFF %s", (query_type == QUERY_QUERY) ? (char *) "statopt " : "", itemargs);
    }
    if (query_type == QUERY_QUERY) {
      display_error(0, query_rsrvd, TRUE);
    } else {
      item_values[1].value = query_rsrvd;
      item_values[1].len = strlen((char *) query_rsrvd);
      number_variables = 1;
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

short extract_statusline(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  switch (STATUSLINEx) {

    case 'B':
      item_values[1].value = (char_t *) "BOTTOM";
      item_values[1].len = 6;
      break;

    case 'T':
      item_values[1].value = (char_t *) "TOP";
      item_values[1].len = 3;
      break;

    case 'O':
      item_values[1].value = (char_t *) "OFF";
      item_values[1].len = 3;
      break;

    case 'G':
      item_values[1].value = (char_t *) "GUI";
      item_values[1].len = 3;
      break;
  }
  return number_variables;
}

short extract_stay(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->stay, 1);
}

#define SYN_PARAMS  4

short extract_synelem(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  unsigned short y = 0, x = 0;
  char_t syntax_element;
  char_t *word[PEN_PARAMS + 1];
  char_t strip[PEN_PARAMS];
  unsigned short num_params = 0;
  length_t row, col;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  num_params = param_split(itemargs, word, SYN_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0 || equal((char_t *) "cursor", word[0], 1)) {
    // No parameters, cursor
    /*
     * Are we in filearea?
     */
    if (CURRENT_VIEW->current_window != WINDOW_FILEAREA || !curses_started) {
      item_values[1].value = (char_t *) "UNKNOWN";
      item_values[1].len = 4;
      return (number_variables);
    }
    /*
     * Determine position of cursor relative to ESCREEN
     * This should result in a direct entry into the highlight_type array
     */
    getyx(CURRENT_WINDOW, y, x);
    syntax_element = get_syntax_element(current_screen, y, x);
  } else if (equal((char_t *) "file", word[0], 1)) {
    if (num_params > 3) {
      display_error(2, (char_t *) "", FALSE);
      return EXTRACT_ARG_ERROR;
    } else if (num_params < 3) {
      display_error(3, (char_t *) "", FALSE);
      return EXTRACT_ARG_ERROR;
    } else {
      /*
       * 3 args; file and two numbers
       */
      if (!valid_positive_integer(word[1])) {
        display_error(1, word[1], FALSE);
        return EXTRACT_ARG_ERROR;
      }
      if (!valid_positive_integer(word[2])) {
        display_error(1, word[2], FALSE);
        return EXTRACT_ARG_ERROR;
      }
      /*
       * Now have two numbers, do they specify a file position
       * that is currently viewable
       */
      row = atol((char *) word[1]);
      col = atol((char *) word[2]);
      if (!line_in_view(current_screen, row)) {
        item_values[1].value = (char_t *) "UNKNOWN";
        item_values[1].len = 4;
        return (number_variables);
      }
      /*
       * If column is not in display, error.
       */
      if (!column_in_view(current_screen, col - 1)) {
        item_values[1].value = (char_t *) "UNKNOWN";
        item_values[1].len = 4;
        return (number_variables);
      }
      x = (line_t) col - (line_t) CURRENT_VIEW->verify_col;
      y = get_row_for_focus_line(current_screen, row, CURRENT_VIEW->current_row);
      syntax_element = get_syntax_element(current_screen, y, x);
    }
  } else {
    /*
     * Assume it is two numbers
     */
    if (num_params > 2) {
      display_error(2, (char_t *) "", FALSE);
      return EXTRACT_ARG_ERROR;
    } else if (num_params < 2) {
      display_error(3, (char_t *) "", FALSE);
      return EXTRACT_ARG_ERROR;
    } else {
      /*
       * 2 args; and two numbers
       */
      if (!valid_positive_integer(word[0])) {
        display_error(1, word[0], FALSE);
        return EXTRACT_ARG_ERROR;
      }
      if (!valid_positive_integer(word[1])) {
        display_error(1, word[1], FALSE);
        return EXTRACT_ARG_ERROR;
      }
      /*
       * Now have two numbers, do they specify a position
       * that is currently viewable
       */
      y = atoi((char *) word[0]) - 1;
      x = atoi((char *) word[1]) - 1;
      syntax_element = get_syntax_element(current_screen, y, x);
    }
  }
  switch (syntax_element) {

    case THE_SYNTAX_NONE:
      item_values[1].value = (char_t *) "NONE";
      item_values[1].len = 4;
      break;

    case THE_SYNTAX_COMMENT:
      item_values[1].value = (char_t *) "COMMENT";
      item_values[1].len = 7;
      break;

    case THE_SYNTAX_FUNCTION:
      item_values[1].value = (char_t *) "FUNCTION";
      item_values[1].len = 8;
      break;

    case THE_SYNTAX_HEADER:
      item_values[1].value = (char_t *) "HEADER";
      item_values[1].len = 6;
      break;

    case THE_SYNTAX_INCOMPLETESTRING:
      item_values[1].value = (char_t *) "INCOMPLETESTRING";
      item_values[1].len = 16;
      break;

    case THE_SYNTAX_KEYWORD:
      item_values[1].value = (char_t *) "KEYWORD";
      item_values[1].len = 7;
      break;

    case THE_SYNTAX_LABEL:
      item_values[1].value = (char_t *) "LABEL";
      item_values[1].len = 5;
      break;

    case THE_SYNTAX_MARKUP:
      item_values[1].value = (char_t *) "MARKUP";
      item_values[1].len = 6;
      break;

    case THE_SYNTAX_MATCH:
      item_values[1].value = (char_t *) "MATCH";
      item_values[1].len = 5;
      break;

    case THE_SYNTAX_NUMBER:
      item_values[1].value = (char_t *) "NUMBER";
      item_values[1].len = 6;
      break;

    case THE_SYNTAX_POSTCOMPARE:
      item_values[1].value = (char_t *) "POSTCOMPARE";
      item_values[1].len = 11;
      break;

    case THE_SYNTAX_PREPROCESSOR:
      item_values[1].value = (char_t *) "PREPROCESSOR";
      item_values[1].len = 12;
      break;

    case THE_SYNTAX_STRING:
      item_values[1].value = (char_t *) "STRING";
      item_values[1].len = 6;
      break;

    default:
      item_values[1].value = (char_t *) "UNKNOWN";
      item_values[1].len = 7;
      break;
  }
  return (number_variables);
}

short extract_synonym(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int off = 0;
  char_t *ptr = NULL;
  DEFINE *curr;

  if (blank_field(itemargs)) {
    if (CURRENT_VIEW->synonym) {
      item_values[1].value = (char_t *) "ON";
      item_values[1].len = 2;
      number_variables = 1;
    } else {
      item_values[1].value = (char_t *) "OFF";
      item_values[1].len = 3;
      number_variables = 1;
    }
    return (number_variables);
  } else if (strcmp((char *) itemargs, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      display_error(1, (char_t *) itemargs, FALSE);
      return EXTRACT_ARG_ERROR;
    }
    for (number_variables = 0, curr = first_synonym; curr != NULL; curr = curr->next) {
      strcpy((char *) query_rsrvd, "");
      ptr = build_synonym_definition(curr, curr->synonym, query_rsrvd, TRUE);
      number_variables++;
      item_values[number_variables].len = strlen((char *) query_rsrvd);
      memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
      item_values[number_variables].value = trec + off;
      off += (item_values[number_variables].len) + 1;
    }
  } else {
    /*
     * Try and find a matching synonym
     */
    for (curr = first_synonym; curr != NULL; curr = curr->next) {
      if (equal(curr->synonym, itemargs, curr->def_funkey)) {
        break;
      }
    }
    if (query_type == QUERY_QUERY) {
      strcpy((char *) query_rsrvd, "");
      ptr = build_synonym_definition(curr, itemargs, query_rsrvd, TRUE);
      sprintf((char *) trec, "synonym %s", ptr);
      display_error(0, trec, TRUE);
      number_variables = EXTRACT_VARIABLES_SET;
    } else {
      /*
       * EXTRACT
       */
      if (curr) {
        strcpy((char *) query_rsrvd, "");
        ptr = build_synonym_definition(curr, itemargs, query_rsrvd, FALSE);
        item_values[1].value = curr->synonym;
        item_values[1].len = strlen((char *) curr->synonym);
        item_values[2].len = sprintf((char *) query_num1, "%d", curr->def_funkey);
        item_values[2].value = query_num1;
        item_values[3].value = ptr;
        item_values[3].len = strlen((char *) ptr);
        item_values[4].len = sprintf((char *) query_num2, "%c", curr->linend);
        item_values[4].value = query_num2;
      } else {
        item_values[1].value = item_values[3].value = itemargs;
        item_values[1].len = item_values[3].len = strlen((char *) itemargs);
        item_values[2].len = sprintf((char *) query_num1, "%ld", item_values[1].len);
        item_values[2].value = query_num1;
        item_values[4].value = (char_t *) "";
        item_values[4].len = 0;
      }
      number_variables = 4;
    }
  }
  return (number_variables);
}

short extract_tabkey(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (tabkey_overwrite == 'T') {
    item_values[1].value = (char_t *) "TAB";
    item_values[1].len = 3;
  } else {
    item_values[1].value = (char_t *) "CHARACTER";
    item_values[1].len = 9;
  }
  if (tabkey_insert == 'T') {
    item_values[2].value = (char_t *) "TAB";
    item_values[2].len = 3;
  } else {
    item_values[2].value = (char_t *) "CHARACTER";
    item_values[2].len = 9;
  }
  return number_variables;
}

short extract_tabline(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  set_on_off_value(CURRENT_VIEW->tab_on, 1);
  if (CURRENT_VIEW->tab_base == POSITION_MIDDLE) {
    sprintf((char *) query_rsrvd, "M%+d", CURRENT_VIEW->tab_off);
  } else {
    sprintf((char *) query_rsrvd, "%d", CURRENT_VIEW->tab_off);
  }
  item_values[2].value = query_rsrvd;
  item_values[2].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_tabs(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  register int i = 0;

  strcpy((char *) query_rsrvd, "");
  if (CURRENT_VIEW->tabsinc) {
    sprintf((char *) query_rsrvd, "INCR %d", CURRENT_VIEW->tabsinc);
  } else {
    for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
      sprintf((char *) query_num1, "%ld ", CURRENT_VIEW->tabs[i]);
      strcat((char *) query_rsrvd, (char *) query_num1);
    }
    if (query_type == QUERY_QUERY || query_type == QUERY_STATUS) {
      query_rsrvd[COLS - 7] = '\0';
    }
  }
  item_values[1].value = query_rsrvd;
  item_values[1].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_tabsin(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  set_on_off_value(TABI_ONx, 1);
  sprintf((char *) query_num1, "%d", TABI_Nx);
  item_values[2].value = query_num1;
  item_values[2].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_tabsout(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  set_on_off_value(CURRENT_FILE->tabsout_on, 1);
  sprintf((char *) query_num1, "%d", CURRENT_FILE->tabsout_num);
  item_values[2].value = query_num1;
  item_values[2].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_targetsave(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  int len = 0;

  switch (TARGETSAVEx) {

    case TARGET_ALL:
      item_values[1].value = (char_t *) "ALL";
      item_values[1].len = 3;
      break;

    case TARGET_UNFOUND:
      item_values[1].value = (char_t *) "NONE";
      item_values[1].len = 4;
      break;

    default:
      strcpy((char *) query_rsrvd, "");
      if (TARGETSAVEx & TARGET_STRING) {
        strcat((char *) query_rsrvd, "STRING ");
      }
      if (TARGETSAVEx & TARGET_REGEXP) {
        strcat((char *) query_rsrvd, "REGEXP ");
      }
      if (TARGETSAVEx & TARGET_ABSOLUTE) {
        strcat((char *) query_rsrvd, "ABSOLUTE ");
      }
      if (TARGETSAVEx & TARGET_RELATIVE) {
        strcat((char *) query_rsrvd, "RELATIVE ");
      }
      if (TARGETSAVEx & TARGET_POINT) {
        strcat((char *) query_rsrvd, "POINT ");
      }
      if (TARGETSAVEx & TARGET_BLANK) {
        strcat((char *) query_rsrvd, "BLANK ");
      }
      len = strlen((char *) query_rsrvd);
      if (query_rsrvd[len - 1] == ' ') {
        query_rsrvd[len - 1] = '\0';
        len--;
      }
      item_values[1].value = query_rsrvd;
      item_values[1].len = len;
      break;
  }
  return number_variables;
}

short extract_terminal(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].value = term_name;
  item_values[1].len = strlen((char *) term_name);
  item_values[2].len = sprintf((char *) query_num1, "%d", LINES);
  item_values[2].value = query_num1;
  item_values[3].len = sprintf((char *) query_num2, "%d", COLS);
  item_values[3].value = query_num2;
  return number_variables;
}

short extract_thighlight(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->thighlight_on, 1);
}

short extract_timecheck(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_FILE->timecheck, 1);
}

short extract_tof(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value((bool) CURRENT_TOF, 1);
}

short extract_tofeof(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->tofeof, 1);
}

short extract_tof_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_boolean_value((bool) (FOCUS_TOF && CURRENT_VIEW->current_window != WINDOW_COMMAND), (short) 1);
}

short extract_topedge_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  short y = 0;

  if (batch_only) {
    item_values[1].value = (char_t *) "0";
    item_values[1].len = 1;
    return 1;
  }
  y = getcury(CURRENT_WINDOW);
  return set_boolean_value((bool) (CURRENT_VIEW->current_window == WINDOW_FILEAREA && y == 0), (short) 1);
}

short extract_trailing(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  switch (CURRENT_FILE->trailing) {

    case TRAILING_ON:
      item_values[1].value = (char_t *) "ON";
      item_values[1].len = 2;
      break;

    case TRAILING_OFF:
      item_values[1].value = (char_t *) "OFF";
      item_values[1].len = 3;
      break;

    case TRAILING_SINGLE:
      item_values[1].value = (char_t *) "SINGLE";
      item_values[1].len = 6;
      break;

    case TRAILING_EMPTY:
      item_values[1].value = (char_t *) "EMPTY";
      item_values[1].len = 5;
      break;

    case TRAILING_REMOVE:
      item_values[1].value = (char_t *) "REMOVE";
      item_values[1].len = 6;
      break;

    default:
      break;
  }
  return number_variables;
}

short extract_typeahead(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(TYPEAHEADx, 1);
}

short extract_ui(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_rsrvd, "%s", curses_version());
  item_values[1].value = query_rsrvd;
  item_values[1].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_undoing(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_FILE->undoing, 1);
}

short extract_untaa(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(UNTAAx, 1);
}

short extract_utf8(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(0, 1);
}

short extract_verify(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num3, "%ld %ld", CURRENT_VIEW->verify_start, CURRENT_VIEW->verify_end);
  item_values[1].value = query_num3;
  item_values[1].len = strlen((char *) query_num3);
  return number_variables;
}

short extract_vershift(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_rsrvd, "%ld", (line_t) CURRENT_VIEW->verify_col - (line_t) CURRENT_VIEW->verify_start);
  item_values[1].value = query_rsrvd;
  item_values[1].len = strlen((char *) query_rsrvd);
  return number_variables;
}

short extract_verone_function(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_boolean_value((bool) (CURRENT_VIEW->verify_col == 1), (short) 1);
}

short extract_version(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  item_values[1].value = (char_t *) "THE";
  item_values[1].len = 3;
  item_values[2].value = (char_t *) the_version;
  item_values[2].len = strlen((char *) the_version);
  item_values[3].value = (char_t *) "???";
  item_values[3].value = (char_t *) "UNIX";
  item_values[3].len = strlen((char *) item_values[3].value);
  item_values[4].value = (char_t *) the_release;
  item_values[4].len = strlen((char *) item_values[4].value);
  item_values[5].value = item_values[3].value;
  item_values[5].len = item_values[3].len;
  return number_variables;
}

short extract_width(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%ld", max_line_length);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  return number_variables;
}

short extract_word(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  if (CURRENT_VIEW->word == 'A') {
    item_values[1].value = (char_t *) "ALPHANUM";
    item_values[1].len = 8;
  } else {
    item_values[1].value = (char_t *) "NONBLANK";
    item_values[1].len = 8;
  }
  return number_variables;
}

short extract_wordwrap(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->wordwrap, 1);
}

short extract_wrap(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  return set_on_off_value(CURRENT_VIEW->wrap, 1);
}

short extract_zone(short number_variables, short itemno, char_t *itemargs, char_t query_type, line_t argc, char_t *arg, line_t arglen) {
  sprintf((char *) query_num1, "%ld", CURRENT_VIEW->zone_start);
  item_values[1].value = query_num1;
  item_values[1].len = strlen((char *) query_num1);
  sprintf((char *) query_num2, "%ld", CURRENT_VIEW->zone_end);
  item_values[2].value = query_num2;
  item_values[2].len = strlen((char *) query_num2);
  return number_variables;
}

