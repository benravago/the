// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* REXX interface routines. */

#include "the.h"
#include "proto.h"

#include "query.h"

#define INCL_REXXSAA
#include "imc/rexxsaa.h"

#include <sys/wait.h>

LINE *rexxout_first_line = NULL;
LINE *rexxout_last_line = NULL;
LINE *rexxout_curr = NULL;
line_t rexxout_number_lines = 0L;

#define BUF_SIZE 512

RexxSubcomHandler THE_Commands;
RexxExitHandler THE_Exit_Handler;
RexxFunctionHandler THE_Function_Handler;

static RXSTRING *get_compound_rexx_variable (char *, RXSTRING *, short);
static short valid_target_function (ULONG, RXSTRING[]);
static short run_os_function (ULONG, RXSTRING[]);
static int run_os_command (char *, char *, char *, char *);
static char *MakeAscii (RXSTRING *);
static char *get_a_line (FILE *, char *, int *, int *);
static short set_rexx_variables_from_file (char *, char *);

static line_t captured_lines;
static bool rexx_halted;

unsigned long MyRexxDeregisterFunction(char * Name) {
  char newname[80];
  ULONG rc;

  strcpy(newname, Name);
  make_upper((uchar*)newname);
  rc = RexxDeregisterFunction((PSZ) newname);
  return rc;
}

unsigned long MyRexxRegisterFunctionExe(char * Name) {
  char newname[256];
  ULONG rc;

  strcpy(newname, Name);
  make_upper((uchar*)newname);
  rc = RexxRegisterFunctionExe((PSZ) newname, (RexxFunctionHandler*) THE_Function_Handler);
  return rc;
}

ULONG THE_Commands(
  PRXSTRING Command,   /* Command string passed from the caller */
  PUSHORT Flags,       /* pointer to short for return of flags */
  PRXSTRING Retstr     /* pointer to RXSTRING for RC return */
) {
  short rc = RC_OK;

  if (allocate_temp_space(Command->strlength, TEMP_TEMP_CMD) != RC_OK) {
    display_error(30, "", FALSE);
    *Flags = RXSUBCOM_ERROR;    /* raise an error condition   */
    sprintf(Retstr->strptr, "%d", rc);      /* format return code string  */
    /* and set the correct length */
    Retstr->strlength = strlen(Retstr->strptr);
    return 0L;                  /* processing completed       */
  }
  memcpy(temp_cmd, Command->strptr, Command->strlength);
  temp_cmd[Command->strlength] = '\0';
  rc = command_line((uchar*)temp_cmd, COMMAND_ONLY_FALSE);
  if (rc < 0) {
    *Flags = RXSUBCOM_ERROR;    /* raise an error condition   */
  } else {
    *Flags = RXSUBCOM_OK;       /* not found is not an error  */
  }
  sprintf(Retstr->strptr, "%d", rc);        /* format return code string  */
  /* and set the correct length */
  Retstr->strlength = strlen(Retstr->strptr);
  return 0L;                    /* processing completed       */
}

LONG THE_Exit_Handler(
  LONG ExitNumber,      /* code defining the exit function */
  LONG Subfunction,     /* code defining the exit subfunction */
  PEXIT ParmBlock       /* function dependent control block */
) {
  RXSIOTRC_PARM *trc_parm;
  LONG rc = 0L;
  char rexxout_temp[80];
  VIEW_DETAILS *found_view = NULL;
  static bool first = TRUE;

  switch (ExitNumber) {

    case RXSIO:
      if (Subfunction != RXSIOSAY && Subfunction != RXSIOTRC) {
        rc = RXEXIT_NOT_HANDLED;
        break;
      }
      trc_parm = (RXSIOTRC_PARM *) ParmBlock;
      /*
       * If this is the first time this exit handler is called, set up the
       * handling of the result; either open the capture file, or set the
       * terminal out of curses mode so scrolling etc. work.
       */
      if (!rexx_output) {
        rexx_output = TRUE;
        if (CAPREXXOUTx) {
          /*
           * Initialise the temporary file...
           */
          if (first) {
            first = FALSE;
            strcpy(rexx_pathname, dir_pathname);
            strcat(rexx_pathname, rexxoutname);
            if (splitpath(rexx_pathname) != RC_OK) {
              rc = RXEXIT_RAISE_ERROR;
            }
            strcpy(rexx_pathname, sp_path);
            strcpy(rexx_filename, (char *) sp_fname);
          }
          /*
           * Free up the existing linked list (if any)
           */
          rexxout_first_line = rexxout_last_line = lll_free(rexxout_first_line);
          rexxout_number_lines = 0L;
          if ((found_view = find_file((uchar*)rexx_pathname, (uchar*)rexx_filename)) != (VIEW_DETAILS *) NULL) {
            found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
            found_view->file_for_view->number_lines = 0L;
          }
          /*
           * first_line is set to "Top of File"
           */
          if ((rexxout_first_line = add_LINE(rexxout_first_line, NULL, (uchar*)TOP_OF_FILE, strlen(TOP_OF_FILE), 0, FALSE)) == NULL)
            rc = RXEXIT_RAISE_ERROR;
          /*
           * last line is set to "Bottom of File"
           */
          if ((rexxout_last_line = add_LINE(rexxout_first_line, rexxout_first_line, (uchar*)BOTTOM_OF_FILE, strlen(BOTTOM_OF_FILE), 0, FALSE)) == NULL) {
            rc = RXEXIT_RAISE_ERROR;
          }
          rexxout_curr = rexxout_first_line;
          if (found_view != (VIEW_DETAILS *) NULL) {
            found_view->file_for_view->first_line = rexxout_first_line;
            found_view->file_for_view->last_line = rexxout_last_line;
          }
        } else {
          if (!batch_only) {
            wmove(statarea, 0, COLS - 1);
            wrefresh(statarea);
            suspend_curses();
          }
          printf("\n");         /* scroll the screen 1 line */
          fflush(stdout);
        }
      }
      /*
       * If the REXX interpreter has been halted by line limit exceeded, just
       * return to the interpreter indicating that THE is hadnling the output
       * of messages. This is done to stop the "clutter" that comes back as
       * the interpreter tries to tell us that it is stopping.
       */
      if (rexx_halted) {
        return (RXEXIT_HANDLED);
      }
      /*
       * If we are capturing the rexx output, print the string to the file.
       */
      if (CAPREXXOUTx) {
        rexxout_number_lines++;
        if ((rexxout_curr = add_LINE(rexxout_first_line, rexxout_curr, (uchar*)trc_parm->rxsio_string.strptr, (trc_parm->rxsio_string.strlength == (-1)) ? 0 : trc_parm->rxsio_string.strlength, 0, FALSE)) == NULL) {
          rc = RXEXIT_RAISE_ERROR;
        } else {
          rc = RXEXIT_HANDLED;
        }
      } else {
        rc = RXEXIT_NOT_HANDLED;
      }
      /*
       * If the number of lines processed exceeds the line limit, display our
       * own message telling what has happened and exit with
       * RXEXIT_RAISE_ERROR. This tells the interpreter that it is to stop.
       */
      if (++captured_lines > CAPREXXMAXx) {
        if (CAPREXXOUTx) {
          rexxout_number_lines++;
          sprintf(rexxout_temp, "THE: REXX macro halted - line limit (%ld) exceeded", CAPREXXMAXx);
          rexxout_curr = add_LINE(rexxout_first_line, rexxout_curr, (uchar*)rexxout_temp, strlen(rexxout_temp), 0, FALSE);
        } else {
          printf("THE: REXX macro halted - line limit (%ld) exceeded\n", CAPREXXMAXx);
        }
        rc = RXEXIT_RAISE_ERROR;
        rexx_halted = TRUE;
      }
      break;

    default:
      rc = RXEXIT_NOT_HANDLED;
  }
  return (rc);
}

ULONG THE_Function_Handler(
  PSZ FunctionName,  /* name of function */
  ULONG Argc,        /* number of arguments */
  PRXSTRING Argv,    /* array of arguments in RXSTRINGs */
  PSZ QueueName,     /* name of queue */
  PRXSTRING Retstr   /* return string */
) {
  int functionname_length = 0;
  int itemno = 0, item_index = 0, rc = 0;  // num_vars = 0

  itemno = split_function_name((uchar*)FunctionName, &functionname_length);
  /*
   * If the tail is > maximum number of variables that we can
   * handle, exit with error.
   */
  if (itemno > MAX_VARIABLES_RETURNED) {
    return (1);
  }
  /*
   * Find the external function name in the array. Error if not found.
   */
  set_compare_exact(TRUE);
  if (itemno == (-1)) {
    rc = search_query_item_array(function_item, number_function_item(), sizeof(QUERY_ITEM), (char *) FunctionName, functionname_length);
    if (rc == (-1)) {
      return (1);
    }
    item_index = function_item[rc].item_values;
    switch (function_item[rc].item_number) {

      case ITEM_VALID_TARGET_FUNCTION:
        if (number_of_files == 0) {
          display_error(83, "", FALSE);
          return (1);
        }
        valid_target_function(Argc, Argv);
        break;

      case ITEM_RUN_OS_FUNCTION:
        run_os_function(Argc, Argv);
        break;

      default:
        if (number_of_files == 0 && function_item[rc].level != LVL_GLOB) {
          display_error(83, "", FALSE);
          return (1);
        }
        if ((Argv == NULL) || (Argc == 0)) {    /* FGC */
          get_item_values(0, function_item[rc].item_number, (uchar*)"", QUERY_FUNCTION, (line_t) Argc, NULL, 0L);
        } else {
          get_item_values(0, function_item[rc].item_number, (uchar*)"", QUERY_FUNCTION, (line_t) Argc, (uchar*)Argv[0].strptr, (line_t) Argv[0].strlength);
        }
        break;
    }
  } else {
    rc = search_query_item_array(query_item, number_query_item(), sizeof(QUERY_ITEM), (char *) FunctionName, functionname_length);
    if (rc == (-1)) {
      return (1);
    }
    item_index = itemno;
    if (number_of_files == 0 && query_item[rc].level != LVL_GLOB) {
      display_error(83, "", FALSE);
      return (1);
    }
    if ((Argv == NULL) || (Argc == 0)) {        /* FGC */
      get_item_values(1, query_item[rc].item_number, (uchar*)"", QUERY_FUNCTION, (line_t) Argc, NULL, 0L);
    } else {
      get_item_values(1, query_item[rc].item_number, (uchar*)"", QUERY_FUNCTION, (line_t) Argc, (uchar*)Argv[0].strptr, (line_t) Argv[0].strlength);
    }
  }

  if (item_values[item_index].len > 256) {
    /* this MUST use the appropriate memory allocation routine that the Rexx interpreter expects. */
    Retstr->strptr = (char*) malloc(item_values[item_index].len);
    if (Retstr->strptr == NULL) {
      display_error(30, "", FALSE);
      return (1);
    }
  }
  memcpy(Retstr->strptr, item_values[item_index].value, item_values[item_index].len);
  Retstr->strlength = item_values[item_index].len;
  return (0);
}

short initialise_rexx(void) {
  ULONG rc;
  register int i = 0, j = 0;
  int num_values = 0;
  char buf[50];

  rc = RexxRegisterSubcomExe((PSZ) "THE", (RexxSubcomHandler*) THE_Commands, (PUCHAR) NULL);
  if (rc != RXSUBCOM_OK) {
    return ((short) rc);
  }

  rc = RexxRegisterExitExe((PSZ) "THE_EXIT", (RexxExitHandler*) THE_Exit_Handler, (PUCHAR) NULL);
  if (rc != RXEXIT_OK) {
    return ((short) rc);
  }

  for (i = 0; i < number_function_item(); i++) {
    rc = MyRexxRegisterFunctionExe((char*)function_item[i].name);
    if (rc != RXFUNC_OK) {
      break;
    }
  }
  if (rc != RXFUNC_OK) {
    return ((short) rc);
  }

  for (i = 0; i < number_query_item(); i++) {
    num_values = query_item[i].item_values + 1;
    for (j = 0; j < num_values; j++) {
      sprintf(buf, "%s.%d", query_item[i].name, j);
      rc = MyRexxRegisterFunctionExe(buf);
      if (rc != RXFUNC_OK) {
        break;
      }
    }
    if (rc != RXFUNC_OK) {
      break;
    }
  }
  return ((short) rc);
}

short finalise_rexx(void) {
  ULONG rc;

  rc = RexxDeregisterSubcom((PSZ) "THE", (PSZ) NULL);
  rc = RexxDeregisterExit((PSZ) "THE_EXIT", (PSZ) NULL);

  /*
   * MHES - 7-4-2001
   * Don't bother deregistering the external functions. As this is an EXE
   * the function registrations will disappear when the process dies.
   */

  return ((short) rc);
}

short execute_macro_file(char * filename, char * params, short *macrorc, bool interactive) {
  SHORT rexxrc = 0L;
  RXSTRING retstr;
  RXSTRING argstr;
  ULONG rc = 0;

  LONG num_params = 0L;
  char *rexx_args = NULL;
  RXSYSEXIT exit_list[3];       /* system exit list           */

  /*
   * Determine how many parameters are to be passed to the interpreter.
   * Only 0 or 1 are valid values.
   */
  if (params == NULL || strcmp(params, "") == 0) {
    num_params = 0;
    MAKERXSTRING(argstr, "", 0);
    strcpy(rexx_macro_parameters, "");
  } else {
    num_params = 1;
    if ((rexx_args = (char*) malloc(strlen(params) + 1)) == (char*) NULL) {
      display_error(30, "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy(rexx_macro_parameters, params);
    strcpy(rexx_args, params);
    MAKERXSTRING(argstr, rexx_args, strlen(rexx_args));
  }

  MAKERXSTRING(retstr, NULL, 0);

  strcpy(rexx_macro_name, (char *) filename);
  /*
   * Set up pointer to REXX Exit Handler.
   */
  exit_list[0].sysexit_name = "THE_EXIT";
  exit_list[1].sysexit_name = "THE_EXIT";
  exit_list[0].sysexit_code = RXSIO;
  exit_list[1].sysexit_code = RXENDLST;

  if (interactive) {
    execute_os_command((uchar*)"echo", TRUE, FALSE);
  }

  captured_lines = 0L;
  rexx_output = FALSE;
  rexx_halted = FALSE;
  /*
   * Call the REXX interpreter.
   */
  rc = RexxStart((long) num_params, (PRXSTRING) & argstr, (char*) filename, (PRXSTRING) NULL, (PSZ) "THE", (long) RXCOMMAND, (PRXSYSEXIT) exit_list, (short*) & rexxrc, (PRXSTRING) & retstr);
  /*
   * If we get a return value in retstr (ie retstr.strptr != NULL)
   * then the Rexx interpreter has allocated some memory for us.
   * We don't want it, free it up.
   */
  free(retstr.strptr);
  /*
   * Edit the captured file or clean up after REXX output displays.
   */
  if (rexx_output) {
    rexx_output = FALSE;
    if (CAPREXXOUTx) {
      strcpy(temp_cmd, rexx_pathname);
      strcat(temp_cmd, (char *) rexx_filename);
      Xedit(temp_cmd);
    } else {
      if (batch_only) {
        error_on_screen = TRUE;
      } else {
        /*
         * Pause for operator intervention and restore the screen to the
         * current screen if there are still file(s) in the ring.
         */
        printf("\n%s", HIT_ANY_KEY);
        fflush(stdout);
        resume_curses();
        wgetch(stdscr);
        if (number_of_files > 0) {
          restore_THE();
        }
      }
    }
  }

  if (rexx_args != NULL) {
    free(rexx_args);
  }
  *macrorc = (short) rexxrc;
  return ((short) rc);
}

short execute_macro_instore(char * commands, short *macrorc, char ** pcode, int *pcode_len, int *tokenised, int macro_ident) {
  USHORT rexxrc = 0L;
  RXSTRING instore[2];
  RXSTRING retstr;
  CHAR retbuf[260];
  char macro_name[20];
  ULONG rc = 0;
  LONG num_params = 0L;
  RXSYSEXIT exit_list[2];       /* system exit list           */
  bool save_in_macro = in_macro;

  in_macro = TRUE;
  sprintf(macro_name, "INSTORE%d", macro_ident);
  /*
   * Set up pointer to REXX Exit Handler.
   */
  exit_list[0].sysexit_name = "THE_EXIT";
  exit_list[0].sysexit_code = RXSIO;
  exit_list[1].sysexit_code = RXENDLST;

  captured_lines = 0L;
  rexx_output = FALSE;
  rexx_halted = FALSE;
  instore[0].strptr = (char*) commands;
  instore[0].strlength = strlen(commands);
  if (pcode) {
    instore[1].strptr = (char*) * pcode;
  } else {
    instore[1].strptr = (char*) NULL;
  }
  instore[1].strlength = (pcode_len) ? *pcode_len : 0;
  MAKERXSTRING(retstr, retbuf, sizeof(retbuf));
  /*
   * Call the REXX interpreter.
   */
  rc = RexxStart((long) num_params, (PRXSTRING) NULL, (char*) macro_name, (PRXSTRING) instore, (PSZ) "THE", (long) RXCOMMAND, (PRXSYSEXIT) exit_list, (short*) & rexxrc, (PRXSTRING) & retstr);
  if (instore[1].strptr) {
    if (pcode_len && pcode && *pcode == NULL) { /* first time an instore defined key is hit */
      *pcode = (char*) malloc(instore[1].strlength);
      if (*pcode != NULL) {
        *pcode_len = instore[1].strlength;
        memcpy(* pcode, (char *) instore[1].strptr, *pcode_len);
        *tokenised = 1;
      }
    }
    /*
     * If the pointer to the pcode returned by RexxStart() is NOT the
     * same as the pointer we already have, then free it.
     */
    if (pcode_len && pcode && * pcode != instore[1].strptr) {
      free(instore[1].strptr);
    }
  }
  /*
   * Edit the captured file or clean up after REXX output displays.
   */
  if (rexx_output) {
    rexx_output = FALSE;
    if (CAPREXXOUTx) {
      strcpy(temp_cmd, rexx_pathname);
      strcat(temp_cmd, rexx_filename);
      Xedit(temp_cmd);
    } else {
      if (batch_only) {
        error_on_screen = TRUE;
      } else {
        /*
         * Pause for operator intervention and restore the screen to the
         * current screen if there are still file(s) in the ring.
         */
        printf("\n%s", HIT_ANY_KEY);
        fflush(stdout);
        resume_curses();
        wgetch(stdscr);
        if (number_of_files > 0) {
          restore_THE();
        }
      }
    }
  }
  in_macro = save_in_macro;
  *macrorc = (short) rexxrc;
  return ((short) rc);
}

short get_rexx_variable(char * name, char ** value, int *value_length) {
  RXSTRING retstr;
  short rc = RC_OK;

  MAKERXSTRING(retstr, "", 0);
  (void) get_compound_rexx_variable(name, &retstr, (-1));
  if (retstr.strptr == NULL) {
    rc = RC_INVALID_ENVIRON;
  }
  *value = retstr.strptr;
  *value_length = retstr.strlength;
  return (rc);
}

short set_rexx_variable(char * name, char * value, length_t value_length, int suffix) {
  SHVBLOCK shv;
  CHAR variable_name[250];
  short rc = 0;

  shv.shvnext = NULL;           /* only one block */
  shv.shvcode = RXSHV_SET;      /* use direct set */
  /*
   * This calls the RexxVariablePool() function for each value. This is
   * not the most efficient way of doing this.
   */
  if (suffix == (-1)) {
    strcpy(variable_name, (char *) name);
  } else {
    sprintf(variable_name, "%s.%-d", name, suffix);
  }
  (void) make_upper((uchar*)variable_name);        /* make variable name uppercase */
  /*
   * Now (attempt to) set the REXX variable
   * Add name/value to SHVBLOCK
   */
  MAKERXSTRING(shv.shvname, variable_name, strlen(variable_name));
  MAKERXSTRING(shv.shvvalue, (char *) value, value_length);
  /*
   * One or both of these is needed, too <sigh>
   */
  shv.shvnamelen = strlen(variable_name);
  shv.shvvaluelen = value_length;

  rc = RexxVariablePool(&shv);  /* Set the REXX variable */
  if (rc != RXSHV_OK && rc != RXSHV_NEWV) {
    display_error(25, "", FALSE);
    rc = RC_SYSTEM_ERROR;
  } else {
    rc = RC_OK;
  }
  return (rc);
}

static RXSTRING *get_compound_rexx_variable(char * name, RXSTRING * value, short suffix) {
  static SHVBLOCK shv;
  CHAR variable_name[250];
  short rc = 0;

  shv.shvnext = NULL;           /* only one block */
  shv.shvcode = RXSHV_FETCH;    /* use direct set */
  /*
   * This calls the RexxVariablePool() function for each value. This is
   * not the most efficient way of doing this.
   */
  if (suffix == (-1)) {
    strcpy(variable_name, (char *) name);
  } else {
    sprintf(variable_name, "%s.%-d", name, suffix);
  }
  /*
   * Now (attempt to) get the REXX variable
   * Set shv.shvvalue to NULL to force interpreter to allocate memory.
   */
  MAKERXSTRING(shv.shvname, variable_name, strlen(variable_name));
  shv.shvvalue.strptr = NULL;
  shv.shvvalue.strlength = 0;
  /*
   * One or both of these is needed, too <sigh>
   */
  shv.shvnamelen = strlen(variable_name);
  shv.shvvaluelen = 0;
  rc = RexxVariablePool(&shv);  /* Set the REXX variable */
  switch (rc) {

    case RXSHV_OK:
      value->strptr = (char *) malloc((sizeof(char) * shv.shvvalue.strlength) + 1);
      if (value->strptr != NULL) {
        value->strlength = shv.shvvalue.strlength;
        memcpy(value->strptr, shv.shvvalue.strptr, value->strlength);
        *(value->strptr + value->strlength) = '\0';
      }
      free(shv.shvvalue.strptr);
      break;

    case RXSHV_NEWV:
      value->strptr = (char *) malloc((sizeof(char) * shv.shvname.strlength) + 1);
      if (value->strptr != NULL) {
        value->strlength = shv.shvname.strlength;
        memcpy(value->strptr, shv.shvname.strptr, value->strlength);
        *(value->strptr + value->strlength) = '\0';
      }
      break;

    default:
      value->strptr = NULL;
      value->strlength = 0;
      break;
  }
  return (value);
}

static short valid_target_function(ULONG Argc, RXSTRING Argv[]) {
  static TARGET target = { NULL, 0L, 0L, 0L, NULL, 0, 0, FALSE };
  short target_type = (short) (TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL);
  line_t true_line = 0L;
  short rc = 0;

  /* We use the switch(1) here to use the break; capability; a bit ugly... */
  switch (1) {
    case 1:
      if (Argc < 1 || Argc > 2) {       /* incorrect no of arguments - error */
        item_values[1].value = (uchar*) "ERROR";
        item_values[1].len = 5;
        break;
      }
      if (Argc == 2) {
        target_type = target_type | TARGET_SPARE;
      }
      /* allocate sufficient space for the spare string and 2 longs */
      if (target_buffer == NULL) {
        target_buffer = (char*) malloc(sizeof(char) * (Argv[0].strlength + 30));
        target_buffer_len = Argv[0].strlength + 30;
      } else {
        if (target_buffer_len < Argv[0].strlength + 30) {
          target_buffer = (char*) realloc(target_buffer, sizeof(char) * (Argv[0].strlength + 30));
          target_buffer_len = Argv[0].strlength + 30;
        }
      }
      if (target_buffer == NULL) {
        item_values[1].value = (uchar*) "ERROR";
        item_values[1].len = 5;
        free_target(&target);
        break;
      }
      memcpy(target_buffer, Argv[0].strptr, Argv[0].strlength);
      *(target_buffer + Argv[0].strlength) = '\0';
      if (in_prefix_macro) {
        true_line = prefix_current_line;
      } else {
        true_line = get_true_line(TRUE);
      }
      initialise_target(&target);
      rc = validate_target((uchar*)target_buffer, &target, target_type, true_line, FALSE, FALSE);
      if (rc == RC_TARGET_NOT_FOUND) {
        item_values[1].value = (uchar*) "NOTFOUND";
        item_values[1].len = 8;
        free_target(&target);
        break;
      }
      if (rc != RC_OK) {
        item_values[1].value = (uchar*) "ERROR";
        item_values[1].len = 5;
        free_target(&target);
        break;
      }
      if (Argc == 2 && target.spare != (-1)) {
        sprintf(target_buffer, "%ld %ld %s", target.true_line, target.num_lines, target.rt[target.spare].string);
      } else {
        sprintf(target_buffer, "%ld %ld", target.true_line, target.num_lines);
      }
      item_values[1].value = (uchar*)target_buffer;
      item_values[1].len = strlen(target_buffer);
      free_target(&target);
      break;
  }
  item_values[0].value = (uchar*) "1";
  item_values[0].len = 1;
  return (rc);
}

static short run_os_function(ULONG Argc, RXSTRING Argv[]) {
  int rc = 0;
  static char num0[5];      /* large enough for 1000+rc */
  char *cmd = NULL, *instem = NULL, *outstem = NULL, *errstem = NULL;

  switch (Argc) {

    case 0:
      sprintf(num0, "%d", RC_INVALID_OPERAND + 1000);
      break;

    case 4:
      if ((errstem = MakeAscii(&Argv[3])) == NULL) {
        sprintf(num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }

    case 3:
      if ((outstem = MakeAscii(&Argv[2])) == NULL) {
        sprintf(num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }

    case 2:
      if ((instem = MakeAscii(&Argv[1])) == NULL) {
        sprintf(num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }

    case 1:
      if ((cmd = MakeAscii(&Argv[0])) == NULL) {
        sprintf(num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }
      rc = run_os_command(cmd, instem, outstem, errstem);
      sprintf(num0, "%hd", rc);
      break;

    default:
      sprintf(num0, "%d", RC_INVALID_OPERAND + 1000);
      break;
  }
  item_values[1].value = (uchar*)num0;
  item_values[1].len = strlen(num0);
  item_values[0].value = (uchar*) "1";
  item_values[0].len = 1;
  if (cmd) {
    free(cmd);
  }
  if (instem) {
    free(instem);
  }
  if (outstem) {
    free(outstem);
  }
  if (errstem) {
    free(errstem);
  }
  return (RC_OK);
}

static int run_os_command(char * cmd, char * instem, char * outstem, char * errstem) {
  RXSTRING tmpstr;
  bool in = TRUE, out = TRUE, err = TRUE;
  bool out_and_err_same = FALSE;
  int inlen = 0, outlen = 0, errlen = 0;
  int i = 0;
  FILE *infp = NULL;
  char *infile = "", *outfile = "", *errfile = "";
  long innum = 0L;
  int rc = 0, rcode = 0;
  int save_stdin = 0, save_stdout = 0, save_stderr = 0;
  int infd = 0, outfd = 0, errfd = 0;

  /*
   * Determine if we are redirecting stdin, stdout or both and if the
   * values passed as stem variables end in '.'.
   */
  if (instem == NULL || strcmp(instem, "") == 0) {
    in = FALSE;
  } else {
    inlen = strlen(instem);
    if (*(instem + inlen - 1) == '.') {
      *(instem + inlen - 1) = '\0';
    } else {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  if (outstem == NULL || strcmp(outstem, "") == 0) {
    out = FALSE;
  } else {
    outlen = strlen(outstem);
    if (*(outstem + outlen - 1) == '.') {
      *(outstem + outlen - 1) = '\0';
    } else {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  if (errstem == NULL || strcmp(errstem, "") == 0) {
    err = FALSE;
  } else {
    errlen = strlen(errstem);
    if (*(errstem + errlen - 1) == '.') {
      *(errstem + errlen - 1) = '\0';
    } else {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  /*
   * Ensure that stdin stem is different to both stdout and stderr stems.
   */
  if (in) {
    if (out && strcmp(instem, (char *) outstem) == 0) {
      return (RC_INVALID_OPERAND + 1000);
    }
    if (err && strcmp(instem, (char *) errstem) == 0) {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  /*
   * An extra check here to determine if stdout and stderr are to be
   * redirected to the same place.
   */
  if (out && err) {
    if (strcmp(outstem, (char *) errstem) == 0) {
      out_and_err_same = TRUE;
    }
  }
  /*
   * If redirecting stdin, get the value of instem.0 to determine how
   * many variables to get...
   */
  if (in) {
    tmpstr.strptr = NULL;
    (void) get_compound_rexx_variable(instem, &tmpstr, 0);
    if (tmpstr.strptr == NULL) {
      return (RC_SYSTEM_ERROR + 1000);
    }
    if (!valid_positive_integer((uchar*)tmpstr.strptr)) {
      return (RC_INVALID_OPERAND + 1000);
    }
    innum = atol(tmpstr.strptr);
    free(tmpstr.strptr);
    /*
     * Write the contents of the stdin stem to a temporary file...
     */
    infile = (char *) malloc(L_tmpnam);
    if (infile == NULL) {
      return (RC_OUT_OF_MEMORY + 1000);
    }
    if ((infile = tmpnam(infile)) == NULL) {
      return (RC_ACCESS_DENIED + 1000);
    }
    if ((infp = fopen(infile, "w")) == NULL) {
      return (RC_ACCESS_DENIED + 1000);
    }
    for (i = 0; i < innum; i++) {
      tmpstr.strptr = NULL;
      (void) get_compound_rexx_variable(instem, &tmpstr, i + 1);
      if (tmpstr.strptr == NULL) {
        return (RC_SYSTEM_ERROR + 1000);
      }
      fputs(tmpstr.strptr, infp);
      fputs("\n", infp);
      free(tmpstr.strptr);
    }
    if (fclose(infp)) {
      return (RC_ACCESS_DENIED + 1000);
    }
  }
  /*
   * If redirecting stdout, create the name of a temporary file for the output.
   */
  if (out) {
    outfile = (char *) malloc(L_tmpnam);
    if (outfile == NULL) {
      return (RC_OUT_OF_MEMORY + 1000);
    }
    if ((outfile = tmpnam(outfile)) == NULL) {
      return (RC_ACCESS_DENIED + 1000);
    }
  }
  /*
   * If redirecting stderr, create the name of a temporary file for the
   * output.
   */
  if (err) {
    if (out_and_err_same) {
      errfile = outfile;
    } else {
      errfile = (char *) malloc(L_tmpnam);
      if (errfile == NULL) {
        return (RC_OUT_OF_MEMORY + 1000);
      }
      if ((errfile = tmpnam(errfile)) == NULL) {
        return (RC_ACCESS_DENIED + 1000);
      }
    }
  }
  /*
   * Save file ids for stdin, stdout and stderr, then reassign them to
   * the appropriate temporary files.
   */
  if (in) {
    save_stdin = dup(fileno(stdin));
  }
  if (out) {
    save_stdout = dup(fileno(stdout));
  }
  if (err) {
    save_stderr = dup(fileno(stderr));
  }
  if (in) {
    if ((infd = open(infile, O_RDONLY)) == (-1)) {
      return (RC_ACCESS_DENIED + 1000);
    }
  }
  if (out) {
    if ((outfd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD)) == (-1)) {
      return (RC_ACCESS_DENIED + 1000);
    }
  }
  if (out_and_err_same) {
    errfd = outfd;
  } else {
    if (err) {
      if ((errfd = open(errfile, O_RDWR | O_CREAT | O_TRUNC)) == (-1)) {
        return (RC_ACCESS_DENIED + 1000);
      }
    }
  }
  if (out) {
    chmod(outfile, S_IWUSR | S_IRUSR);
  }
  if (!out_and_err_same) {
    if (err) {
      chmod(errfile, S_IWUSR | S_IRUSR);
    }
  }
  if (in) {
    dup2(infd, fileno(stdin));
  }
  if (out) {
    dup2(outfd, fileno(stdout));
  }
  if (err) {
    dup2(errfd, fileno(stderr));
  }
  if (in) {
    close(infd);
  }
  if (out) {
    close(outfd);
  }
  if (!out_and_err_same) {
    if (err) {
      close(errfd);
    }
  }
  /*
   * Execute the OS command supplied.
   */
  rcode = system(cmd);
  if (rcode) {
    rcode = WEXITSTATUS(rcode);
  }
  /*
   * Put all file ids back the way they were...
   */
  if (in) {
    dup2(save_stdin, fileno(stdin));
  }
  if (out) {
    dup2(save_stdout, fileno(stdout));
  }
  if (err) {
    dup2(save_stderr, fileno(stderr));
  }
  if (in) {
    close(save_stdin);
  }
  if (out) {
    close(save_stdout);
  }
  if (err) {
    close(save_stderr);
  }
  /*
   * If redirecting stdout, we now have to read the file and set a REXX variable for each line read.
   */
  if (out) {
    rc = set_rexx_variables_from_file(outfile, outstem);
  }
  if (err) {
    if (!out_and_err_same) {
      rc = set_rexx_variables_from_file(errfile, errstem);
    }
  }
  /*
   * Delete the temporary file(s) and free up any memory.
   */
  if (in) {
    unlink(infile);
    free(infile);
  }
  if (out) {
    unlink(outfile);
    free(outfile);
  }
  if (err) {
    if (!out_and_err_same) {
      unlink(errfile);
      free(errfile);
    }
  }
  /*
   * Return with, hopefully, return code from system() command.
   */
  if (rc) {
    return (rc + 1000);
  } else {
    return (rcode);
  }
}

static char *MakeAscii(RXSTRING * rxstring) {
  char *string = NULL;

  string = (char*) malloc((sizeof(char) * rxstring->strlength) + 1);
  if (string != NULL) {
    memcpy(string, rxstring->strptr, rxstring->strlength);
    *(string + (rxstring->strlength)) = '\0';
  }
  return (string);
}

static char *get_a_line(FILE * fp, char *string, int *length, int *rcode) {
  int ch;
  static int bufs = 1;
  register int i = 0;

  /*
   * Allocate the first block of memory.
   */
  if ((string = (char *) malloc(BUF_SIZE + 1)) == NULL) {
    *rcode = RC_OUT_OF_MEMORY;
    return (NULL);
  }
  for (;;) {
    /*
     * Read a character from the stream...
     */
    if ((ch = fgetc(fp)) == EOF) {
      /*
       * If EOF is reached, check that it really is end of file.
       */
      if (feof(fp)) {
        *length = i;
        *rcode = RC_TOF_EOF_REACHED;
        return (string);
      }
    }
    /*
     * If end of line is reached, nul terminate string and return.
     */
    if ((char) ch == '\n') {
      *(string + i) = '\0';
      break;
    }
    /*
     * All other characters, copy to string.
     */
    *(string + i++) = (char) ch;
    /*
     * If we have got to the end of the allocated memory, realloc some more
     */
    if (i == BUF_SIZE * bufs) {
      if ((string = (char *) realloc(string, (BUF_SIZE * (++bufs)) + 1)) == NULL) {
        *rcode = RC_OUT_OF_MEMORY;
        return (NULL);
      }
    }
  }
  /*
   * Return a line read from the temporary file.
   */
  *length = i;
  *rcode = 0;
  return (string);
}

static short set_rexx_variables_from_file(char *filename, char * stem) {
  FILE *fp = NULL;
  register int i = 0;
  char *string = NULL;
  int length = 0, rcode = 0, rc = 0;
  char tmpnum[12];

  if ((fp = fopen(filename, "r")) == NULL) {
    return (RC_ACCESS_DENIED);
  }
  for (i = 0;; i++) {
    string = get_a_line(fp, string, &length, &rcode);
    if (rcode == RC_OUT_OF_MEMORY) {
      return (rcode);
    }
    if (rcode == RC_TOF_EOF_REACHED && length == 0) {
      free(string);
      break;
    }
    rc = set_rexx_variable(stem, string, strlen(string), i + 1);
    free(string);
    if (rcode == RC_TOF_EOF_REACHED) {
      break;
    }
  }
  sprintf(tmpnum, "%d", i);
  rc = set_rexx_variable(stem, tmpnum, strlen(tmpnum), 0);
  if (fclose(fp)) {
    rc = RC_ACCESS_DENIED;
  }
  return (rc);
}

