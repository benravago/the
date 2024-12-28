// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include <sys/wait.h>

#include "the.h"
#include "proto.h"

#define _declare_ extern
#include "query.h"

#include "rexxsaa.h"

LINE *rexxout_first_line = NULL;
LINE *rexxout_last_line = NULL;
LINE *rexxout_curr = NULL;
long rexxout_number_lines = 0L;

#define BUF_SIZE 512

RexxSubcomHandler THE_Commands;
RexxExitHandler THE_Exit_Handler;
RexxFunctionHandler THE_Function_Handler;

static RXSTRING *get_compound_rexx_variable(uchar *, RXSTRING *, short);
static short valid_target_function(ULONG, RXSTRING[]);
static short run_os_function(ULONG, RXSTRING[]);
static int run_os_command(uchar *, uchar *, uchar *, uchar *);
static uchar *MakeAscii(RXSTRING *);
static char *get_a_line(FILE *, char *, int *, int *);
static short set_rexx_variables_from_file(char *, uchar *);

static long captured_lines;
static bool rexx_halted;

/*
 * The following 2 variables count the number of calls made within the execution of a macro in case SET REXXHALT is set
 */
static int command_calls;
static int function_calls;
static int halt_signalled;

/*
 * This function is made available for functions to deregister implied functions.
 */
unsigned long MyRexxDeregisterFunction(uchar *Name      /* Name of function to be deregistered  */
                                      ) {
  uchar newname[256];
  ULONG rc;

  strcpy((char *) newname, (char *) Name);
  make_upper(newname);
  rc = RexxDeregisterFunction((PSZ) newname);
  return rc;
}

unsigned long MyRexxRegisterFunctionExe(uchar *Name) {
  uchar newname[256];
  ULONG rc;

  strcpy((char *) newname, (char *) Name);
  make_upper(newname);
  rc = RexxRegisterFunctionExe((PSZ) newname, (RexxFunctionHandler *) THE_Function_Handler);
  return rc;
}

ULONG THE_Commands(PRXSTRING Command, PUSHORT Flags, PRXSTRING Retstr) {
  short rc = RC_OK;

  if (allocate_temp_space(Command->strlength, TEMP_TEMP_CMD) != RC_OK) {
    display_error(30, (uchar *) "", FALSE);
    *Flags = RXSUBCOM_ERROR;    /* raise an error condition   */
    sprintf((char *) Retstr->strptr, "%d", rc); /* format return code string  */
    /* and set the correct length */
    Retstr->strlength = strlen((char *) Retstr->strptr);
    return 0L;                  /* processing completed       */
  }
  memcpy(temp_cmd, Command->strptr, Command->strlength);
  temp_cmd[Command->strlength] = '\0';
  rc = command_line(temp_cmd, COMMAND_ONLY_FALSE);
  if (rc == RC_TERMINATE_MACRO) {
    *Flags = RXSUBCOM_OK;       /* user termination is not an error  */
    // RexxSetHalt( getpid(), 0 );          /* raise a halt condition */
    halt_signalled = 1;
  } else if (rc < 0) {
    *Flags = RXSUBCOM_ERROR;  /* raise an error condition   */
  } else {
    *Flags = RXSUBCOM_OK;  /* not found is not an error  */
  }
  /*
   * Test if we have exceeded the number of command calls...
   */
  command_calls++;
  if (COMMANDCALLSx != 0 && command_calls >= COMMANDCALLSx) {
    *Flags = RXSUBCOM_OK;       /* macro termination is not an error  */
    if (halt_signalled == 0) {
      // RexxSetHalt( getpid(), 0 );          /* raise a halt condition */
      halt_signalled = 1;
    }
    rc = 0;
  }
  /*
   * format return code string and set the correct length
   */
  Retstr->strlength = sprintf((char *) Retstr->strptr, "%d", rc);
  return 0L;                    /* processing completed       */
}

LONG THE_Exit_Handler(LONG ExitNumber, LONG Subfunction, PEXIT ParmBlock) {
  LONG rc = 0L;
  int i = 0;
  char rexxout_temp[100];
  VIEW_DETAILS *found_view = NULL;
  RXSTRING *saytrcstr;
  static bool first = TRUE;
  FILE *outfp;

  if (ExitNumber == RXSIO) {
    switch (Subfunction) {

      case RXSIOSAY:
      case RXSIOTRC: {
        if (Subfunction == RXSIOSAY) {
          RXSIOSAY_PARM *say_parm = (RXSIOSAY_PARM *) ParmBlock;

          saytrcstr = &say_parm->rxsio_string;
          outfp = stdout;
        } else {
          RXSIOTRC_PARM *trc_parm = (RXSIOTRC_PARM *) ParmBlock;

          saytrcstr = &trc_parm->rxsio_string;
          outfp = stderr;
        }
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
              strcpy((char *) rexx_pathname, (char *) dir_pathname);
              strcat((char *) rexx_pathname, (char *) rexxoutname);
              if (splitpath(rexx_pathname) != RC_OK) {
                rc = RXEXIT_RAISE_ERROR;
              }
              strcpy((char *) rexx_pathname, (char *) sp_path);
              strcpy((char *) rexx_filename, (char *) sp_fname);
            }
            /*
             * Free up the existing linked list (if any)
             */
            rexxout_first_line = rexxout_last_line = lll_free(rexxout_first_line);
            rexxout_number_lines = 0L;
            if ((found_view = find_file(rexx_pathname, rexx_filename)) != (VIEW_DETAILS *) NULL) {
              found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
              found_view->file_for_view->number_lines = 0L;
            }
            /*
             * first_line is set to "Top of File"
             */
            if ((rexxout_first_line = add_LINE(rexxout_first_line, NULL, TOP_OF_FILE, strlen((char *) TOP_OF_FILE), 0, FALSE)) == NULL) {
              rc = RXEXIT_RAISE_ERROR;
            }
            /*
             * last line is set to "Bottom of File"
             */
            if ((rexxout_last_line = add_LINE(rexxout_first_line, rexxout_first_line, BOTTOM_OF_FILE, strlen((char *) BOTTOM_OF_FILE), 0, FALSE)) == NULL) {
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
            fputc('\n', outfp);       /* scroll the screen 1 line */
            if (saytrcstr->strptr != NULL) {
              for (i = 0; i < (long) saytrcstr->strlength; i++) {
                fputc((char) saytrcstr->strptr[i], outfp);
              }
            }
            fputc('\n', outfp);
            rc = RXEXIT_HANDLED;
          }
        }
        /*
         * If the REXX interpreter has been halted by line limit exceeded,
         * just return to the interpreter indicating that THE is hadnling the output of messages.
         * This is done to stop the "clutter" that comes back
         * as the interpreter tries to tell us that it is stopping.
         */
        if (rexx_halted) {
          return (RXEXIT_HANDLED);
        }
        /*
         * If we are capturing the rexx output, print the string to the file.
         */
        if (CAPREXXOUTx) {
          rexxout_number_lines++;
          if ((rexxout_curr = add_LINE(rexxout_first_line, rexxout_curr, (uchar *) saytrcstr->strptr, (saytrcstr->strlength == (-1)) ? 0 : saytrcstr->strlength, 0, FALSE)) == NULL) {
            rc = RXEXIT_RAISE_ERROR;
          } else {
            rc = RXEXIT_HANDLED;
          }
        } else {
          if (saytrcstr->strptr != NULL) {
            for (i = 0; i < (long) saytrcstr->strlength; i++) {
              fputc((char) saytrcstr->strptr[i], outfp);
            }
          }
          fputc('\n', outfp);
          rc = RXEXIT_HANDLED;
        }
        /*
         * If the number of lines processed exceeds the line limit,
         * display our own message telling what has happened and exit with RXEXIT_RAISE_ERROR.
         * This tells the interpreter that it is to stop.
         */
        if (++captured_lines > CAPREXXMAXx) {
          if (CAPREXXOUTx) {
            rexxout_number_lines++;
            sprintf(rexxout_temp, "THE: REXX macro halted - line limit (%ld) exceeded", CAPREXXMAXx);
            rexxout_curr = add_LINE(rexxout_first_line, rexxout_curr, (uchar *) rexxout_temp, strlen((char *) rexxout_temp), 0, FALSE);
          } else {
            printf("THE: REXX macro halted - line limit (%ld) exceeded\n", CAPREXXMAXx);
          }
          rc = RXEXIT_RAISE_ERROR;
          rexx_halted = TRUE;
        }
        break;
      }
      case RXSIOTRD: {
        RXSIOTRD_PARM *trd_parm = (RXSIOTRD_PARM *) ParmBlock;
        int ch = 0;

        do {
          if (i < 256) {
            trd_parm->rxsiotrd_retc.strptr[i++] = ch = getc(stdin);
          }
        } while (ch != '\012' && (ch != EOF));
        trd_parm->rxsiotrd_retc.strlength = i - 1;
        rc = RXEXIT_HANDLED;
        break;
      }
      case RXSIODTR: {
        RXSIODTR_PARM *dtr_parm = (RXSIODTR_PARM *) ParmBlock;
        int ch = 0;

        do {
          if (i < 256) {
            dtr_parm->rxsiodtr_retc.strptr[i++] = ch = getc(stdin);
          }
        } while (ch != '\012' && (ch != EOF));
        dtr_parm->rxsiodtr_retc.strlength = i - 1;
        rc = RXEXIT_HANDLED;
        break;
      }
      default: {
        rc = RXEXIT_NOT_HANDLED;
      }
    }
  } else {
    rc = RXEXIT_NOT_HANDLED;
  }
  return (rc);
}

ULONG THE_Function_Handler(PSZ FunctionName, ULONG Argc, PRXSTRING Argv, PSZ QueueName, PRXSTRING Retstr) {
  int functionname_length = 0;
  int itemno = 0, item_index = 0, rc = 0;

  itemno = split_function_name((uchar *) FunctionName, &functionname_length);
  /*
   * If the tail is > maximum number of variables that we can handle, exit with error.
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
          display_error(83, (uchar *) "", FALSE);
          return (1);
        }
        valid_target_function(Argc, Argv);
        break;
      case ITEM_RUN_OS_FUNCTION:
        run_os_function(Argc, Argv);
        break;
      default:
        if (number_of_files == 0 && function_item[rc].level != LVL_GLOB) {
          display_error(83, (uchar *) "", FALSE);
          return (1);
        }
        if ((Argv == NULL) || (Argc == 0)) {    /* FGC */
          get_item_values(0, function_item[rc].item_number, (uchar *) "", QUERY_FUNCTION, (long) Argc, NULL, 0L);
        } else {
          get_item_values(0, function_item[rc].item_number, (uchar *) "", QUERY_FUNCTION, (long) Argc, (uchar *) Argv[0].strptr, (long) Argv[0].strlength);
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
      display_error(83, (uchar *) "", FALSE);
      return (1);
    }
    if ((Argv == NULL) || (Argc == 0)) {        /* FGC */
      get_item_values(1, query_item[rc].item_number, (uchar *) "", QUERY_FUNCTION, (long) Argc, NULL, 0L);
    } else {
      get_item_values(1, query_item[rc].item_number, (uchar *) "", QUERY_FUNCTION, (long) Argc, (uchar *) Argv[0].strptr, (long) Argv[0].strlength);
    }
  }
  /*
   * Test if we have exceeded the number of function calls...
   */
  function_calls++;
  if (FUNCTIONCALLSx != 0 && function_calls >= FUNCTIONCALLSx) {
    // RexxSetHalt( getpid(), 0 );          /* raise a halt condition */
    halt_signalled = 1;
  }

  if (item_values[item_index].len > 256) {
    /*
     * this MUST use the appropriate memory allocation routine that the Rexx interpreter expects.
     */
    Retstr->strptr = (char *) malloc(item_values[item_index].len);
    if (Retstr->strptr == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (1);
    }
  }
  memcpy(Retstr->strptr, item_values[item_index].value, item_values[item_index].len);
  Retstr->strlength = item_values[item_index].len;
  return (0);
}

short initialise_rexx(void) {
  ULONG rc;
  int i = 0, j = 0;
  int num_values = 0;
  char buf[50];

  rc = RexxRegisterSubcomExe((PSZ) "THE", (RexxSubcomHandler *) THE_Commands, (PUCHAR) NULL);
  if (rc != RXSUBCOM_OK) {
    return ((short) rc);
  }
  rc = RexxRegisterExitExe((PSZ) "THE_EXIT", (RexxExitHandler *) THE_Exit_Handler, (PUCHAR) NULL);
  if (rc != RXEXIT_OK) {
    return ((short) rc);
  }
  for (i = 0; i < number_function_item(); i++) {
    rc = MyRexxRegisterFunctionExe(function_item[i].name);
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
      rc = MyRexxRegisterFunctionExe((uchar *) buf);
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
  return ((short) rc);
}

short execute_macro_file(uchar *filename, uchar *params, short *macrorc, bool interactive) {
  SHORT rexxrc = 0L;

  RXSTRING retstr;
  RXSTRING argstr;
  ULONG rc = 0;
  LONG num_params = 0L;
  uchar *rexx_args = NULL;
  RXSYSEXIT exit_list[3];       /* system exit list           */

  /*
   * Determine how many parameters are to be passed to the interpreter.
   * Only 0 or 1 are valid values.
   */
  if (params == NULL || strcmp((char *) params, "") == 0) {
    num_params = 0;
    MAKERXSTRING(argstr, "", 0);
    strcpy((char *) rexx_macro_parameters, "");
  } else {
    num_params = 1;
    if ((rexx_args = (uchar *) malloc(strlen((char *) params) + 1)) == (uchar *) NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) rexx_macro_parameters, (char *) params);
    strcpy((char *) rexx_args, (char *) params);
    MAKERXSTRING(argstr, (char *) rexx_args, strlen((char *) rexx_args));
  }
  MAKERXSTRING(retstr, NULL, 0);
  /*
   * Save the name of the macro being run so we can see the name in the pseudo file for Rexx output
   */
  strcpy((char *) rexx_macro_name, (char *) filename);
  /*
   * Set up pointer to REXX Exit Handler.
   */
  exit_list[0].sysexit_name = "THE_EXIT";
  exit_list[1].sysexit_name = "THE_EXIT";
  exit_list[0].sysexit_code = RXSIO;
  exit_list[1].sysexit_code = RXENDLST;
  if (interactive) {
    execute_os_command((uchar *) "echo", TRUE, FALSE);
  }
  captured_lines = 0L;
  rexx_output = FALSE;
  rexx_halted = FALSE;
  /*
   * Reset our count of calls to commands, functions and messages
   */
  function_calls = command_calls = 0;
  halt_signalled = 0;
  /*
   * Call the REXX interpreter.
   */
  rc = RexxStart((long) num_params, (PRXSTRING) & argstr, (char *) filename, (PRXSTRING) NULL, (PSZ) "THE", (long) RXCOMMAND, (PRXSYSEXIT) exit_list, (short *) &rexxrc, (PRXSTRING) & retstr);
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
      strcpy((char *) temp_cmd, (char *) rexx_pathname);
      strcat((char *) temp_cmd, (char *) rexx_filename);
      Xedit(temp_cmd);
    } else {
      if (batch_only) {
        error_on_screen = TRUE;
      } else {
        /*
         * Pause for operator intervention and
         * restore the screen to the current screen if there are still file(s) in the ring.
         */
        printf("\n%s", HIT_ANY_KEY);
        fflush(stdout);
        (void) my_getch(stdscr);
        resume_curses();
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

short execute_macro_instore(uchar *commands, short *macrorc, uchar **pcode, int *pcode_len, int *tokenised, int macro_ident) {
  SHORT rexxrc = 0L;
  RXSTRING instore[2];
  RXSTRING retstr;
  CHAR retbuf[260];
  uchar macro_name[20];
  ULONG rc = 0;
  LONG num_params = 0L;
  RXSYSEXIT exit_list[2];       /* system exit list           */
  bool save_in_macro = in_macro;

  in_macro = TRUE;
  sprintf((char *) macro_name, "INSTORE%d", macro_ident);
  /*
   * Set up pointer to REXX Exit Handler.
   */
  exit_list[0].sysexit_name = "THE_EXIT";
  exit_list[0].sysexit_code = RXSIO;
  exit_list[1].sysexit_code = RXENDLST;
  captured_lines = 0L;
  rexx_output = FALSE;
  rexx_halted = FALSE;
  instore[0].strptr = (char *) commands;
  instore[0].strlength = strlen((char *) commands);
  if (pcode) {
    instore[1].strptr = (char *) *pcode;
  } else {
    instore[1].strptr = (char *) NULL;
  }
  instore[1].strlength = (pcode_len) ? *pcode_len : 0;
  MAKERXSTRING(retstr, retbuf, sizeof(retbuf));
  /*
   * Reset our count of calls to commands, functions and messages
   */
  function_calls = command_calls = 0;
  /*
   * Call the REXX interpreter.
   */
  rc = RexxStart((long) num_params, (PRXSTRING) NULL, (char *) macro_name, (PRXSTRING) instore, (PSZ) "THE", (long) RXCOMMAND, (PRXSYSEXIT) exit_list, (short *) &rexxrc, (PRXSTRING) & retstr);
  if (instore[1].strptr) {
    if (pcode_len && pcode && *pcode == NULL) { /* first time an instore defined key is hit */
      *pcode = (uchar *) malloc(instore[1].strlength);
      if (*pcode != NULL) {
        *pcode_len = instore[1].strlength;
        memcpy((char *) *pcode, (char *) instore[1].strptr, *pcode_len);
        *tokenised = 1;
      }
    }
    /*
     * If the pointer to the pcode returned by RexxStart() is NOT the
     * same as the pointer we already have, then free it.
     */
    if (pcode_len && pcode && (uchar *) * pcode != (uchar *) instore[1].strptr) {
      free(instore[1].strptr);
    }
  }
  /*
   * Edit the captured file or clean up after REXX output displays.
   */
  if (rexx_output) {
    rexx_output = FALSE;
    if (CAPREXXOUTx) {
      strcpy((char *) temp_cmd, (char *) rexx_pathname);
      strcat((char *) temp_cmd, (char *) rexx_filename);
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
        (void) my_getch(stdscr);
        resume_curses();
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

short get_rexx_variable(uchar *name, uchar **value, int *value_length) {
  RXSTRING retstr;
  short rc = RC_OK;

  MAKERXSTRING(retstr, "", 0);
  (void) get_compound_rexx_variable(name, &retstr, (-1));
  if (retstr.strptr == NULL) {
    rc = RC_INVALID_ENVIRON;
  }
  *value = (uchar *) retstr.strptr;
  *value_length = retstr.strlength;
  return (rc);
}

short set_rexx_variable(uchar *name, uchar *value, long value_length, int suffix) {
  SHVBLOCK shv;
  CHAR variable_name[250];
  short rc = 0;

  shv.shvnext = NULL;           /* only one block */
  shv.shvcode = RXSHV_SET;      /* use direct set */
  /*
   * This calls the RexxVariablePool() function for each value.
   * This is not the most efficient way of doing this.
   */
  if (suffix == (-1)) {
    strcpy(variable_name, (char *) name);
  } else {
    sprintf(variable_name, "%s.%-d", name, suffix);
  }
  (void) make_upper((uchar *) variable_name);   /* make variable name uppercase */
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
  /* rc = RXSHV_OK;*/
  if (rc != RXSHV_OK && rc != RXSHV_NEWV) {
    display_error(25, (uchar *) "", FALSE);
    rc = RC_SYSTEM_ERROR;
  } else {
    rc = RC_OK;
  }
  return (rc);
}

static RXSTRING *get_compound_rexx_variable(uchar *name, RXSTRING *value, short suffix) {
  static SHVBLOCK shv;
  CHAR variable_name[250];
  short rc = 0;

  shv.shvnext = NULL;           /* only one block */
  shv.shvcode = RXSHV_FETCH;    /* use direct set */
  /*
   * This calls the RexxVariablePool() function for each value.
   * This is not the most efficient way of doing this.
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
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL;
  long true_line = 0L;
  short rc = 0;

  /*
   * We use the switch(1) here to use the break; capability; a bit ugly...
   */
  switch (1) { case 1:
    if (Argc < 1 || Argc > 2) {       /* incorrect no of arguments - error */
      item_values[1].value = (uchar *) "ERROR";
      item_values[1].len = 5;
      break;
    }
    if (Argc == 2) {
      target_type = target_type | TARGET_SPARE;
    }
    /* allocate sufficient space for the spare string and 2 longs */
    if (target_buffer == NULL) {
      target_buffer = (uchar *) malloc(sizeof(uchar) * (Argv[0].strlength + 30));
      target_buffer_len = Argv[0].strlength + 30;
    } else {
      if (target_buffer_len < Argv[0].strlength + 30) {
        target_buffer = (uchar *) realloc(target_buffer, sizeof(uchar) * (Argv[0].strlength + 30));
        target_buffer_len = Argv[0].strlength + 30;
      }
    }
    if (target_buffer == (uchar *) NULL) {
      item_values[1].value = (uchar *) "ERROR";
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
    rc = validate_target(target_buffer, &target, target_type, true_line, FALSE, FALSE);
    if (rc == RC_TARGET_NOT_FOUND) {
      item_values[1].value = (uchar *) "NOTFOUND";
      item_values[1].len = 8;
      free_target(&target);
      break;
    }
    if (rc != RC_OK) {
      item_values[1].value = (uchar *) "ERROR";
      item_values[1].len = 5;
      free_target(&target);
      break;
    }
    if (Argc == 2 && target.spare != (-1)) {
      sprintf((char *) target_buffer, "%ld %ld %s", target.true_line, target.num_lines, target.rt[target.spare].string);
    } else {
      sprintf((char *) target_buffer, "%ld %ld", target.true_line, target.num_lines);
    }
    item_values[1].value = target_buffer;
    item_values[1].len = strlen((char *) target_buffer);
    free_target(&target);
    break;
  }
  item_values[0].value = (uchar *) "1";
  item_values[0].len = 1;
  return (rc);
}

static short run_os_function(ULONG Argc, RXSTRING Argv[]) {
  int rc = 0;
  static uchar num0[15];        /* large enough for 1000+rc */
  uchar *cmd = NULL, *instem = NULL, *outstem = NULL, *errstem = NULL;

  switch (Argc) {
    case 0:
      sprintf((char *) num0, "%d", RC_INVALID_OPERAND + 1000);
      break;
    case 4:
      if ((errstem = (uchar *) MakeAscii(&Argv[3])) == NULL) {
        sprintf((char *) num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }
    case 3:
      if ((outstem = (uchar *) MakeAscii(&Argv[2])) == NULL) {
        sprintf((char *) num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }
    case 2:
      if ((instem = (uchar *) MakeAscii(&Argv[1])) == NULL) {
        sprintf((char *) num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }
    case 1:
      if ((cmd = (uchar *) MakeAscii(&Argv[0])) == NULL) {
        sprintf((char *) num0, "%d", RC_OUT_OF_MEMORY + 1000);
        break;
      }
      rc = run_os_command(cmd, instem, outstem, errstem);
      sprintf((char *) num0, "%d", rc);
      break;
    default:
      sprintf((char *) num0, "%d", RC_INVALID_OPERAND + 1000);
      break;
  }
  item_values[1].value = num0;
  item_values[1].len = strlen((char *) num0);
  item_values[0].value = (uchar *) "1";
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

static int run_os_command(uchar *cmd, uchar *instem, uchar *outstem, uchar *errstem) {
  RXSTRING tmpstr;
  bool in = TRUE, out = TRUE, err = TRUE;
  bool out_and_err_same = FALSE;
  bool redirect_std_files;
  int inlen = 0, outlen = 0, errlen = 0;
  int i = 0;
  FILE *infp = NULL;
  char *infile = "", *outfile = "", *errfile = "";
  long innum = 0L;
  int rc = 0, rcode = 0;
  int save_stdin = 0, save_stdout = 0, save_stderr = 0;
  int infd = 0, outfd = 0, errfd = 0;

  redirect_std_files = TRUE;
  /*
   * Determine if we are redirecting stdin, stdout or both and
   * if the * values passed as stem variables end in '.'.
   */
  if (instem == NULL || strcmp((char *) instem, "") == 0) {
    in = FALSE;
  } else {
    inlen = strlen((char *) instem);
    if (*(instem + inlen - 1) == '.') {
      *(instem + inlen - 1) = '\0';
    } else {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  if (outstem == NULL || strcmp((char *) outstem, "") == 0) {
    out = FALSE;
  } else {
    outlen = strlen((char *) outstem);
    if (*(outstem + outlen - 1) == '.') {
      *(outstem + outlen - 1) = '\0';
    } else {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  if (errstem == NULL || strcmp((char *) errstem, "") == 0) {
    err = FALSE;
  } else {
    errlen = strlen((char *) errstem);
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
    if (out && strcmp((char *) instem, (char *) outstem) == 0) {
      return (RC_INVALID_OPERAND + 1000);
    }
    if (err && strcmp((char *) instem, (char *) errstem) == 0) {
      return (RC_INVALID_OPERAND + 1000);
    }
  }
  /*
   * An extra check here to determine if stdout and stderr are to be redirected to the same place.
   */
  if (out && err) {
    if (strcmp((char *) outstem, (char *) errstem) == 0) {
      out_and_err_same = TRUE;
    }
  }
  /*
   * If redirecting stdin, get the value of instem.0 to determine how many variables to get...
   */
  if (in) {
    tmpstr.strptr = NULL;
    (void) get_compound_rexx_variable(instem, &tmpstr, 0);
    if (tmpstr.strptr == NULL) {
      return (RC_SYSTEM_ERROR + 1000);
    }
    if (!valid_positive_integer((uchar *) tmpstr.strptr)) {
      return (RC_INVALID_OPERAND + 1000);
    }
    innum = atol((char *) tmpstr.strptr);
    free(tmpstr.strptr);
    /*
     * Write the contents of the stdin stem to a temporary file...
     */
    if ((infile = tmpname("IN")) == NULL) {
      return (RC_OUT_OF_MEMORY + 1000);
    }
    if ((infp = fopen(infile, "w")) == NULL) {
      return (RC_ACCESS_DENIED + 1000);
    }
    for (i = 0; i < innum; i++) {
      tmpstr.strptr = NULL;
      (void) get_compound_rexx_variable(instem, &tmpstr, (short) (i + 1));
      if (tmpstr.strptr == NULL) {
        return (RC_SYSTEM_ERROR + 1000);
      }
      fputs((char *) tmpstr.strptr, infp);
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
    if ((outfile = tmpname("OUT")) == NULL) {
      return (RC_OUT_OF_MEMORY + 1000);
    }
  }
  /*
   * If redirecting stderr, create the name of a temporary file for the output.
   */
  if (err) {
    if (out_and_err_same) {
      errfile = outfile;
    } else {
      if ((errfile = tmpname("ERR")) == NULL) {
        return (RC_OUT_OF_MEMORY + 1000);
      }
    }
  }

  if (redirect_std_files) {
    /*
     * Save file ids for stdin, stdout and stderr, then reassign them to the appropriate temporary files.
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
        if ((errfd = open(errfile, O_RDWR | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD)) == (-1)) {
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
  }
  /*
   * Execute the OS command supplied.
   */
  rcode = system((char *) cmd);
  if (rcode) {
    rcode = WEXITSTATUS(rcode);
  }
  if (redirect_std_files) {
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
  }
  /*
   * If redirecting stdout, we now have to read the file and set a REXX variable for each line read.
   */
  if (out) {
    rc = set_rexx_variables_from_file(outfile, outstem);
  } else {
    rc = set_rexx_variable(outstem, (uchar *) "0", 1, 0);
  }
  if (err) {
    if (!out_and_err_same) {
      rc = set_rexx_variables_from_file(errfile, errstem);
    }
  } else {
    rc = set_rexx_variable(errstem, (uchar *) "0", 1, 0);
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

static uchar *MakeAscii(RXSTRING *rxstring) {
  uchar *string = NULL;

  string = (uchar *) malloc((sizeof(uchar) * rxstring->strlength) + 1);
  if (string != NULL) {
    memcpy(string, rxstring->strptr, rxstring->strlength);
    *(string + (rxstring->strlength)) = '\0';
  }
  return (string);
}

static char *get_a_line(FILE *fp, char *string, int *length, int *rcode) {
  int ch;
  static int bufs = 1;
  int i = 0;

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

static short set_rexx_variables_from_file(char *filename, uchar *stem) {
  FILE *fp = NULL;
  int i = 0;
  char *string = NULL;
  int length = 0, rcode = 0, rc = 0;
  char tmpnum[15];

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
    rc = set_rexx_variable(stem, (uchar *) string, strlen(string), i + 1);
    free(string);
    if (rcode == RC_TOF_EOF_REACHED) {
      break;
    }
  }
  sprintf(tmpnum, "%d", i);
  rc = set_rexx_variable(stem, (uchar *) tmpnum, strlen(tmpnum), 0);
  if (fclose(fp)) {
    rc = RC_ACCESS_DENIED;
  }
  return (rc);
}

