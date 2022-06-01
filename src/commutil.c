/* COMMUTIL.C -                                                        */
/* This file contains all utility functions used when processing       */
/* commands.                                                           */
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

#include "the.h"
#include "proto.h"
#include "key.h"
#include "command.h"

static char_t *build_defined_key_definition(int, char_t *, DEFINE *, int);
static void save_last_command(char_t *, char_t *);
void AdjustThighlight(int);
static bool save_target(TARGET *);

#define HEXVAL(c) (((c)>'9')?(tolower(c)-'a'+10):((c)-'0'))

static char_t *cmd_history[MAX_SAVED_COMMANDS];
static short cmd_history_len[MAX_SAVED_COMMANDS];
static short last_cmd = (-1), current_cmd = 0, number_cmds = 0, offset_cmd = 0;
char_t *last_command_for_reexecute;
short last_command_for_reexecute_len;
char_t *last_command_for_repeat;
short last_command_for_repeat_len;
char_t *last_command_for_repeat_in_macro;
short last_command_for_repeat_in_macro_len;

#define KEY_REDEF "/* Key re-definitions */"
#define KEY_DEFAULT "/* Default key definitions */"
#define KEY_MOUSE_REDEF "/* Mouse definitions */"
/*
 * The following two static variables are for reserving space for the
 * parameters of a command. Space for temp_params is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_params is > length_temp_params, reallocate a larger area and
 * set the value of length_temp_params to reflect the new size.
 */
static char_t *temp_params = NULL;
static length_t length_temp_params = 0;

/*
 * The following two static variables are for reserving space for the
 * parameters of an explicit SET command. Space for temp_set_params is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_set_params is > length_temp_set_params, reallocate a larger area and
 * set the value of length_temp_set_params to reflect the new size.
 */
static char_t *temp_set_params = NULL;
static length_t length_temp_set_params = 0;

/*
 * The following two static variables are for reserving space for the
 * directories in a macro path. Space for temp_macros is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_macros is > length_temp_macros  reallocate a larger area and
 * set the value of length_temp_macros to reflect the new size.
 */
static char_t *temp_macros = NULL;
static length_t length_temp_macros = 0;

/*
 * The following two static variables are for reserving space for the
 * contents of   a command. Space for tmp_cmd     is allocated and
 * freed in the.c. If the size of the string to be placed into
 * tmp_cmd     is > length_tmp_cmd    , reallocate a larger area and
 * set the value of length_tmp_cmd     to reflect the new size.
 */
static char_t *tmp_cmd = NULL;
static length_t length_tmp_cmd = 0;

/*
 * The following two        variables are for reserving space for the
 * contents of   a command. Space for temp_cmd    is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_cmd    is > length_temp_cmd   , reallocate a larger area and
 * set the value of length_temp_cmd    to reflect the new size.
 */
char_t *temp_cmd = NULL;
static length_t length_temp_cmd = 0;

/*
 * The following two are to specify the first and last items in the
 * linked list for key definitions.
 */
DEFINE *first_define = NULL;
DEFINE *last_define = NULL;

/*
 * The following two are to specify the first and last items in the
 * linked list for command synonyms.
 */
DEFINE *first_synonym = NULL;
DEFINE *last_synonym = NULL;

/*
 * The following two are to specify the first and last items in the
 * linked list for mouse button definitions.
 */
DEFINE *first_mouse_define = NULL;
DEFINE *last_mouse_define = NULL;

/*
 * Key re-definition pseudo files.
 */
LINE *key_first_line = NULL;
LINE *key_last_line = NULL;
line_t key_number_lines = 0L;

AREAS valid_areas[ATTR_MAX] = {
  { (char_t *) "FILEAREA", 1, WINDOW_FILEAREA, TRUE },
  { (char_t *) "CURLINE", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "BLOCK", 1, WINDOW_FILEAREA, FALSE },
  { (char_t *) "CBLOCK", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "CMDLINE", 1, WINDOW_COMMAND, TRUE },
  { (char_t *) "IDLINE", 1, WINDOW_IDLINE, TRUE },
  { (char_t *) "MSGLINE", 1, WINDOW_ERROR, FALSE },
  { (char_t *) "ARROW", 1, WINDOW_ARROW, TRUE },
  { (char_t *) "PREFIX", 2, WINDOW_PREFIX, TRUE },
  { (char_t *) "CPREFIX", 3, WINDOW_PREFIX, TRUE },
  { (char_t *) "PENDING", 1, WINDOW_PREFIX, FALSE },
  { (char_t *) "SCALE", 1, WINDOW_FILEAREA, FALSE },
  { (char_t *) "TOFEOF", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "CTOFEOF", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "TABLINE", 1, WINDOW_FILEAREA, FALSE },
  { (char_t *) "SHADOW", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "STATAREA", 2, WINDOW_STATAREA, TRUE },
  { (char_t *) "DIVIDER", 1, WINDOW_DIVIDER, TRUE },
  { (char_t *) "RESERVED", 1, WINDOW_RESERVED, FALSE },
  { (char_t *) "NONDISP", 1, WINDOW_FILEAREA, FALSE },
  { (char_t *) "HIGHLIGHT", 2, WINDOW_FILEAREA, FALSE },
  { (char_t *) "CHIGHLIGHT", 3, WINDOW_FILEAREA, FALSE },
  { (char_t *) "THIGHLIGHT", 5, WINDOW_FILEAREA, FALSE },
  { (char_t *) "SLK", 3, WINDOW_SLK, FALSE },
  { (char_t *) "GAP", 3, WINDOW_PREFIX, FALSE },
  { (char_t *) "CGAP", 4, WINDOW_PREFIX, FALSE },
  { (char_t *) "ALERT", 5, WINDOW_DIVIDER, FALSE },
  { (char_t *) "DIALOG", 6, WINDOW_DIVIDER, FALSE },
  { (char_t *) "BOUNDMARKER", 5, WINDOW_FILEAREA, FALSE },
  { (char_t *) "FILETABS", 8, WINDOW_FILETABS, TRUE },
  { (char_t *) "FILETABSDIV", 11, WINDOW_FILETABS, TRUE },
  { (char_t *) "CURSORLINE", 6, WINDOW_FILEAREA, TRUE },
  { (char_t *) "DIALOGBORDER", 12, WINDOW_DIVIDER, FALSE },
  { (char_t *) "DIALOGEDITFIELD", 15, WINDOW_DIVIDER, FALSE },
  { (char_t *) "DIALOGBUTTON", 12, WINDOW_DIVIDER, FALSE },
  { (char_t *) "DIALOGABUTTON", 13, WINDOW_DIVIDER, FALSE },
  { (char_t *) "POPUPBORDER", 11, WINDOW_DIVIDER, FALSE },
  { (char_t *) "POPUPCURLINE", 12, WINDOW_DIVIDER, FALSE },
  { (char_t *) "POPUP", 5, WINDOW_DIVIDER, FALSE },
  { (char_t *) "POPUPDIVIDER", 12, WINDOW_DIVIDER, FALSE },
};

char_t *get_key_name(int key, int *shift) {
  register short i = 0;
  char_t *keyname = NULL;

  /*
   * Get name of key...
   */
  *shift = 0;
  for (i = 0; key_table[i].mnemonic != NULL; i++) {
    if (key == key_table[i].key_value) {
      keyname = key_table[i].mnemonic;
      *shift = key_table[i].shift;
      break;
    }
  }
  return (keyname);
}
char_t *get_key_definition(int key, int define_format, bool default_keys, bool mouse_key) {
  register short i = 0;
  DEFINE *curr = NULL;
  bool check_redefined = TRUE;
  bool check_default = TRUE;
  char_t *keyname = NULL;
  char_t key_buf[50];
  int dummy = 0;

  /*
   * First determine if the key is a named key.
   */
  if (mouse_key)
    keyname = mouse_key_number_to_name(key, key_buf, &dummy);
  else
    keyname = get_key_name(key, &dummy);
  /*
   * If key is invalid,  show it as a character and decimal; provided it
   * is an ASCII or extended character.
   */
  if (keyname == NULL) {
    if (define_format == THE_KEY_DEFINE_DEFINE)
      sprintf((char *) temp_cmd, "\"define \\%d ", key);
    else if (define_format == THE_KEY_DEFINE_SHOW)
      sprintf((char *) temp_cmd, "Key: \\%d", key);
    else
      strcpy((char *) temp_cmd, "");
  } else {
    if (define_format == THE_KEY_DEFINE_DEFINE) {
      sprintf((char *) temp_cmd, "\"define %s ", (char *) keyname);
    } else if (define_format == THE_KEY_DEFINE_SHOW) {
      sprintf((char *) temp_cmd, "Key: %s ", (char *) keyname);
    } else if (define_format == THE_KEY_DEFINE_QUERY) {
      sprintf((char *) temp_cmd, "%s ", (char *) keyname);
    } else
      strcpy((char *) temp_cmd, "");
  }

  if (mouse_key)
    check_default = check_redefined = FALSE;
  else {
    if (define_format == THE_KEY_DEFINE_DEFINE) {
      check_default = (default_keys) ? TRUE : FALSE;
      check_redefined = (default_keys) ? FALSE : TRUE;
    }
  }
  /*
   * If we want to first check redefined keys...
   */
  if (check_redefined) {
    /*
     * Next check to see if the key has been "defined".
     */
    curr = first_define;
    if (build_defined_key_definition(key, temp_cmd, curr, define_format) != (char_t *) NULL) {
      if (define_format != THE_KEY_DEFINE_RAW && define_format != THE_KEY_DEFINE_QUERY)
        strcat((char *) temp_cmd, "\"");
      return (temp_cmd);
    }
  }
  /*
   * If not, check for the default function key values.
   */
  if (check_default) {
    for (i = 0; command[i].text != NULL; i++) {
      if (key == command[i].funkey) {
        if (define_format == THE_KEY_DEFINE_DEFINE)
          strcat((char *) temp_cmd, " ");
        else if (define_format == THE_KEY_DEFINE_SHOW)
          strcat((char *) temp_cmd, " - assigned to \"");
        build_default_key_definition(i, temp_cmd);
        if (define_format != THE_KEY_DEFINE_RAW && define_format != THE_KEY_DEFINE_QUERY)
          strcat((char *) temp_cmd, "\"");
        return (temp_cmd);
      }
    }
  }
  /*
   * If we want to check mouse key definitions.
   */
  if (mouse_key) {
    /*
     * Next check to see if the key has been "defined".
     */
    curr = first_mouse_define;
    if (build_defined_key_definition(key, temp_cmd, curr, define_format) != (char_t *) NULL) {
      if (define_format != THE_KEY_DEFINE_RAW && define_format != THE_KEY_DEFINE_QUERY)
        strcat((char *) temp_cmd, "\"");
      return (temp_cmd);
    }
  }
  /*
   * If none of the above, it is unassigned. We should never get here if
   * define_format is TRUE.
   */
  if (define_format == THE_KEY_DEFINE_RAW) {
    if (key < 256 && key >= 0) {
      if (etmode_flag[key]) {
        sprintf((char *) temp_cmd, "TEXT d'%d'", key);
      } else {
        sprintf((char *) temp_cmd, "TEXT %c", (char) key);
      }
    }
  } else {
    strcat((char *) temp_cmd, " - unassigned");
  }
  return (temp_cmd);
}

static short execute_synonym(char_t * synonym, char_t * params) {
  DEFINE *curr = (DEFINE *) NULL;
  short rc = RC_FILE_NOT_FOUND;
  char_t *key_cmd = NULL;
  short macrorc = 0;
  int tokenised = 0;
  int params_len, def_params_len;
  char *spc;

  /*
   * First check to see if the function key has been redefined and save
   * all key redefinitions.  This is because we may be redefining a
   * function key in the middle of its redefinition. eg ENTER key
   */
  curr = first_synonym;
  while (curr != (DEFINE *) NULL) {
    if (equal(curr->synonym, synonym, curr->def_funkey)) {
      /*
       * If there are no more files in the ring, and the command is not a
       * command to edit a new file, then ignore the command.
       */
      if (number_of_files == 0 && curr->def_command != (-1)
          && !command[curr->def_command].edit_command) {
        rc = RC_OK;
        curr = NULL;
        break;
      }
      /*
       * If running in read-only mode and the function selected is not valid
       * display an error.
       */
      if (number_of_files != 0 && ISREADONLY(CURRENT_FILE)
          && curr->def_command != (-1)
          && !command[curr->def_command].valid_in_readonly) {
        display_error(56, (char_t *) "", FALSE);
        rc = RC_INVALID_ENVIRON;
        curr = NULL;
        break;
      }
      /*
       * We need to sort out synonym arguments now. We can have arguments
       * from the synonym's definition and also from the command line.
       * Append them in that order.
       */
      def_params_len = strlen((char *) curr->def_params);
      params_len = strlen((char *) params);
      if ((key_cmd = (char_t *) malloc(def_params_len + params_len + 2)) == NULL) {
        display_error(30, (char_t *) "", FALSE);
        rc = RC_OUT_OF_MEMORY;
        curr = NULL;
        break;
      }
      if (def_params_len && params_len)
        spc = " ";
      else
        spc = "";
      sprintf((char *) key_cmd, "%s%s%s", (char *) curr->def_params, spc, (char *) params);
      if (curr->def_command == (-1)) {
        rc = execute_macro_instore(key_cmd, &macrorc, &curr->pcode, &curr->pcode_len, &tokenised, curr->def_funkey);
        if (tokenised) {
          tokenised = 0;
        }
        if (number_of_files > 0) {
          if (display_screens > 1) {
            build_screen((char_t) other_screen);
            display_screen((char_t) other_screen);
          }
          build_screen(current_screen);
          display_screen(current_screen);
        }
      } else {
        /*
         * If operating in CUA mode, and a CUA block exists, check
         * if the block should be reset or deleted before executing
         * the command.
         */
        if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
          ResetOrDeleteCUABlock(command[curr->def_command].cua_behaviour);
        }
        /*
         * Possibly reset the THIGHLIGHT area
         */
        if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
          AdjustThighlight(command[curr->def_command].thighlight_behaviour);
        }
        rc = (*command[curr->def_command].function) ((char_t *) key_cmd);
      }
      free(key_cmd);
    }
    curr = curr->next;
  }
  return rc;
}

short function_key(int key, int option, bool mouse_details_present) {
  register short i = 0;
  DEFINE *curr = (DEFINE *) NULL;
  DEFINE *tcurr = (DEFINE *) NULL;
  DEFINE *first_save = (DEFINE *) NULL, *last_save = (DEFINE *) NULL;
  short rc = RC_OK;
  short len = 0, num_cmds = 0;
  char_t *key_cmd = NULL;
  char_t tmpnum[15];
  short macrorc = 0;
  int tokenised = 0;

  /*
   * Reset last command found index
   */
  last_command_index = -1;
  if (mouse_details_present) {
    rc = execute_mouse_commands(key);
    return (rc);
  }
  /*
   * First check to see if the function key has been redefined and save
   * all key redefinitions.  This is because we may be redefining a
   * function key in the middle of its redefinition. eg ENTER key
   */
  curr = first_define;
  while (curr != (DEFINE *) NULL) {
    if (key == curr->def_funkey) {
      rc = append_define(&first_save, &last_save, key, curr->def_command, curr->def_params, curr->pcode, curr->pcode_len, NULL, 0);
      if (rc != RC_OK) {
        return (rc);
      }
    }
    curr = curr->next;
  }
  /*
   * Now we have saved any current definition of the function key, use
   * these definitions to drive the following...
   */
  curr = first_save;
  while (curr != (DEFINE *) NULL) {
    switch (option) {
      case OPTION_NORMAL:
        /*
         * If there are no more files in the ring, and the command is not a
         * command to edit a new file, then ignore the command.
         */
        if (number_of_files == 0 && curr->def_command != (-1)
            && !command[curr->def_command].edit_command) {
          rc = RC_OK;
          curr = NULL;
          break;
        }
        /*
         * If running in read-only mode and the function selected is not valid
         * display an error.
         */
        if (number_of_files != 0 && ISREADONLY(CURRENT_FILE)
            && curr->def_command != (-1)
            && !command[curr->def_command].valid_in_readonly) {
          display_error(56, (char_t *) "", FALSE);
          rc = RC_INVALID_ENVIRON;
          curr = NULL;
          break;
        }
        if ((key_cmd = (char_t *) my_strdup(curr->def_params)) == NULL) {
          display_error(30, (char_t *) "", FALSE);
          rc = RC_OUT_OF_MEMORY;
          curr = NULL;
          break;
        }
        if (curr->def_command == (-1)) {
          rc = execute_macro_instore(key_cmd, &macrorc, &curr->pcode, &curr->pcode_len, &tokenised, curr->def_funkey);
          if (tokenised) {
            tokenised = 0;
            tcurr = first_define;
            while (tcurr != (DEFINE *) NULL) {
              if (key == tcurr->def_funkey) {
                /*
                 * The pcode returned from the interpreter needs
                 * to be transfered from the copy of the key
                 * definition (curr and first_save to the "real"
                 * key definition found here.  Setting the
                 * pcode pointer in tcurr to the pcode in curr
                 * and then setting pcode in curr to NULL stops
                 * the later call to dll_free() from freeing
                 * the memory associated with pcode.
                 */
                tcurr->pcode = curr->pcode;
                tcurr->pcode_len = curr->pcode_len;
                curr->pcode = NULL;
                break;
              }
              tcurr = tcurr->next;
            }
          }
          if (number_of_files > 0) {
            if (display_screens > 1) {
              build_screen((char_t) other_screen);
              display_screen((char_t) other_screen);
            }
            build_screen(current_screen);
            display_screen(current_screen);
          }
        } else {
          /*
           * If operating in CUA mode, and a CUA block exists, check
           * if the block should be reset or deleted before executing
           * the command.
           */
          if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
            ResetOrDeleteCUABlock(command[curr->def_command].cua_behaviour);
          }
          /*
           * Possibly reset the THIGHLIGHT area
           */
          if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
            AdjustThighlight(command[curr->def_command].thighlight_behaviour);
          }
          rc = (*command[curr->def_command].function) ((char_t *) key_cmd);
        }
        free(key_cmd);
        if (rc != RC_OK && rc != RC_TOF_EOF_REACHED && rc != RC_NO_LINES_CHANGED && rc != RC_TARGET_NOT_FOUND) {
          curr = NULL;
          break;
        }
        break;
      case OPTION_EXTRACT:
        /*
         * If the request is to extract a keys commands, set a REXX variable
         * for each command associated with the function key.
         */
        if (curr->def_command != (-1)) {
          len = strlen((char *) command[curr->def_command].text) + strlen((char *) curr->def_params) + 2;
        } else {
          len = strlen((char *) curr->def_params) + 1;
        }
        if ((key_cmd = (char_t *) malloc(len)) == NULL) {
          display_error(30, (char_t *) "", FALSE);
          curr = NULL;
          rc = RC_OUT_OF_MEMORY;
          break;
        }
        if (curr->def_command != (-1)) {
          strcpy((char *) key_cmd, (char *) command[curr->def_command].text);
          strcat((char *) key_cmd, " ");
          strcat((char *) key_cmd, (char *) curr->def_params);
        } else
          strcpy((char *) key_cmd, (char *) curr->def_params);
        rc = set_rexx_variable((char_t *) "SHOWKEY", key_cmd, strlen((char *) key_cmd), ++num_cmds);
        free(key_cmd);
        break;
      case OPTION_READV:
        /*
         * If the key hit is KEY_ENTER, KEY_RETURN or KEY_NUMENTER, or TAB terminate
         * the READV CMDLINE command.
         */
        if (key == KEY_RETURN || key == KEY_ENTER || key == KEY_C_m || key == KEY_NUMENTER || key == 9) {
          rc = RC_READV_TERM;
          curr = NULL;
          break;
        }
        /*
         * If the command is not allowed in READV CMDLINE, or a REXX macro is
         * assigned to the key, return with an error.
         */
        if (curr->def_command == (-1)) {
          rc = RC_INVALID_ENVIRON;
          curr = NULL;
          break;
        }
        if (!command[curr->def_command].valid_readv) {
          rc = RC_INVALID_ENVIRON;
          curr = NULL;
          break;
        }
        /*
         * To get here, a valid READV CMDLINE command is present; execute it.
         */
        if ((key_cmd = (char_t *) my_strdup(curr->def_params)) == NULL) {
          display_error(30, (char_t *) "", FALSE);
          rc = RC_OUT_OF_MEMORY;
          curr = NULL;
          break;
        }
        rc = (*command[curr->def_command].function) ((char_t *) key_cmd);
        free(key_cmd);
        if (rc != RC_OK && rc != RC_TOF_EOF_REACHED && rc != RC_NO_LINES_CHANGED && rc != RC_TARGET_NOT_FOUND) {
          curr = NULL;
          break;
        }
        break;
      default:
        break;
    }
    if (curr == NULL)
      break;
    curr = curr->next;
  }
  /*
   * If the key has been redefined, exit here...
   */
  if (first_save) {
    if (option == OPTION_EXTRACT) {
      sprintf((char *) tmpnum, "%d", num_cmds);
      rc = set_rexx_variable((char_t *) "SHOWKEY", tmpnum, strlen((char *) tmpnum), 0);
      rc = num_cmds;
    }
    dll_free(first_save);
    return (rc);
  }
  /*
   * If not, check for the default function key values.
   */
  for (i = 0; command[i].text != NULL; i++) {
    if (key == command[i].funkey) {
      switch (option) {
        case OPTION_NORMAL:
          /*
           * If running in read-only mode and the function selected is not valid
           * display an error.
           */
          if (number_of_files != 0 && ISREADONLY(CURRENT_FILE)
              && !command[i].valid_in_readonly) {
            display_error(56, (char_t *) "", FALSE);
            rc = RC_INVALID_ENVIRON;
            break;
          }
          if ((key_cmd = (char_t *) my_strdup(command[i].params)) == NULL) {
            display_error(30, (char_t *) "", FALSE);
            rc = RC_OUT_OF_MEMORY;
            break;
          }
          /*
           * If operating in CUA mode, and a CUA block exists, check
           * if the block should be reset or deleted before executing
           * the command.
           */
          if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
            ResetOrDeleteCUABlock(command[i].cua_behaviour);
          }
          /*
           * Possibly reset the THIGHLIGHT area
           */
          if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
            AdjustThighlight(command[i].thighlight_behaviour);
          }
          rc = (*command[i].function) ((char_t *) key_cmd);
          free(key_cmd);
          break;
        case OPTION_EXTRACT:
          len = strlen((char *) command[i].text) + strlen((char *) command[i].params) + 10;
          if ((key_cmd = (char_t *) malloc(len)) == NULL) {
            display_error(30, (char_t *) "", FALSE);
            rc = RC_OUT_OF_MEMORY;
            break;
          }
          strcpy((char *) key_cmd, "");
          key_cmd = build_default_key_definition(i, key_cmd);
          rc = set_rexx_variable((char_t *) "SHOWKEY", key_cmd, strlen((char *) key_cmd), 1);
          free(key_cmd);
          rc = set_rexx_variable((char_t *) "SHOWKEY", (char_t *) "1", 1, 0);
          break;
        case OPTION_READV:
          /*
           * If the key hit is KEY_ENTER, KEY_RETURN or KEY_NUMENTER, terminate
           * the READV CMDLINE command.
           */
          if (key == KEY_RETURN || key == KEY_ENTER || key == KEY_C_m || key == KEY_NUMENTER || key == 9) {
            rc = RC_READV_TERM;
            break;
          }
          /*
           * If the command is not allowed in READV CMDLINE, return with an error
           */
          if (!command[i].valid_readv) {
            rc = RC_INVALID_ENVIRON;
            break;
          }
          /*
           * To get here, a valid READV CMDLINE command is present; execute it.
           */
          if ((key_cmd = (char_t *) my_strdup(command[i].params)) == NULL) {
            display_error(30, (char_t *) "", FALSE);
            rc = RC_OUT_OF_MEMORY;
            break;
          }
          rc = (*command[i].function) ((char_t *) key_cmd);
          free(key_cmd);
          break;
      }
      return (rc);
    }
  }
  if (option == OPTION_EXTRACT)
    rc = set_rexx_variable((char_t *) "SHOWKEY", (char_t *) "0", 1, 0);
  return (RAW_KEY);
}
bool is_modifier_key(int key) {
  register short i = 0;

  /*
   * Get name of key...
   */
  for (i = 0; key_table[i].mnemonic != NULL; i++) {
    if (key == key_table[i].key_value && key_table[i].shift == SHIFT_MODIFIER_ONLY) {
      return TRUE;
    }
  }
  return (FALSE);
}
char_t *build_default_key_definition(int key, char_t * buf) {
  /*
   * The argument, buf, MUST be long enough to to accept the full command
   * and arguments and MUST have be nul terminated before this function
   * is called.
   * ---------------------------------------------------------------------
   * If a SET command, prefix with 'set'
   */
  if (command[key].set_command)
    strcat((char *) buf, "set ");
  /*
   * If a SOS command, prefix with 'sos'
   */
  if (command[key].sos_command)
    strcat((char *) buf, "sos ");
  /*
   * Append the command name.
   */
  strcat((char *) buf, (char *) command[key].text);
  /*
   * Append any parameters.
   */
  if (strcmp((char *) command[key].params, "") != 0) {
    strcat((char *) buf, " ");
    strcat((char *) buf, (char *) command[key].params);
  }
  return (buf);
}
static char_t *build_defined_key_definition(int key, char_t * buf, DEFINE * curr, int define_format) {
  bool key_defined = FALSE;
  bool first_time = TRUE;
  char_t delim[2];

  /*
   * The argument, buf, MUST be long enough to to accept the full command
   * and arguments and MUST have be nul terminated before this function
   * is called.
   */
  delim[1] = '\0';
  delim[0] = CURRENT_VIEW->linend_value;
  while (curr != NULL) {
    if (key == curr->def_funkey) {
      key_defined = TRUE;
      if (first_time) {
        if (define_format == THE_KEY_DEFINE_DEFINE)
          strcat((char *) buf, " ");
        else if (define_format == THE_KEY_DEFINE_QUERY)
          strcat((char *) buf, " ");
        else if (define_format == THE_KEY_DEFINE_SHOW)
          strcat((char *) buf, " - assigned to \"");
      } else {
        strcat((char *) buf, (char *) delim);
      }
      /*
       * Append the command to the string.
       */
      if (curr->def_command == (-1))    /* definition is REXX instore */
        strcat((char *) buf, (char *) "REXX");
      else
        strcat((char *) buf, (char *) command[curr->def_command].text);
      /*
       * Append any parameters.
       */
      if (strcmp((char *) curr->def_params, "") != 0) {
        strcat((char *) buf, " ");
        strcat((char *) buf, (char *) curr->def_params);
      }
      first_time = FALSE;
    }
    curr = curr->next;
  }
  return ((key_defined) ? buf : (char_t *) NULL);
}

char_t *build_synonym_definition(DEFINE * curr, char_t * name, char_t * buf, bool full_definition) {
  char_t delim[2];
  char *cmd;

  /*
   * The argument, buf, MUST be long enough to to accept the full command
   * and arguments and MUST have be nul terminated before this function
   * is called.
   */
  delim[1] = '\0';
  if (curr) {
    if (curr->linend)
      delim[0] = curr->linend;
    else
      delim[0] = CURRENT_VIEW->linend_value;
    if (curr->def_command == (-1))      /* definition is REXX instore */
      cmd = "REXX";
    else
      cmd = (char *) command[curr->def_command].text;
    if (full_definition)
      sprintf((char *) buf, "%s %d%s%s %s%s%s", name, curr->def_funkey, (curr->linend) ? " LINEND " : "", (curr->linend) ? (char *) delim : "", cmd, (curr->def_params) ? " " : "", curr->def_params);
    else
      sprintf((char *) buf, "%s%s%s", cmd, (curr->def_params) ? " " : "", curr->def_params);
  } else {
    if (full_definition)
      sprintf((char *) buf, "%s %lu %s", name, (unsigned long) strlen((char *) name), name);
    else
      sprintf((char *) buf, "%s", name);
  }
  return (buf);
}

short display_all_keys(void) {
  LINE *curr = NULL;
  DEFINE *curr_define = NULL;
  int key = 0, save_funkey = 0;
  short rc = RC_OK;
  register int i = 0;
  char_t *keydef = NULL;
  VIEW_DETAILS *found_view = NULL;
  static bool first = TRUE;

  /*
   * If this is the first time we've called this, create the pseudo file
   * name.
   */
  if (first) {
    strcpy((char *) key_pathname, (char *) dir_pathname);
    strcat((char *) key_pathname, (char *) keyfilename);
    if ((rc = splitpath(key_pathname)) != RC_OK) {
      return (rc);
    }
    strcpy((char *) key_pathname, (char *) sp_path);
    strcpy((char *) key_filename, (char *) sp_fname);
    first = FALSE;
  }
  key_first_line = key_last_line = lll_free(key_first_line);
  key_number_lines = 0L;
  if ((found_view = find_file(key_pathname, key_filename)) != (VIEW_DETAILS *) NULL) {
    found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
    found_view->file_for_view->number_lines = 0L;
  }
  /*
   * first_line is set to "Top of File"
   */
  if ((key_first_line = add_LINE(key_first_line, NULL, TOP_OF_FILE, strlen((char *) TOP_OF_FILE), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * last line is set to "Bottom of File"
   */
  if ((key_last_line = add_LINE(key_first_line, key_first_line, BOTTOM_OF_FILE, strlen((char *) BOTTOM_OF_FILE), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  curr = key_first_line;
  /*
   * First display default key mappings...
   */
  if ((curr = add_LINE(key_first_line, curr, (char_t *) KEY_DEFAULT, strlen(KEY_DEFAULT), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  key_number_lines++;
  save_funkey = (-1);
  for (i = 0; command[i].text != NULL; i++) {
    if (command[i].funkey != (-1)
        && save_funkey != command[i].funkey) {
      save_funkey = command[i].funkey;
      keydef = get_key_definition(command[i].funkey, THE_KEY_DEFINE_DEFINE, TRUE, FALSE);
      if ((curr = add_LINE(key_first_line, curr, keydef, strlen((char *) keydef), 0, FALSE)) == NULL) {
        return (RC_OUT_OF_MEMORY);
      }
      key_number_lines++;
    }
  }
  /*
   * ...next, display any key redefinitions.
   */
  if ((curr = add_LINE(key_first_line, curr, (char_t *) KEY_REDEF, strlen(KEY_REDEF), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  key_number_lines++;
  curr_define = first_define;
  while (curr_define != NULL) {
    if (key != curr_define->def_funkey) {
      keydef = get_key_definition(curr_define->def_funkey, THE_KEY_DEFINE_DEFINE, FALSE, FALSE);
      if ((curr = add_LINE(key_first_line, curr, keydef, strlen((char *) keydef), 0, FALSE)) == NULL) {
        return (RC_OUT_OF_MEMORY);
      }
      key_number_lines++;
    }
    key = curr_define->def_funkey;
    curr_define = curr_define->next;
  }
  /*
   * ...last, display any mouse key definitions.
   */
  if ((curr = add_LINE(key_first_line, curr, (char_t *) KEY_MOUSE_REDEF, strlen(KEY_MOUSE_REDEF), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  key_number_lines++;
  curr_define = first_mouse_define;
  while (curr_define != NULL) {
    if (key != curr_define->def_funkey) {
      keydef = get_key_definition(curr_define->def_funkey, THE_KEY_DEFINE_DEFINE, FALSE, TRUE);
      if ((curr = add_LINE(key_first_line, curr, keydef, strlen((char *) keydef), 0, FALSE)) == NULL) {
        return (RC_OUT_OF_MEMORY);
      }
      key_number_lines++;
    }
    key = curr_define->def_funkey;
    curr_define = curr_define->next;
  }
  strcpy((char *) temp_cmd, (char *) key_pathname);
  strcat((char *) temp_cmd, (char *) key_filename);
  Xedit(temp_cmd);
  return (RC_OK);
}
int set_rexx_variables_for_all_keys(int key_type, int *number_keys_return) {
  DEFINE *curr_define = NULL;
  int key = 0, save_funkey = 0;
  short rc = RC_OK;
  register int i = 0;
  char_t *keydef = NULL;
  int number_keys = 0;

  *number_keys_return = 0;
  if (key_type == KEY_TYPE_KEY || key_type == KEY_TYPE_ALL) {
    save_funkey = (-1);
    for (i = 0; command[i].text != NULL; i++) {
      if (command[i].funkey != (-1)
          && save_funkey != command[i].funkey) {
        number_keys++;
        save_funkey = command[i].funkey;
        keydef = get_key_definition(command[i].funkey, THE_KEY_DEFINE_SHOW, TRUE, FALSE);
        if ((rc = set_rexx_variable((char_t *) "define", keydef, strlen((char *) keydef), number_keys)) != RC_OK) {
          return (rc);
        }
      }
    }
    /*
     * ...next, display any key redefinitions.
     */
    curr_define = first_define;
    while (curr_define != NULL) {
      if (key != curr_define->def_funkey) {
        number_keys++;
        keydef = get_key_definition(curr_define->def_funkey, THE_KEY_DEFINE_SHOW, FALSE, FALSE);
        if ((rc = set_rexx_variable((char_t *) "define", keydef, strlen((char *) keydef), number_keys)) != RC_OK) {
          return (rc);
        }
      }
      key = curr_define->def_funkey;
      curr_define = curr_define->next;
    }
  }
  /*
   * ...last, display any mouse key definitions.
   */
  if (key_type == KEY_TYPE_MOUSE || key_type == KEY_TYPE_ALL) {
    curr_define = first_mouse_define;
    while (curr_define != NULL) {
      if (key != curr_define->def_funkey) {
        number_keys++;
        keydef = get_key_definition(curr_define->def_funkey, THE_KEY_DEFINE_SHOW, FALSE, TRUE);
        if ((rc = set_rexx_variable((char_t *) "define", keydef, strlen((char *) keydef), number_keys)) != RC_OK) {
          return (rc);
        }
      }
      key = curr_define->def_funkey;
      curr_define = curr_define->next;
    }
  }
  *number_keys_return = number_keys;
  return (RC_OK);
}
short command_line(char_t * cmd_line, bool command_only) {
  bool valid_command = FALSE;
  bool target_found;
  bool linend_status = (number_of_files) ? CURRENT_VIEW->linend_status : LINEND_STATUSx;
  char_t linend_value = 0;
  register short i = 0, j = 0;
  short rc = RC_OK;
  char_t *cmd[MAX_COMMANDS + 1];
  unsigned short num_commands = 0;
  char_t command_delim[2];
  char_t *command_entered = NULL;
  char_t *saved_command = NULL;
  char_t *cl_cmd = NULL;
  char_t *cl_param = NULL;
  short macrorc = 0;
  bool display_parse_error;

  /*
   * If the command line is blank, just return.
   */
  if (blank_field(cmd_line)) {
    if (curses_started)
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
    return (RC_OK);
  }
  /*
   * Set up values for LINEND for later processing...
   */
  if (number_of_files == 0) {
    linend_status = LINEND_STATUSx;
    linend_value = LINEND_VALUEx;
  } else {
    linend_status = CURRENT_VIEW->linend_status;
    linend_value = CURRENT_VIEW->linend_value;
  }
  /*
   * If the command is to be kept displayed on the command line, copy it.
   */
  if (*(cmd_line) == '&') {
    if ((saved_command = (char_t *) my_strdup(cmd_line)) == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    cmd_line++;
  } else {
    cleanup_command_line();
  }
  /*
   * Copy the incoming cmd_line, so we can play with it.
   */
  if ((command_entered = (char_t *) my_strdup(cmd_line)) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Allocate some space to cl_cmd and cl_param for the a command when
   * it is split into a command and its parameters.
   */
  if ((cl_cmd = (char_t *) malloc((strlen((char *) cmd_line) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  if ((cl_param = (char_t *) malloc((strlen((char *) cmd_line) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * If [SET] LINENd is set to ON, split the line up into a number of
   * individual commands.
   */
  if (linend_status && !define_command(cmd_line)) {
    command_delim[0] = linend_value;
    command_delim[1] = '\0';
    num_commands = command_split(cmd_line, cmd, MAX_COMMANDS, command_delim, command_entered);
  } else {
    cmd[0] = command_entered;
    num_commands = 1;
  }
  /*
   * For each command entered, split it up into command and params, and
   * process it...
   */
  for (j = 0; j < num_commands; j++) {
    /*
     * Reset last command found index
     */
    last_command_index = -1;

    valid_command = FALSE;
    split_command(cmd[j], cl_cmd, cl_param);
    cl_cmd = MyStrip(cl_cmd, STRIP_BOTH, ' ');
    /*
     * Check for synonyms first.
     */
    if (!command_only && CURRENT_VIEW && CURRENT_VIEW->synonym) {
      rc = execute_synonym(cl_cmd, cl_param);
      if (rc != RC_FILE_NOT_FOUND) {
        lastrc = (rc == RC_SYSTEM_ERROR) ? rc : macrorc;
        save_last_command(cmd[j], cl_cmd);
        continue;
      }
    }
    /*
     * To reduce file accesses, check if command is COMMAND or MACRO, and treat
     * both as though they were already prefixed with COMMAND
     */
    if (strcasecmp((char *) "COMMAND", (char *) cl_cmd) == 0 || strcasecmp((char *) "MACRO", (char *) cl_cmd) == 0) {
      command_only = 1;
    }
    /*
     * Before we try the command array, if SET MACRO is ON and IMPMACRO is
     * ON and we have not reached here via COMMAND command, try for a
     * macro...
     */
    if (number_of_files > 0 && CURRENT_VIEW->macro && CURRENT_VIEW->imp_macro && !command_only) {
      rc = execute_macro(cmd[j], FALSE, &macrorc);
      if (rc != RC_FILE_NOT_FOUND) {
        lastrc = (rc == RC_SYSTEM_ERROR) ? rc : macrorc;
        save_last_command(cmd[j], cl_cmd);
        continue;
      }
    }
    /*
     * Look up the command in the command array in command.h
     */
    for (i = 0; command[i].text != NULL; i++) {
      /*
       * If no command text, continue.
       */
      if (strcmp((char *) command[i].text, "") == 0)
        continue;
      rc = RC_OK;
      /*
       * Check that the supplied command matches the command for the length
       * of the command and that the length is at least as long as the
       * necessary significance.
       */
      if (equal(command[i].text, cl_cmd, command[i].min_len)
          && command[i].min_len != 0 && !command[i].sos_command) {
        /*
         * Found the matching command. Save it for extended error reporting
         */
        if (ERRORFORMATx == 'E')
          last_command_index = i;
        if (batch_only && !command[i].valid_batch_command) {
          display_error(24, command[i].text, FALSE);
          lastrc = rc = RC_INVALID_ENVIRON;
          break;
        }
        valid_command = TRUE;
        /*
         * Here is a big kludge. Because only a few commands need leading
         * spaces to be present in temp_params and all other commands barf at
         * leading spaces, we need to left truncate temp_params for most
         * commands.
         */
        if (command[i].strip_param)
          cl_param = MyStrip(cl_param, command[i].strip_param, ' ');
        /*
         * If we are currently processing the profile file as a result of
         * reprofile, ignore those commands that are invalid.
         */
        if (profile_file_executions > 1 && in_reprofile && !command[i].valid_for_reprofile) {
          rc = RC_OK;
          break;
        }
        /*
         * If there are no more files in the ring, and the command is not a
         * command to edit a new file, then ignore the command.
         */
        if (number_of_files == 0 && !command[i].edit_command) {
          rc = RC_OK;
          break;
        }
        /*
         * If running in read-only mode and the function selected is not valid
         * display an error.
         */
        if (number_of_files != 0 && ISREADONLY(CURRENT_FILE)
            && !command[i].valid_in_readonly) {
          display_error(56, (char_t *) "", FALSE);
          rc = RC_INVALID_ENVIRON;
          break;
        }
        /*
         * If operating in CUA mode, and a CUA block exists, check
         * if the block should be reset or deleted before executing
         * the command.
         */
        if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
          ResetOrDeleteCUABlock(command[i].cua_behaviour);
        }
        /*
         * Possibly reset the THIGHLIGHT area
         */
        if (CURRENT_VIEW && CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
          AdjustThighlight(command[i].thighlight_behaviour);
        }
        /*
         * Now call the function associated with the supplied command string
         * and the possibly stripped parameters.
         */
        lastrc = rc = (*command[i].function) (cl_param);
        break;
      }
    }
    /*
     * If an error occurred while executing a command above, break.
     */
    if (rc != RC_OK && rc != RC_TOF_EOF_REACHED)
      break;
    /*
     * If we found and successfully executed a command above, process the
     * next command.
     */
    if (valid_command) {
      save_last_command(cmd[j], cl_cmd);
      continue;
    }
    /*
     * If there are no more files in the ring, then ignore the command.
     */
    if (number_of_files == 0) {
      save_last_command(cmd[j], cl_cmd);
      continue;
    }
    /*
     * To get here the command was not a 'command'; check if a valid target
     */
    if (!CURRENT_VIEW->imp_macro && !CURRENT_VIEW->imp_os)
      display_parse_error = TRUE;
    else
      display_parse_error = FALSE;
    rc = execute_locate(cmd[j], display_parse_error, THE_NOT_SEARCH_SEMANTICS, &target_found);
    if (rc == RC_OK || rc == RC_TOF_EOF_REACHED || rc == RC_TARGET_NOT_FOUND || target_found) {
      lastrc = rc;
      save_last_command(cmd[j], cl_cmd);
      continue;
    }

    /*
     * If return is RC_INVALID_OPERAND, check if command is OS command...
     */
    if (cmd[j][0] == '!') {
      memmove(command_entered, cmd[j], strlen((char *) cmd[j]) + 1);
      lastrc = rc = Os(command_entered + 1);
      save_last_command(cmd[j], cl_cmd);
      continue;
    }
    /*
     * ...or if command is a macro command (as long as IMPMACRO is ON) and
     * command_only is FALSE...
     */
    if (CURRENT_VIEW->imp_macro && !command_only) {
      memmove(command_entered, cmd[j], strlen((char *) cmd[j]) + 1);
      if (CURRENT_VIEW->imp_os) {
        rc = execute_macro(command_entered, FALSE, &macrorc);
        if (rc != RC_FILE_NOT_FOUND) {
          lastrc = (rc == RC_SYSTEM_ERROR) ? rc : macrorc;
          save_last_command(cmd[j], cl_cmd);
          continue;
        }
      } else {
        rc = execute_macro(command_entered, TRUE, &macrorc);
        if (rc == RC_FILE_NOT_FOUND) {
          lastrc = rc = RC_NOT_COMMAND;
          break;
        } else {
          lastrc = (rc == RC_SYSTEM_ERROR) ? rc : macrorc;
          save_last_command(cmd[j], cl_cmd);
          continue;
        }
      }
    }
    /*
     * ...or if command is an OS command (as long as IMPOS is ON).
     */
    if (CURRENT_VIEW->imp_os) {
      error_on_screen = FALSE;
      memmove(command_entered, cmd[j], strlen((char *) cmd[j]) + 1);
      rc = Os(command_entered);
    } else {
      display_error(21, cmd[j], FALSE);
      rc = RC_NOT_COMMAND;
    }
    /*
     * If the 'command' is not a command then do not process any more.
     */
    lastrc = rc;
    if (rc == RC_NOT_COMMAND)
      break;
    save_last_command(cmd[j], cl_cmd);
  }
/* cleanup_command_line(); */
  if (saved_command) {
    Cmsg(saved_command);
    free(saved_command);
  }
  free(command_entered);
  free(cl_cmd);
  free(cl_param);

  return (rc);
}
void cleanup_command_line(void) {
  if (!curses_started || in_macro || number_of_views == 0) {
    return;
  }
  if (CURRENT_WINDOW_COMMAND != (WINDOW *) NULL) {
    wmove(CURRENT_WINDOW_COMMAND, 0, 0);
    my_wclrtoeol(CURRENT_WINDOW_COMMAND);
  }
  memset(cmd_rec, ' ', max_line_length);
  cmd_rec_len = 0;
  if (CURRENT_WINDOW_COMMAND != (WINDOW *) NULL) {
    if (CURRENT_VIEW->cmdline_col == (-1))
      wmove(CURRENT_WINDOW_COMMAND, 0, cmd_rec_len);
    else
      wmove(CURRENT_WINDOW_COMMAND, 0, CURRENT_VIEW->cmdline_col);
  }
  CURRENT_VIEW->cmdline_col = (-1);
  cmd_verify_col = 1;
  return;
}
void split_command(char_t * cmd_line, char_t * cmd, char_t * param) {
  length_t pos = 0;
  char_t *param_ptr = NULL;

  strcpy((char *) cmd, (char *) cmd_line);
  MyStrip(cmd, STRIP_LEADING, ' ');
  /*
   * Special test here for ? and = command...
   */
  if (*cmd == '?') {
    strcpy((char *) param, (char *) (cmd + 1));
    strcpy((char *) cmd, "?");
    return;
  }
  if (*cmd == '=') {
    strcpy((char *) param, (char *) (cmd + 1));
    strcpy((char *) cmd, "=");
    return;
  }
  for (param_ptr = cmd; *param_ptr != '\0'; param_ptr++) {
    if (!isalpha(*param_ptr))
      break;
  }
  if (!param_ptr) {
    strcpy((char *) param, "");
    return;
  }
  if (param_ptr == cmd || *param_ptr == '\0') {
    strcpy((char *) param, "");
    return;
  }
  pos = strzne(param_ptr, ' ');
  if (pos == (-1)) {            /* parameters are all spaces */
    strcpy((char *) param, (char *) param_ptr + 1);
    return;
  }
  strcpy((char *) param, (char *) param_ptr + (*(param_ptr) == ' ' ? 1 : 0));
  *(param_ptr) = '\0';
  return;
}
short param_split(char_t * params, char_t * word[], int words, char_t * delims, char_t param_type, char_t * strip, bool trailing_spaces_is_arg) {
#define STATE_START    0
#define STATE_WORD     1
#define STATE_DELIM    2
  register short k = 0, delims_len = strlen((char *) delims);
  char_t j = 0;
  length_t i = 0, len = 0;
  char_t *param_ptr = NULL;
  char_t *space_ptr = NULL;
  char_t state = STATE_START;
  short str_start = 0, str_end = (-1);

  /*
   * Allocate some memory to the temporary area.
   */
  if (params != NULL) {
    if (allocate_temp_space(strlen((char *) params), param_type) != RC_OK) {
      return (-1);
    }
  }
  /*
   * Based on param_type, point param_ptr to appropriate buffer.
   */
  switch (param_type) {
    case TEMP_PARAM:
      param_ptr = temp_params;
      break;
    case TEMP_SET_PARAM:
      param_ptr = temp_set_params;
      break;
    case TEMP_MACRO:
      param_ptr = temp_macros;
      break;
    case TEMP_TEMP_CMD:
      param_ptr = temp_cmd;
      break;
    case TEMP_TMP_CMD:
      param_ptr = tmp_cmd;
      break;
    default:
      return (-1);
      break;
  }
  /*
   * In case params is NULL, copy an empty string into param_ptr...
   */
  if (params == NULL)
    strcpy((char *) param_ptr, "");
  else
    memmove((char *) param_ptr, (char *) params, strlen((char *) params) + 1);

  for (i = 0; i < words; i++)
    word[i] = (char_t *) "";
  word[0] = param_ptr;
  len = strlen((char *) param_ptr);
  if (trailing_spaces_is_arg) {
    i = strzrevne(param_ptr, ' ');
    if (i != (-1)
        && (len - i) > 2) {
      space_ptr = param_ptr + (i + 2);
      param_ptr[i + 1] = '\0';
    }
  }
  j = 0;
  str_start = 0;
  for (i = 0; i < len && j < words; i++) {
    switch (state) {
      case STATE_START:
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_DELIM)
          break;
        word[j++] = param_ptr + str_start;
        if (str_end != (-1)) {
          *(param_ptr + str_end) = '\0';
        }
        state = STATE_WORD;
        break;
      case STATE_WORD:
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_DELIM) {
          str_end = i;
          str_start = str_end + 1;
          break;
        }
        break;
      case STATE_DELIM:
        state = STATE_WORD;
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_WORD) {
          word[j++] = param_ptr + str_start;
          if (str_end != (-1)) {
            *(param_ptr + str_end) = '\0';
          }
        }
        break;
    }
  }
  for (i = 0; i < words; i++) {
    if (*(strip + i))
      word[i] = MyStrip(word[i], *(strip + i), ' ');
  }
  if (space_ptr) {
    word[j] = space_ptr;
    j++;
  }
  return (j);
}
short quoted_param_split(char_t * params, char_t * word[], int words, char_t * delims, char_t param_type, char_t * strip, bool trailing_spaces_is_arg, char_t * quoted) {
/*
 * Handle args like:
 * <  "filename with spaces" arg  arg >
 * <"filename with spaces" arg arg>
 * <arg "filename with spaces">
 * <  "filename with spaces">
 * <"filename with spaces>
 * <nospacearg arg2>
 */
#define STATE_START        0
#define STATE_WORD         1
#define STATE_DELIM        2
#define STATE_QUOTED_WORD  3
  register short k = 0, delims_len = strlen((char *) delims);
  char_t j = 0, current_word;
  length_t i = 0, len = 0;
  char_t *param_ptr = NULL;
  char_t *space_ptr = NULL;
  char_t state = STATE_START;
  short str_start = 0, str_end = (-1);

  /*
   * Allocate some memory to the temporary area.
   */
  if (params != NULL) {
    if (allocate_temp_space(strlen((char *) params), param_type) != RC_OK) {
      return (-1);
    }
  }
  /*
   * Based on param_type, point param_ptr to appropriate buffer.
   */
  switch (param_type) {
    case TEMP_PARAM:
      param_ptr = temp_params;
      break;
    case TEMP_SET_PARAM:
      param_ptr = temp_set_params;
      break;
    case TEMP_MACRO:
      param_ptr = temp_macros;
      break;
    case TEMP_TEMP_CMD:
      param_ptr = temp_cmd;
      break;
    case TEMP_TMP_CMD:
      param_ptr = tmp_cmd;
      break;
    default:
      return (-1);
      break;
  }
  /*
   * In case params is NULL, copy an empty string into param_ptr...
   */
  if (params == NULL)
    strcpy((char *) param_ptr, "");
  else
    memmove((char *) param_ptr, (char *) params, strlen((char *) params) + 1);

  /*
   * Set all our return word values to empty strings
   */
  for (i = 0; i < words; i++)
    word[i] = (char_t *) "";
  word[0] = param_ptr;
  /*
   * If we are allowed a trailing arg of spaces (eg 'text  ')
   * set it now
   */
  len = strlen((char *) param_ptr);
  if (trailing_spaces_is_arg) {
    i = strzrevne(param_ptr, ' ');
    if (i != (-1)
        && (len - i) > 2) {
      space_ptr = param_ptr + (i + 2);
      param_ptr[i + 1] = '\0';
    }
  }

  j = 0;
  current_word = 0;
  str_start = 0;
  for (i = 0; i < len && j < words; i++) {
    switch (state) {
      case STATE_START:
        /*
         * Is the current character a delimiter?
         */
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_DELIM)
          break;
        /*
         * We have found the first character, not in delims
         * this is the start of the word.
         */
        if (quoted[j] == *(param_ptr + i)) {
          current_word = j;
          word[j++] = param_ptr + str_start;
          state = STATE_QUOTED_WORD;
        } else {
          current_word = j;
          word[j++] = param_ptr + str_start;
          state = STATE_WORD;
        }
        if (str_end != (-1)) {
          *(param_ptr + str_end) = '\0';
        }
        break;
      case STATE_QUOTED_WORD:
        if (quoted[current_word] == *(param_ptr + i)) {
          /*
           * We have found our terminator for our quoted word
           */
          state = STATE_WORD;
          str_end = i;
          str_start = i + 1;
          break;
        }
        break;
      case STATE_WORD:
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_DELIM) {
          str_end = i;
          str_start = str_end + 1;
          break;
        }
        break;
      case STATE_DELIM:
        state = STATE_START;
        for (k = 0; k < delims_len; k++) {
          if (*(param_ptr + i) == *(delims + k)) {
            state = STATE_DELIM;
            break;
          }
        }
        if (state == STATE_START) {
          current_word = j;
          word[j++] = param_ptr + str_start;
          if (str_end != (-1)) {
            *(param_ptr + str_end) = '\0';
          }
          if (*(param_ptr + i) == quoted[current_word]) {
            state = STATE_QUOTED_WORD;
          } else {
            state = STATE_WORD;
          }
        }
        break;
    }
  }
  /*
   * Strip spaces from each word as specified unless
   * we found a quoted value
   */
  for (i = 0; i < words; i++) {
    if (quoted[i]) {
      word[i] = MyStrip(word[i], STRIP_BOTH, quoted[i]);
    } else if (*(strip + i)) {
      word[i] = MyStrip(word[i], *(strip + i), ' ');
    }
  }
  /*
   * If we found a trailing space argument, set the last
   * word to the start of the trailing space
   */
  if (space_ptr) {
    word[j] = space_ptr;
    j++;
  }
  return (j);
}
short command_split(char_t * params, char_t * word[], int words, char_t * delims, char_t * buffer) {
  register short k, delims_len = strlen((char *) delims);
  length_t i, len;
  char_t j = 0;
  bool end_of_string = FALSE, end_of_word = FALSE;

  /*
   * In case params is NULL, copy an empty string into buffer...
   */
  if (params == NULL)
    strcpy((char *) buffer, "");
  else
    strcpy((char *) buffer, (char *) params);

  for (i = 0; i < words; i++) {
    word[i] = (char_t *) "";
  }
  j = 0;
  end_of_string = TRUE;
  len = strlen((char *) buffer);
  for (i = 0; i < len && j < words; i++) {
    end_of_word = FALSE;
    for (k = 0; k < delims_len; k++) {
      if (*(buffer + i) == *(delims + k))
        end_of_word = TRUE;
    }
    if (end_of_word) {
      *(buffer + i) = '\0';
      end_of_string = TRUE;
    } else {
      if (end_of_string) {
        word[j++] = buffer + i;
        end_of_string = FALSE;
      }
    }
  }
  return (j);
}
line_t get_true_line(bool respect_compat) {
  line_t true_line = 0L;

  /*
   * Determine 'true_line'.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || (compatible_feel == COMPAT_XEDIT && respect_compat)
      || batch_only)
    true_line = CURRENT_VIEW->current_line;
  else
    true_line = CURRENT_VIEW->focus_line;
  return (true_line);
}
length_t get_true_column(bool respect_compat) {
  length_t true_column = 0;
  short x, y;

  /*
   * Determine 'true_column'.
   */
  if (CURRENT_VIEW->current_window == WINDOW_COMMAND || (compatible_feel == COMPAT_XEDIT && respect_compat)
      || batch_only)
    true_column = CURRENT_VIEW->current_column;
  else {
    getyx(CURRENT_WINDOW_FILEAREA, y, x);
    true_column = CURRENT_VIEW->verify_col + x;
  }
  return (true_column);
}
char_t next_char(LINE * curr, long *off, length_t end_col) {
  if (*(off) < (long) min(curr->length, end_col)) {
    (*(off))++;
    return (*(curr->line + ((*(off)) - 1L)));
  }
  *(off) = (-1L);
  return (0);
}

short add_define(DEFINE ** first, DEFINE ** last, int key_value, char_t * commands, bool instore, char_t * synonym, char_t linend)
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
/*   commands: commands and parameters                                 */
{
  register short j = 0;
  short cmd_nr = 0;
  char_t *word[MAX_COMMANDS + 1];
  unsigned short num_commands = 0;
  char_t command_delim[2];
  short rc = RC_OK;
  char_t *command_entered = NULL, *cl_cmd = NULL, *cl_param = NULL;

  /*
   * If the commands argument is empty, delete the definition of the key
   * definitions for the key, so just return.
   */
  if (strcmp((char *) commands, "") == 0) {
    remove_define(first, last, key_value, synonym);
    return (RC_OK);
  }
  if (instore) {
    if (rexx_support) {
      remove_define(first, last, key_value, synonym);
      rc = append_define(first, last, key_value, (-1), commands, NULL, 0, synonym, linend);
      return (rc);
    } else {
      display_error(58, (char_t *) "instore macros", FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  /*
   * To have reached here we are dealing with "plain" key definitions,
   * rather than instore macros...
   * Copy the incoming commands, so we can play with it.
   */
  if ((command_entered = (char_t *) my_strdup(commands)) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Allocate some space to cl_cmd and cl_param for the a command when
   * it is split into a command and its parameters.
   */
  if ((cl_cmd = (char_t *) malloc((strlen((char *) commands) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  if ((cl_param = (char_t *) malloc((strlen((char *) commands) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  if (synonym) {
    /*
     * If the is called from a SET SYNONYM call, then use the passed value
     * for LINEND...
     */
    if (linend) {
      command_delim[0] = linend;
      command_delim[1] = '\0';
      num_commands = command_split(commands, word, MAX_COMMANDS, command_delim, command_entered);
    } else {
      if (CURRENT_VIEW->linend_status) {
        command_delim[0] = CURRENT_VIEW->linend_value;
        command_delim[0] = '\0';
        num_commands = command_split(commands, word, MAX_COMMANDS, command_delim, command_entered);
      } else {
        word[0] = command_entered;
        num_commands = 1;
      }
    }
  } else {
    /*
     * If [SET] LINENd is set to ON, split the args up into a number of
     * individual commands.
     */
    if (CURRENT_VIEW) {
      if (CURRENT_VIEW->linend_status) {
        command_delim[0] = CURRENT_VIEW->linend_value;
        command_delim[1] = '\0';
        num_commands = command_split(commands, word, MAX_COMMANDS, command_delim, command_entered);
      } else {
        word[0] = command_entered;
        num_commands = 1;
      }
    } else {
      command_delim[0] = '#';
      command_delim[1] = '\0';
      num_commands = command_split(commands, word, MAX_COMMANDS, command_delim, command_entered);
    }
  }
  /*
   * For each command entered, split it up into command and params, and
   * validate that each command is valid.  The cmd_nr is discarded here.
   */
  for (j = 0; j < num_commands; j++) {
    split_command(word[j], cl_cmd, cl_param);
    cl_cmd = MyStrip(cl_cmd, STRIP_BOTH, ' ');
    if ((cmd_nr = find_command(cl_cmd, FALSE)) == (-1)) {
      display_error(21, cl_cmd, FALSE);
      rc = RC_INVALID_OPERAND;
      break;
    }
  }
  /*
   * Now we know each command is valid, we can remove any prior
   * definition and assign the new one.
   */
  if (rc == RC_OK) {
    remove_define(first, last, key_value, synonym);
    for (j = 0; j < num_commands; j++) {
      split_command(word[j], cl_cmd, cl_param);
      cl_cmd = MyStrip(cl_cmd, STRIP_BOTH, ' ');
      if ((cmd_nr = find_command(cl_cmd, FALSE)) == (-1)) {
        display_error(21, cl_cmd, FALSE);       /* this should not be reached */
        rc = RC_INVALID_OPERAND;
        break;
      }
      rc = append_define(first, last, key_value, cmd_nr, cl_param, NULL, 0, synonym, linend);
      if (rc != RC_OK)
        break;
    }
  }
  free(command_entered);
  free(cl_cmd);
  free(cl_param);
  return (rc);
}

short remove_define(DEFINE ** first, DEFINE ** last, int key_value, char_t * synonym)
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
{
  DEFINE *curr = NULL;

  /*
   * Find all items in the linked list for the key_value and remove them
   * from the list.
   */
  curr = *first;
  if (synonym) {
    while (curr != NULL) {
      if (strcasecmp((char *) curr->synonym, (char *) synonym) == 0) {
        if (curr->def_params != NULL)
          free(curr->def_params);
        if (curr->pcode != NULL)
          free(curr->pcode);
        if (curr->synonym != NULL)
          free(curr->synonym);
        curr = dll_del(first, last, curr, DIRECTION_FORWARD);
      } else
        curr = curr->next;
    }
  } else {
    while (curr != NULL) {
      if (curr->def_funkey == key_value) {
        if (curr->def_params != NULL)
          free(curr->def_params);
        if (curr->pcode != NULL)
          free(curr->pcode);
        curr = dll_del(first, last, curr, DIRECTION_FORWARD);
      } else
        curr = curr->next;
    }
  }

  return (RC_OK);
}

short append_define(DEFINE ** first, DEFINE ** last, int key_value, short cmd, char_t * prm, char_t * pcode, int pcode_len, char_t * synonym, char_t linend)
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
{
  DEFINE *curr = NULL;

  /*
   * Add the new key definition to the end of the linked list...
   */
  curr = dll_add(*first, *last, sizeof(DEFINE));
  if (curr == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  curr->def_params = (char_t *) malloc((strlen((char *) prm) + 1) * sizeof(char_t));
  if (curr->def_params == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  strcpy((char *) curr->def_params, (char *) prm);

  if (synonym) {
    curr->synonym = (char_t *) malloc((strlen((char *) synonym) + 1) * sizeof(char_t));
    if (curr->synonym == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) curr->synonym, (char *) synonym);
  } else
    curr->synonym = NULL;

  curr->def_funkey = key_value;
  curr->def_command = cmd;
  curr->linend = linend;
  if (pcode && pcode_len) {
    curr->pcode = (char_t *) malloc(pcode_len * sizeof(char_t));
    if (curr->pcode == NULL) {
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    memcpy(curr->pcode, pcode, pcode_len);
    curr->pcode_len = pcode_len;
  }
  *last = curr;
  if (*first == NULL)
    *first = *last;
  return (RC_OK);
}

short find_command(char_t * cmd, bool search_for_target)
/*   Function: determine if the string supplied is a valid abbrev for  */
/*             a command.                                              */
/* Parameters:                                                         */
/*        cmd:               the string to be checked                  */
/*        search_for_target: determine if command is a valid target    */
{
  register short i = 0;
  short rc = RC_OK;
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK | TARGET_ALL;

  for (i = 0; command[i].text != NULL; i++) {
    if (equal(command[i].text, cmd, (command[i].min_len == 0) ? strlen((char *) command[i].text) : command[i].min_len)
        && !command[i].sos_command) {
      return (i);
    }
  }
  /*
   * To get here the command was not a 'command'. If we don't want to
   * search for targets, exit with (-1).
   */
  if (!search_for_target) {
    return (-1);
  }
  /*
   * Find if it is a valid target...
   */
  initialise_target(&target);
  rc = validate_target(cmd, &target, target_type, get_true_line(TRUE), TRUE, TRUE);
  if (rc != RC_OK && rc != RC_TARGET_NOT_FOUND) {
    free_target(&target);
    return (-1);
  }
  free_target(&target);
  /*
   * If a valid target, find 'LOCATE' command and return the index.
   */
  strcpy((char *) temp_params, (char *) cmd);
  for (i = 0; command[i].text != NULL; i++) {
    if (strcmp((char *) command[i].text, "locate") == 0)
      break;
  }
  return (i);
}
void init_command(void) {
  register short i = 0;

  last_command_for_reexecute = NULL;
  last_command_for_reexecute_len = 0;
  last_command_for_repeat_in_macro = NULL;
  last_command_for_repeat_in_macro_len = 0;
  last_command_for_repeat = NULL;
  last_command_for_repeat_len = 0;
  for (i = 0; i < MAX_SAVED_COMMANDS; i++) {
    cmd_history[i] = NULL;
    cmd_history_len[i] = 0;
  }
  return;
}
void add_command(char_t * new_cmd) {
  int len_cmd;

  /*
   * Do not save commands if the commands are issued from a macro.
   */
  if (in_macro) {
    return;
  }
  /*
   * If the command to be added is the same as the current command or if
   * the command line is empty or if the command is "=" or "?", return
   * without adding command to array.
   */
  if (!valid_command_to_save(new_cmd)) {
    return;
  }
  len_cmd = strlen((char *) new_cmd);
  offset_cmd = 0;
  /*
   * if the incoming command is the same as the current one, don't save it again
   */
  if (cmd_history[current_cmd]
      && (strcmp((char *) new_cmd, (char *) cmd_history[current_cmd]) == 0)) {
    return;
  }
  if (number_cmds == MAX_SAVED_COMMANDS) {
    if (last_cmd == MAX_SAVED_COMMANDS - 1)
      current_cmd = last_cmd = 0;
    else
      current_cmd = ++last_cmd;
  } else
    current_cmd = ++last_cmd;
  /*
   * Allocate/reallocate memory for current command if the new
   * command length is > current command in array
   */
  if (len_cmd > cmd_history_len[current_cmd]) {
    cmd_history[current_cmd] = (char_t *) realloc(cmd_history[current_cmd], len_cmd + 1);
    if (cmd_history[current_cmd] == NULL) {
      cmd_history_len[current_cmd] = 0;
      return;
    }
  }
  strcpy((char *) cmd_history[current_cmd], (char *) new_cmd);
  number_cmds++;
  if (number_cmds > MAX_SAVED_COMMANDS)
    number_cmds = MAX_SAVED_COMMANDS;
  return;
}
char_t *get_next_command(short direction, int num) {
  char_t *ret_cmd = NULL;

  if (number_cmds == 0) {
    return ((char_t *) NULL);
  }
  while (num--) {
    switch (direction) {
      case DIRECTION_BACKWARD:
        if (current_cmd + 1 == number_cmds) {
          current_cmd = 0;
          ret_cmd = cmd_history[current_cmd];
        } else
          ret_cmd = cmd_history[++current_cmd];
        break;
      case DIRECTION_FORWARD:
        if (current_cmd + offset_cmd < 0) {
          current_cmd = number_cmds - 1;
          ret_cmd = cmd_history[current_cmd];
        } else {
          current_cmd = current_cmd + offset_cmd;
          ret_cmd = cmd_history[current_cmd];
        }
        offset_cmd = (-1);
        break;
      case DIRECTION_NONE:
        ret_cmd = cmd_history[current_cmd];
        break;
    }
  }
  return (ret_cmd);
}
bool valid_command_to_save(char_t * save_cmd) {
  /*
   * If the command to be added is empty or is "=" or starts with "?",
   * return FALSE, otherwise return TRUE.
   */
  if (save_cmd == NULL || strcmp((char *) save_cmd, "") == 0 || strcmp((char *) save_cmd, "=") == 0 || save_cmd[0] == '?') {
    return (FALSE);
  }
  return (TRUE);
}
static void save_last_command(char_t * last_cmd, char_t * cmnd) {
  int last_cmd_len;

  /*
   * If the command to be added is the same as the current command or if
   * the command line is empty or if the command is "=" or "?", return
   * without adding command to array.
   * Save commands for repeat but NOT for execute if the command is
   * run from a macro...
   */
  if (valid_command_to_save(last_cmd)) {
    last_cmd_len = strlen((char *) last_cmd);
    if (!in_macro) {
      if (last_cmd_len > last_command_for_reexecute_len) {
        last_command_for_reexecute = (char_t *) realloc(last_command_for_reexecute, last_cmd_len + 1);
      }
      if (last_command_for_reexecute) {
        strcpy((char *) last_command_for_reexecute, (char *) last_cmd);
        last_command_for_reexecute_len = last_cmd_len;
      } else
        last_command_for_reexecute_len = 0;
    }
    if (!equal((char_t *) "repeat", cmnd, 4)
        && save_for_repeat) {
      if (in_macro) {
        if (last_cmd_len > last_command_for_repeat_in_macro_len) {
          last_command_for_repeat_in_macro = (char_t *) realloc(last_command_for_repeat_in_macro, last_cmd_len + 1);
        }
        if (last_command_for_repeat_in_macro) {
          strcpy((char *) last_command_for_repeat_in_macro, (char *) last_cmd);
        } else
          last_command_for_repeat_in_macro_len = 0;
      } else {
        if (last_cmd_len > last_command_for_repeat_len) {
          last_command_for_repeat = (char_t *) realloc(last_command_for_repeat, last_cmd_len + 1);
        }
        if (last_command_for_repeat) {
          strcpy((char *) last_command_for_repeat, (char *) last_cmd);
        } else
          last_command_for_repeat_len = 0;
      }
    }
  }
  return;
}
bool is_tab_col(length_t x) {
  register short i = 0;
  bool rc = FALSE;

  for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
    if (CURRENT_VIEW->tabs[i] == x) {
      rc = TRUE;
      break;
    }
  }
  return (rc);
}
length_t find_next_tab_col(length_t x) {
  register short i = 0;
  length_t next_tab_col = 0;

  for (i = 0; i < CURRENT_VIEW->numtabs; i++) {
    if (CURRENT_VIEW->tabs[i] > x) {
      next_tab_col = CURRENT_VIEW->tabs[i];
      break;
    }
  }
  return (next_tab_col);
}
length_t find_prev_tab_col(length_t x) {
  register short i = 0;
  length_t next_tab_col = 0;

  for (i = CURRENT_VIEW->numtabs - 1; i > -1; i--) {
    if (CURRENT_VIEW->tabs[i] < x) {
      next_tab_col = CURRENT_VIEW->tabs[i];
      break;
    }
  }
  return (next_tab_col);
}
short tabs_convert(LINE * curr, bool expand_tabs, bool use_tabs, bool add_to_recovery) {
#define STATE_NORMAL 0
#define STATE_TAB    1
  length_t i, j;
  bool expanded = FALSE;
  bool tabs_exhausted = FALSE;
  int state = STATE_NORMAL;
  length_t tabcol = 0;

  /*
   * If we are expanding tabs to spaces, do the following...
   */
  if (expand_tabs) {
    for (i = 0, j = 0; i < curr->length; i++) {
      if (curr->line[i] == '\t') {
        if (use_tabs) {
          if (tabs_exhausted) {
            trec[j++] = ' ';
            if (j >= max_line_length)
              break;
          } else {
            tabcol = find_next_tab_col(j + 1);
            if (tabcol == 0)
              tabs_exhausted = TRUE;
            else {
              tabcol--;
              do {
                trec[j++] = ' ';
                if (j >= max_line_length)
                  break;
              }
              while (j < tabcol);
            }
          }
        } else {
          do {
            trec[j++] = ' ';
            if (j >= max_line_length)
              break;
          }
          while ((j % TABI_Nx) != 0);
        }
        expanded = TRUE;
      } else {
        trec[j++] = curr->line[i];
        if (j >= max_line_length)
          break;
      }
    }
    /*
     * If we expanded tabs, we need to reallocate memory for the line.
     */
    if (expanded) {
      if (add_to_recovery)
        add_to_recovery_list(curr->line, curr->length);
      curr->line = (char_t *) realloc((void *) curr->line, (j + 1) * sizeof(char_t));
      if (curr->line == (char_t *) NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      /*
       * Copy the contents of rec into the line.
       */
      memcpy(curr->line, trec, j);
      curr->length = j;
      *(curr->line + j) = '\0';
    }
  } else {
    for (i = (curr->length) - 1, j = 0; i > (-1); i--) {
      switch (state) {
        case STATE_NORMAL:
          trec[j++] = *(curr->line + i);
          if (is_tab_col(i + 1)
              && i != 0) {
            if (*(curr->line + (i - 1)) == ' ') {
              trec[j++] = '\t';
              state = STATE_TAB;
              expanded = TRUE;
            }
          }
          break;
        case STATE_TAB:
          if (is_tab_col(i + 1)
              && i != 0) {
            if (*(curr->line + i) == ' ') {
              if (*(curr->line + (i - 1)) == ' ')
                trec[j++] = '\t';
            } else {
              trec[j++] = *(curr->line + i);
              state = STATE_NORMAL;
            }
          } else {
            if (*(curr->line + i) != ' ') {
              trec[j++] = *(curr->line + i);
              state = STATE_NORMAL;
            }
          }
          break;
      }
    }
    if (expanded) {
      trec[j] = '\0';
      curr->length = j;
      for (i = 0, j--; j > (-1); i++, j--)
        *(curr->line + i) = trec[j];
      *(curr->line + curr->length) = '\0';
    }
  }
  return ((expanded) ? RC_FILE_CHANGED : RC_OK);
}

/* -----------------------------------------------------------------
 * This code borrowed heavily from Regina!!
 * Input is a hex string, which is converted to a char string
 * representing the same information and returned.
 *
 * We have to concider the optional grouping of hex digits by spaces at
 * byte boundaries, and the possibility of having to pad first group
 * with a zero.
 *
 * There is one performance problem with this. If the hex string is
 * normalized and long, we have to loop through the string twice, while
 * once would suffice. To determine whether or not to pad first group
 * with a zero, all of first group must be scanned, which is identical
 * to the whole string if it is normalized.
 */
static int pack_hex(char *string, char *out) {
  char *ptr = NULL;             /* current digit in input hex string */
  char *end_ptr = NULL;         /* ptr to end+1 in input hex string */
  char *res_ptr = NULL;         /* ptr to current char in output string */
  int byte_boundary = 0;        /* boolean, are we at at byte bounary? */
  int count;                    /* used to count positions */

  res_ptr = out;
  /*
   * Initiate pointers to current char in intput string, and to
   * end+1 in input string.
   */
  ptr = string;
  end_ptr = string + strlen(string);
  /*
   * Explicitly check for space at start or end. Illegal space within
   * the hex string is checked for during the loop.
   */
  if ((ptr < end_ptr) && ((isspace(*ptr)) || (isspace(*(end_ptr - 1))))) {
    return ((-1));
  }
  /*
   * Find the number of hex digits in the first group of hex digits.
   * Let the variable 'byte_boundary' be a boolean, indicating if
   * current char might be a byte boundary. I.e if byte_boundary is
   * set, spaces are legal.
   *
   * Also, set the first byte in the output string. That is not
   * necessary if the first group of hex digits has an even number of
   * digits, but it is cheaper to do it always that check for it.
   */
  for (; (ptr < end_ptr) && (isxdigit(*ptr)); ptr++);
  byte_boundary = !((ptr - string) % 2);
  /*
   * Set output to null string
   */
  *res_ptr = 0x00;
  /*
   * Loop through the elements of the input string. Skip over spaces.
   * Stuff hex digits into the output string, and report error
   * for any other type of data.
   */
  for (count = 1, ptr = string; ptr < end_ptr; ptr++, count++) {
    if (isspace(*ptr)) {
      /*
       * Just make sure that this space occurs at a byte boundary,
       * except from that, ignore it.
       */
      if (!byte_boundary) {
        return ((-1));
      }
    } else if (isxdigit(*ptr)) {
      /*
       * Stuff it into the output array, either as upper or lower
       * part of a byte, depending on the value of 'byte_boundary'.
       * Then toggle the value of 'byte_boundary'.
       */
      if (byte_boundary)
        *res_ptr = (char) (HEXVAL(*ptr) << 4);
      else {
        /* Damn'ed MSVC: */
        *res_ptr = (char) (*res_ptr + (char) (HEXVAL(*ptr)));
        res_ptr++;
      }
      byte_boundary = !byte_boundary;
    } else {
      return ((-1));
    }
  }
  /*
   * Set the length and do 'redundant' check for problems. In
   * particular, check 'byte_boundary' to verify that the last group
   * of hex digits ended at a byte boundary; report error if not.
   */
  if (!byte_boundary) {
    return ((-1));
  }

  return res_ptr - out;
}
short convert_hex_strings(char_t * str) {
  length_t i = 0;
  char_t *p = NULL;
  bool dec_char = FALSE;
  char_t *temp_str;
  char_t *ptr, *end_ptr, *first_non_blank = NULL, *last_non_blank = NULL;
  short num = 0;
  int str_len;

  /*
   * If the string is less than 4 chars; d'3', then it can't be a hex/dec
   * value.
   */
  str_len = strlen((char *) str);
  if (str_len < 4) {
    return (str_len);
  }
  /*
   * Allocate some termporary space; it MUST be <= input string
   */
  temp_str = (char_t *) alloca(str_len + 1);
  if (temp_str == NULL) {
    return (-2);
  }
  ptr = str;
  end_ptr = str + strlen((char *) str) - 1;
  /*
   * Determine the first and last non-blank characters...
   */
  for (ptr = str; ptr < end_ptr; ptr++) {
    if (*ptr != (char_t) ' ') {
      first_non_blank = ptr;
      break;
    }
  }
  for (ptr = str; ptr < end_ptr; end_ptr--) {
    if (*end_ptr != (char_t) ' ') {
      last_non_blank = end_ptr;
      break;
    }
  }
  /*
   * If the number of non-blank characters is less than 4 it can't be
   * a hex/dec value.
   */
  if (last_non_blank - first_non_blank < 3) {
    return (str_len);
  }
  /*
   * Check if the first non-blank character is d,D,x or X and it has '
   * as its 2nd non-blank character and as its last non-blank character
   * If not, then return with string unchanged.
   */
  if (*last_non_blank != (char_t) '\'' || *(first_non_blank + 1) != (char_t) '\'') {
    return (str_len);
  }
  temp_str[0] = toupper(*first_non_blank);
  if (temp_str[0] == (char_t) 'D')
    dec_char = TRUE;
  else if (temp_str[0] == (char_t) 'X')
    dec_char = FALSE;
  else {
    return (str_len);
  }
  /*
   * If we got here we can validate (and change) the contents of the string.
   */
  *(last_non_blank) = (char_t) '\0';
  if (dec_char == FALSE) {
    i = pack_hex((char *) (first_non_blank + 2), (char *) temp_str);
    if (i != (-1))
      memcpy(str, temp_str, i);
  } else {
    p = (char_t *) strtok((char *) first_non_blank + 2, " ");
    while (p != NULL) {
      if (equal((char_t *) "000000", p, 1))
        temp_str[i++] = (char_t) 0;
      else {
        num = atoi((char *) p);
        if (num < 1 || num > 255) {
          return ((-1));
        }
        temp_str[i++] = (char_t) num;
      }
      p = (char_t *) strtok(NULL, " ");
    }
    memcpy(str, temp_str, i);
  }
  return (i);
}
short marked_block(bool in_current_view) {
  if (batch_only) {             /* block commands invalid in batch */
    display_error(24, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (MARK_VIEW == (VIEW_DETAILS *) NULL) {     /* no marked block */
    display_error(44, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  if (MARK_VIEW != CURRENT_VIEW /* marked block not in current view */
      && in_current_view) {
    display_error(45, (char_t *) "", FALSE);
    return (RC_INVALID_ENVIRON);
  }
  return (RC_OK);
}
short suspend_curses(void) {

  endwin();

  return (RC_OK);
}
short resume_curses(void) {
  reset_prog_mode();

  return (RC_OK);
}
short restore_THE(void) {
  unsigned short y = 0, x = 0;

  /*
   * If curses hasn't started, no point in doing anything...
   */
  if (!curses_started) {
    return (RC_OK);
  }
  getyx(CURRENT_WINDOW, y, x);

  if (display_screens > 1) {
    touch_screen((char_t) other_screen);
    refresh_screen((char_t) other_screen);
    if (!horizontal) {
      touchwin(divider);
      wnoutrefresh(divider);
    }
  }
  touch_screen(current_screen);
  if (statarea != (WINDOW *) NULL)
    touchwin(statarea);
  if (filetabs != (WINDOW *) NULL)
    touchwin(filetabs);
  if (max_slk_labels) {
    slk_touch();
    slk_noutrefresh();
  }
  wmove(CURRENT_WINDOW, y, x);
  return (RC_OK);
}
short execute_set_sos_command(bool set_command, char_t * params) {
#define SETSOS_PARAMS  2
  char_t *word[SETSOS_PARAMS + 1];
  char_t strip[SETSOS_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK, command_index = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_NONE;
  num_params = param_split(params, word, SETSOS_PARAMS, WORD_DELIMS, TEMP_SET_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(1, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if ((command_index = valid_command_type(set_command, word[0])) == RC_NOT_COMMAND) {
    display_error((unsigned short) (set_command ? 42 : 41), word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Found the matching SET/SOS. Save it for extended error reporting
   */
  if (ERRORFORMATx == 'E')
    last_command_index = command_index;
  /*
   * If the SOS command is being executed while in READV CMDLINE, only
   * execute those commands that are allowed...
   */
  if (in_readv) {
    if (command[command_index].valid_readv)
      rc = (*command[command_index].function) (word[1]);
  } else {
    /*
     * If operating in CUA mode, and a CUA block exists, check
     * if the block should be reset or deleted before executing
     * the command.
     */
    if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
      ResetOrDeleteCUABlock(command[command_index].cua_behaviour);
    }
    /*
     * Possibly reset the THIGHLIGHT area
     */
    if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
      AdjustThighlight(command[command_index].thighlight_behaviour);
    }
    rc = (*command[command_index].function) (word[1]);
  }
  return (rc);
}
short valid_command_type(bool set_command, char_t * cmd_line) {
  register short i;
  short rc = RC_NOT_COMMAND;

  for (i = 0; command[i].text != NULL; i++) {
    /*
     * If no command text, continue.
     */
    if (strcmp((char *) command[i].text, "") == 0)
      continue;
    /*
     * Check that the supplied command matches the command for the length
     * of the command and that the length is at least as long as the
     * necessary significance.
     */
    if (equal(command[i].text, cmd_line, command[i].min_len)
        && command[i].min_len != 0) {
      if (set_command && command[i].set_command) {
        rc = i;
        break;
      }
      if (!set_command && command[i].sos_command) {
        rc = i;
        break;
      }
    }
  }
  return (rc);
}
short allocate_temp_space(length_t length, char_t param_type) {
  char_t *temp_ptr = NULL;
  length_t *temp_length = NULL;

  /*
   * Based on param_type, point param_ptr to appropriate buffer.
   */
  switch (param_type) {
    case TEMP_PARAM:
      temp_ptr = temp_params;
      temp_length = &length_temp_params;
      break;
    case TEMP_SET_PARAM:
      temp_ptr = temp_set_params;
      temp_length = &length_temp_set_params;
      break;
    case TEMP_MACRO:
      temp_ptr = temp_macros;
      temp_length = &length_temp_macros;
      break;
    case TEMP_TMP_CMD:
      temp_ptr = tmp_cmd;
      temp_length = &length_tmp_cmd;
      break;
    case TEMP_TEMP_CMD:
      temp_ptr = temp_cmd;
      temp_length = &length_temp_cmd;
      break;
    default:
      return (-1);
      break;
  }
  if (*temp_length >= length) {
    return (RC_OK);
  }
  if (temp_ptr == NULL)
    temp_ptr = (char_t *) malloc(sizeof(char_t) * (length + 1));
  else
    temp_ptr = (char_t *) realloc(temp_ptr, sizeof(char_t) * (length + 1));
  if (temp_ptr == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Based on param_type, point param_ptr to appropriate buffer.
   */
  switch (param_type) {
    case TEMP_PARAM:
      temp_params = temp_ptr;
      break;
    case TEMP_SET_PARAM:
      temp_set_params = temp_ptr;
      break;
    case TEMP_MACRO:
      temp_macros = temp_ptr;
      break;
    case TEMP_TMP_CMD:
      tmp_cmd = temp_ptr;
      break;
    case TEMP_TEMP_CMD:
      temp_cmd = temp_ptr;
      break;
    default:
      return (-1);
      break;
  }
  *temp_length = length;
  return (RC_OK);
}
void free_temp_space(char_t param_type) {
  char_t *temp_ptr = NULL;
  length_t *temp_length = 0;

  /*
   * Based on param_type, point param_ptr to appropriate buffer.
   */
  switch (param_type) {
    case TEMP_PARAM:
      temp_ptr = temp_params;
      temp_params = NULL;
      temp_length = &length_temp_params;
      break;
    case TEMP_SET_PARAM:
      temp_ptr = temp_set_params;
      temp_params = NULL;
      temp_length = &length_temp_set_params;
      break;
    case TEMP_MACRO:
      temp_ptr = temp_macros;
      temp_macros = NULL;
      temp_length = &length_temp_macros;
      break;
    case TEMP_TMP_CMD:
      temp_ptr = tmp_cmd;
      tmp_cmd = NULL;
      temp_length = &length_tmp_cmd;
      break;
    case TEMP_TEMP_CMD:
      temp_ptr = temp_cmd;
      temp_cmd = NULL;
      temp_length = &length_temp_cmd;
      break;
    default:
      return;
      break;
  }
  free(temp_ptr);
  *temp_length = 0;
  return;
}

char_t calculate_actual_row(short base, short off, row_t rows, bool force_in_view) {
  short row = 0;

  switch (base) {
    case POSITION_TOP:
      row = off;
      break;
    case POSITION_MIDDLE:
      row = (rows / 2) + off;
      break;
    case POSITION_BOTTOM:
      row = rows + off + 1;
      break;
  }
  /*
   * If the calculated row is outside the screen size, default to middle.
   */
  if ((row < 0 || row > rows)
      && force_in_view)
    row = rows / 2;
  return ((char_t) row - 1);
}

short get_valid_macro_file_name(char * inmacroname, char * filename, char * macro_ext, short *errnum) {
  register short i = 0;
  char_t delims[3];
  bool file_found = FALSE;
  char_t macroname[MAX_FILE_NAME + 1];
  int len_macroname = strlen((char *) inmacroname);
  int len_macro_suffix = strlen((char *) macro_ext);
  bool append_suffix = TRUE;

  /*
   * Create the full name of the macro file by prepending the default
   * macropath provided the filename does not already contain a path.
   */
  strcpy((char *) macroname, (char *) inmacroname);
  strrmdup(strtrans(macroname, OSLASH, ISLASH), ISLASH, TRUE);
  strcpy((char *) delims, (char *) ISTR_SLASH);
  if (strpbrk((char *) macroname, (char *) delims) == NULL && *(macroname) != '~') {
    /*
     * The supplied macro file name does not contain a path...so for each
     * directory in the_macro_path, try to find the supplied file in that
     * directory.
     */
    if (len_macroname > len_macro_suffix) {
      if (strcmp((char *) macroname + (len_macroname - len_macro_suffix), (char *) macro_ext) == 0)
        append_suffix = FALSE;
      else
        append_suffix = TRUE;
    }
    file_found = FALSE;
    for (i = 0; i < max_macro_dirs; i++) {
      strcpy((char *) filename, (char *) the_macro_dir[i]);
      if (strlen((char *) filename) == 0)
        continue;
      if (*(filename + strlen((char *) filename) - 1) != ISLASH)
        strcat((char *) filename, (char *) ISTR_SLASH);
      strcat((char *) filename, (char *) macroname);      /* append the file name */
      if (append_suffix)
        strcat((char *) filename, (char *) macro_ext);    /* append default suffix */
      if (file_exists(filename) == THE_FILE_EXISTS) {   /* check if file exists... */
        file_found = TRUE;
        break;
      }
    }
    if (!file_found) {
      strcpy((char *) filename, (char *) macroname);
      if (append_suffix)
        strcat((char *) filename, (char *) macro_ext);
      *errnum = 11;
      return (RC_FILE_NOT_FOUND);
    }
  } else {                      /* file contains a path specifier */
  /*
   * The supplied macro file name does contain a path...so just check to
   * ensure that the file exists.
   */ if (splitpath(macroname) != RC_OK) {
    *errnum = 9;
    return (RC_FILE_NOT_FOUND);
  }
  strcpy((char *) filename, (char *) sp_path);
  strcat((char *) filename, (char *) sp_fname);
  if (file_exists(filename) != THE_FILE_EXISTS || strcmp((char *) sp_fname, "") == 0) {
    *errnum = 9;
    return (RC_FILE_NOT_FOUND);
  }
  }
  /*
   * If the file is not readable, error.
   */
  if (!file_readable(filename)) {
    *errnum = 8;
    return (RC_ACCESS_DENIED);
  }
  *errnum = 0;
  return (RC_OK);
}
bool define_command(char_t * cmd_line) {
  register short i = 0;
  char_t buf[7];

  /*
   * First check if the command is a synonym, and use the real name to
   * search the command array.
   */
  memset(buf, '\0', 7);
  memcpy(buf, cmd_line, min(6, strlen((char *) cmd_line)));
  for (i = 0; i < 7; i++) {
    if (buf[i] == ' ')
      buf[i] = '\0';
  }
  if ((i = find_command(buf, FALSE)) == (-1)) {
    return (FALSE);
  }
  if (strcmp("define", (char *) command[i].text) == 0) {
    return (TRUE);
  }
  return (FALSE);
}
int find_key_name(char_t * keyname) {
  register int i = 0;
  int key = (-1);

  for (i = 0; key_table[i].mnemonic != NULL; i++) {
    if (strcasecmp(keyname, key_table[i].mnemonic) == 0) {
      key = key_table[i].key_value;
      break;
    }
  }
  return (key);
}
int readv_cmdline(char_t * initial, WINDOW * dw, int start_col) {
  int key = 0;
  short rc = RC_OK;
  char_t buf[3];

  if (CURRENT_WINDOW_COMMAND == (WINDOW *) NULL) {
    display_error(86, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  buf[1] = '\0';
  Cmsg(initial);
  if (start_col == -1)
    THEcursor_cmdline(current_screen, CURRENT_VIEW, (short) (strlen((char *) initial) + 1));
  else
    THEcursor_cmdline(current_screen, CURRENT_VIEW, (short) (start_col + 1));
  in_readv = TRUE;              /* this MUST go after THEcursor_cmdline() to work */
  /*
   * If we were called from execute_dialog, refresh the dialog window
   */
  if (dw)
    wnoutrefresh(dw);
  wrefresh(CURRENT_WINDOW_COMMAND);
  while (1) {
    key = wgetch(CURRENT_WINDOW_COMMAND);
    if (key == KEY_MOUSE) {
      int b, ba, bm, y, x;

      if (get_mouse_info(&b, &ba, &bm) != RC_OK)
        continue;
      if (b != 1 || ba == BUTTON_PRESSED)
        continue;
      wmouse_position(CURRENT_WINDOW_COMMAND, &y, &x);
      if (y == -1 && x == -1) {
        /*
         * Button 1 clicked or released outside of window.
         */
        if (dw != NULL) {
          rc = RC_READV_TERM_MOUSE;
          break;
        }
        continue;

      }
      /*
       * Got a valid button. Check if its a click or press
       */
      if (ba == BUTTON_CLICKED || ba == BUTTON_RELEASED) {
        /*
         * Got a mouse event
         */
        wmove(CURRENT_WINDOW_COMMAND, 0, x);
      } else
        continue;
    } else {
      rc = function_key(key, OPTION_READV, FALSE);
      switch (rc) {
        case RC_READV_TERM:
          break;
        case RAW_KEY:
          if (rc >= RAW_KEY) {
            if (rc > RAW_KEY)
              key = rc - (RAW_KEY * 2);
            if (key < 256 && key >= 0) {
              buf[0] = (char_t) key;
              rc = Text(buf);
            }
          }
          break;
        default:
          break;
      }
    }
    show_statarea();
    /*
     * If we were called from execute_dialog, refresh the dialog window
     */
    if (dw)
      wnoutrefresh(dw);
    wrefresh(CURRENT_WINDOW_COMMAND);
    if (rc == RC_READV_TERM || rc == RC_READV_TERM_MOUSE)
      break;
  }
  /*
   * If we were NOT on the command line, go back to where we were.
   */
  in_readv = FALSE;             /* this MUST go here to allow THEcursor_home() to work */
  return (rc);
}
short execute_mouse_commands(int key) {
  DEFINE *curr = (DEFINE *) NULL;
  char_t *key_cmd = NULL;
  short rc = RC_OK;
  short macrorc = 0;

  curr = first_mouse_define;
  while (curr != (DEFINE *) NULL) {
    if (key == curr->def_funkey) {
      /*
       * If running in read-only mode and the function selected is not valid
       * display an error.
       */
      if (curr->def_command != (-1)
          && ISREADONLY(CURRENT_FILE)
          && !command[curr->def_command].valid_in_readonly) {
        display_error(56, (char_t *) "", FALSE);
        rc = RC_INVALID_ENVIRON;
        curr = NULL;
        break;
      }
      /*
       * If there are no more files in the ring, and the command is not a
       * command to edit a new file, then ignore the command.
       */
      if (curr->def_command != (-1)
          && number_of_files == 0 && !command[curr->def_command].edit_command) {
        rc = RC_OK;
        curr = NULL;
        break;
      }
      if ((key_cmd = (char_t *) my_strdup(curr->def_params)) == NULL) {
        display_error(30, (char_t *) "", FALSE);
        rc = RC_OUT_OF_MEMORY;
        curr = NULL;
        break;
      }
      if (curr->def_command == (-1))
        rc = execute_macro_instore(key_cmd, &macrorc, &curr->pcode, &curr->pcode_len, NULL, curr->def_funkey);
      else {
        /*
         * If operating in CUA mode, and a CUA block exists, check
         * if the block should be reset or deleted before executing
         * the command.
         */
        if (INTERFACEx == INTERFACE_CUA && MARK_VIEW == CURRENT_VIEW && MARK_VIEW->mark_type == M_CUA) {
          ResetOrDeleteCUABlock(command[curr->def_command].cua_behaviour);
        }
        /*
         * Possibly reset the THIGHLIGHT area
         */
        if (CURRENT_VIEW->thighlight_on && CURRENT_VIEW->thighlight_active) {
          AdjustThighlight(command[curr->def_command].thighlight_behaviour);
        }
        rc = (*command[curr->def_command].function) ((char_t *) key_cmd);
      }
      free(key_cmd);
      if (rc != RC_OK && rc != RC_TOF_EOF_REACHED && rc != RC_NO_LINES_CHANGED && rc != RC_TARGET_NOT_FOUND) {
        curr = NULL;
        break;
      }
    }
    if (curr == NULL)
      break;
    curr = curr->next;
  }
  return (rc);
}
short validate_n_m(char_t * params, short *col1, short *col2) {
#define NM_PARAMS  2
  char_t *word[NM_PARAMS + 1];
  char_t strip[NM_PARAMS];
  unsigned short num_params = 0;
  short rc = RC_OK;

  /*
   * Validate the parameters that have been supplied. One only
   * parameter MUST be supplied. The first parameter MUST be a positive
   * integer. The second can be a positive integer or '*'. If no second
   * parameter is supplied, defaults to p1. The second parameter MUST be
   * >= first parameter. '*' is regarded as the biggest number and is
   * literally 255.
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(params, word, NM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    display_error(3, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (num_params > 2) {
    display_error(2, (char_t *) "", FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (!valid_positive_integer(word[0])) {
    display_error(4, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  *col1 = atoi((char *) word[0]);
  if (strcmp((char *) word[1], "*") == 0)
    *col2 = 255;
  else {
    if (num_params == 1)        /* no second parameter, default to first */
      *col2 = *col1;
    else {
      if (!valid_positive_integer(word[1])) {
        display_error(4, word[1], FALSE);
        return (RC_INVALID_OPERAND);
      } else
        *col2 = atoi((char *) word[1]);
    }
  }

  if (*col2 > 255)
    *col2 = 255;
  if (*col1 > *col2) {
    display_error(6, word[0], FALSE);
    return (RC_INVALID_OPERAND);
  }
  return (rc);
}

void ResetOrDeleteCUABlock(int cua_behaviour) {
  length_t save_col;
  line_t save_line;

  if (cua_behaviour & CUA_RESET_BLOCK) {
    Reset((char_t *) "BLOCK");
  } else if (cua_behaviour & CUA_DELETE_BLOCK) {
    /*
     * Determine start of block, and save this position so
     * we can move the cursor there after deleting the
     * marked block.
     */
    if ((CURRENT_VIEW->mark_start_line * max_line_length) + CURRENT_VIEW->mark_start_col < (CURRENT_VIEW->mark_end_line * max_line_length) + CURRENT_VIEW->mark_end_col) {
      save_line = CURRENT_VIEW->mark_start_line;
      save_col = CURRENT_VIEW->mark_start_col;
    } else {
      save_line = CURRENT_VIEW->mark_end_line;
      save_col = CURRENT_VIEW->mark_end_col;
    }
    box_operations(BOX_D, SOURCE_BLOCK_RESET, FALSE, ' ');
    THEcursor_goto(save_line, save_col);
  }
  return;
}

void AdjustThighlight(int thighlight_behaviour) {
  if (thighlight_behaviour == THIGHLIGHT_RESET_ALL) {
    CURRENT_VIEW->thighlight_active = FALSE;
  } else if (thighlight_behaviour == THIGHLIGHT_RESET_FOCUS) {
    if (CURRENT_VIEW->focus_line == CURRENT_VIEW->thighlight_target.true_line) {
      CURRENT_VIEW->thighlight_active = FALSE;
    }
  }
  return;
}

bool save_target(TARGET * target) {
  int i;
  bool string_target = FALSE;

  if (TARGETSAVEx == TARGET_ALL) {
    return TRUE;
  }

  if (TARGETSAVEx == TARGET_UNFOUND) {
    return FALSE;
  }

  for (i = 0; i < target->num_targets; i++) {
    if (TARGETSAVEx & target->rt[i].target_type) {
      string_target = TRUE;
      break;
    }
  }
  return string_target;
}

short execute_locate(char_t * cmd, bool display_parse_error, bool search_semantics, bool *target_found) {
  line_t save_focus_line = 0L;
  line_t save_current_line = 0L;
  TARGET target;
  long target_type;
  bool negative = FALSE;
  short rc;
  bool wrapped = FALSE;
  line_t true_line = 0L;
  length_t focus_column;

  if (target_found)
    *target_found = FALSE;
  /*
   * SEARCH is limited to string and regexp
   */
  if (search_semantics) {
    target_type = TARGET_STRING | TARGET_REGEXP;
    focus_column = get_true_column(TRUE);
  } else {
    target_type = TARGET_NORMAL | TARGET_SPARE | TARGET_REGEXP | TARGET_BLOCK;
    focus_column = 0;
  }

  save_focus_line = CURRENT_VIEW->focus_line;
  save_current_line = CURRENT_VIEW->current_line;
  CURRENT_VIEW->thighlight_active = FALSE;
  initialise_target(&target);
  target.search_semantics = search_semantics;
  target.focus_column = focus_column;
  rc = validate_target(cmd, &target, target_type, get_true_line(TRUE), display_parse_error, (bool) ((CURRENT_VIEW->wrap) ? FALSE : TRUE));
  /*
   * If a valid target, but target not found, continue...
   */
  if (rc == RC_TARGET_NOT_FOUND) {
    if (CURRENT_VIEW->wrap) {
      wrapped = TRUE;
      negative = target.rt[0].negative;
      free_target(&target);
      initialise_target(&target);
      target.search_semantics = search_semantics;
      target.focus_column = focus_column;
      post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
      true_line = (negative ? CURRENT_FILE->number_lines + 1 : 0L);
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      rc = validate_target(cmd, &target, target_type, true_line, display_parse_error, TRUE);
      if (rc != RC_OK) {
        CURRENT_VIEW->focus_line = save_focus_line;
        CURRENT_VIEW->current_line = save_current_line;
        pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
      }
    }
    if (rc == RC_TARGET_NOT_FOUND) {
      if (!in_macro && save_target(&target)) {
        if (search_semantics)
          rc = save_lastop(LASTOP_SEARCH, target.string);
        else
          rc = save_lastop(LASTOP_LOCATE, target.string);
        if (rc != RC_OK) {
          display_error(30, (char_t *) "", FALSE);
          return rc;
        }
      }
      /*
       * Don't free the target if THIGHLIGHT is on
       */
      if (!CURRENT_VIEW->thighlight_on)
        free_target(&target);
      rc = RC_TARGET_NOT_FOUND;
      return rc;
    }
  }
  /*
   * If a valid target and found, go there and execute any following
   * command.
   */
  if (rc == RC_OK) {
    if (target_found)
      *target_found = TRUE;
    if (wrapped) {
      display_error(0, (char_t *) "Wrapped...", FALSE);
      CURRENT_VIEW->focus_line = save_focus_line;
      CURRENT_VIEW->current_line = save_current_line;
      build_screen(current_screen);
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND || compatible_feel == COMPAT_XEDIT)
        CURRENT_VIEW->current_line = true_line;
      else
        CURRENT_VIEW->focus_line = true_line;
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
    if (!in_macro && save_target(&target)) {
      if (search_semantics)
        rc = save_lastop(LASTOP_SEARCH, target.string);
      else
        rc = save_lastop(LASTOP_LOCATE, target.string);
      if (rc != RC_OK) {
        display_error(30, (char_t *) "", FALSE);
        return rc;
      }
    }
    /*
     * If the number of targetted lines is not 0, move the current or focus line
     */
    if (target.num_lines)
      rc = advance_current_or_focus_line(target.num_lines);
    /*
     * If SEARCHing reposition the cursor or current_column
     * to the FIRST NO SET target.focus_column to found column and use that!!
     * TODO
     */
    if (search_semantics) {
      if (CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
        THEcursor_goto(CURRENT_VIEW->focus_line, target.focus_column + 1);
      } else {
        CURRENT_VIEW->current_column = target.focus_column + 1;
      }
    }
    /*
     * Set the THIGHLIGHT location if supported and the target was a string...
     */
    if (CURRENT_VIEW->thighlight_on) {
      free_target(&CURRENT_VIEW->thighlight_target);
      CURRENT_VIEW->thighlight_target = target;
      CURRENT_VIEW->thighlight_target.true_line = get_true_line(TRUE);
      CURRENT_VIEW->thighlight_active = TRUE;
      build_screen(current_screen);
      display_screen(current_screen);
    }
    if ((rc == RC_OK || rc == RC_TOF_EOF_REACHED)
        && target.spare != (-1))
      rc = command_line(MyStrip(target.rt[target.spare].string, STRIP_LEADING, ' '), FALSE);
    /*
     * Don't free the target if THIGHLIGHT is on
     */
    if (!CURRENT_VIEW->thighlight_on)
      free_target(&target);
    return rc;
  }
  free_target(&target);

  return rc;
}

void adjust_other_screen_shadow_lines(void)
/*
 * This function adjusts displayed lines in the other screen if the display lines
 * are changed in the current screen. Used by SET SELECT, SET DISPLAY, ALL, MORE and LESS
 */
{

  /*
   * If the same file is in the other screen, refresh it
   */
  if (display_screens > 1 && SCREEN_FILE(current_screen) == SCREEN_FILE((char_t) (other_screen))) {
    OTHER_VIEW->current_line = find_next_in_scope(OTHER_VIEW, NULL, OTHER_VIEW->current_line, DIRECTION_FORWARD);
    build_screen((char_t) (other_screen));
    if (!line_in_view((char_t) (other_screen), OTHER_VIEW->focus_line)) {
      OTHER_VIEW->focus_line = OTHER_VIEW->current_line;
      pre_process_line(OTHER_VIEW, OTHER_VIEW->focus_line, (LINE *) NULL);
      build_screen((char_t) (other_screen));
    }
    display_screen((char_t) (other_screen));
  }

  return;
}

int is_file_in_ring(char_t * fpath, char_t * fname) {
  VIEW_DETAILS *curr = vd_first;

  while (curr) {
    if (strcmp((char *) curr->file_for_view->fpath, (char *) fpath) == 0 && strcmp((char *) curr->file_for_view->fname, (char *) fname) == 0) {
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

int save_lastop(int idx, char_t * op) {
  int op_len;
  int rc = RC_OK;

  op_len = strlen((char *) op);
  if (op_len > lastop[idx].value_len) {
    lastop[idx].value = (char_t *) realloc(lastop[idx].value, op_len + 1);
  }
  if (lastop[idx].value) {
    strcpy((char *) lastop[idx].value, (char *) op);
    lastop[idx].value_len = op_len;
  } else {
    lastop[idx].value_len = 0;
    rc = RC_OUT_OF_MEMORY;
  }
  return rc;
}

char_t *get_command_name(int idx, bool *set_command, bool *sos_command) {

  if (idx < 0 || idx >= sizeof(command) / sizeof(struct commands) - 1) {
    return NULL;
  }
  *set_command = command[idx].set_command;
  *sos_command = command[idx].sos_command;
  return command[idx].text;
}
