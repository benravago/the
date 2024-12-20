// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

uchar *last_message = NULL;     /* contents of last error message */
int last_message_length = 0;
static int errors_displayed = 0;        /* number of errors displayed */
static LINE *first_error = NULL;        /* first error message */
static LINE *last_error = NULL; /* last error message */

static uchar *error_message[] = {
  (uchar *) "",
  (uchar *) "Error 0001: Invalid operand:",
  (uchar *) "Error 0002: Too many operands",
  (uchar *) "Error 0003: Too few operands",
  (uchar *) "Error 0004: Invalid number:",
  (uchar *) "Error 0005: Numeric operand too small",
  (uchar *) "Error 0006: Numeric operand too large",
  (uchar *) "Error 0007: Invalid fileid:",
  (uchar *) "Error 0008: Invalid or protected file",
  (uchar *) "Error 0009: File not found",
  (uchar *) "Error 0010: Path not found",
  (uchar *) "Error 0011: File not found in THE_MACRO_PATH:",
  (uchar *) "Error 0012: Margins settings are inconsistent",
  (uchar *) "Error 0013: Invalid key name:",
  (uchar *) "File is read-only:",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0017: Target not found",
  (uchar *) "Error 0018: Invalid line name",
  (uchar *) "",
  (uchar *) "New file:",
  (uchar *) "Error 0021: Invalid command:",
  (uchar *) "Error 0022: File has been changed - use QQUIT to really quit",
  (uchar *) "Error 0023: Help file not found:",
  (uchar *) "Error 0024: Invalid command while running in batch:",
  (uchar *) "Error 0025: Error accessing REXX variable",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0029: Cannot edit -",
  (uchar *) "Error 0030: Memory shortage",
  (uchar *) "Error 0031: File already exists - use FFILE/SSAVE",
  (uchar *) "Error 0032: Invalid hexadecimal or decimal value:",
  (uchar *) "",
  (uchar *) "Error 0034: Line not found",
  (uchar *) "",
  (uchar *) "Error 0036: No lines changed",
  (uchar *) "Error 0037: Operand too long:",
  (uchar *) "Error 0038: Improper cursor position",
  (uchar *) "Error 0039: No remembered operand available",
  (uchar *) "Error 0040: /bin/sh cannot suspend this process",
  (uchar *) "Error 0041: Invalid SOS command:",
  (uchar *) "Error 0042: Invalid SET command:",
  (uchar *) "",
  (uchar *) "Error 0044: No marked block",
  (uchar *) "Error 0045: Marked block not in current file",
  (uchar *) "Error 0046: Block boundary excluded, not in range, or past truncation column",
  (uchar *) "Error 0047: Operation invalid for line blocks",
  (uchar *) "Error 0048: Operation invalid for box blocks",
  (uchar *) "Error 0049: Operation invalid for stream blocks",
  (uchar *) "Error 0050: Invalid move location",
  (uchar *) "Error 0051: No preserved settings to restore",
  (uchar *) "Error 0052: Non-REXX macros MUST have a first line of /*NOREXX*/",
  (uchar *) "Error 0053: Valid only when issued from a REXX macro",
  (uchar *) "Error 0054: REXX interpreter returned an error",
  (uchar *) "Error 0055: No lines sorted",
  (uchar *) "Error 0056: Action invalid in read-only mode.",
  (uchar *) "Error 0057: Disk full error",
  (uchar *) "Error 0058: Valid only with REXX support:",
  (uchar *) "",
  (uchar *) "Error 0060: Line name not found:",
  (uchar *) "Error 0061: Colour support not available:",
  (uchar *) "Error 0062: Operation invalid for multi-line stream blocks",
  (uchar *) "Error 0063: Invalid cursor line or column",
  (uchar *) "Error 0064: Line not reserved",
  (uchar *) "",
  (uchar *) "Error 0066: Invalid match position",
  (uchar *) "Error 0067: Invalid match character",
  (uchar *) "Error 0068: Matching character not found",
  (uchar *) "Error 0069: Invalid character",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0075: Too many sort fields - maximum is 1000",
  (uchar *) "Error 0076: Fileid already in ring:",
  (uchar *) "Error 0077: Files still open in batch:",
  (uchar *) "Error 0078: Printing error:",
  (uchar *) "Error 0079: Can't add another tab position; already have 32 defined",
  (uchar *) "Error 0080: Can't add another CTLCHAR; already have 64 defined",
  (uchar *) "Error 0081: Only single-line marked blocks allowed",
  (uchar *) "Error 0082: Feature not supported:",
  (uchar *) "Error 0083: Command invalid when ring is empty",
  (uchar *) "Unable to restore",
  (uchar *) "Error 0085: Length of operand > 10",
  (uchar *) "Error 0086: Command line unavailable",
  (uchar *) "Error 0087: Cursor line not in scope",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0138: Unexpected file time stamp change - use FFILE/SSAVE",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0148: Already recording a macro",
  (uchar *) "Error 0149: Operand in stem operation requires trailing period:",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0154: JOIN would cause truncation",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0186: Error accessing clipboard",
  (uchar *) "Error 0187: No text in clipboard",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0199: Parser not defined:",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "",
  (uchar *) "Error 0216: TLD error:",
};

static void open_msgline(ushort, ushort, ushort);

int display_error(unsigned short err_num, uchar *mess, bool ignore_bell) {
  uchar *last_cmd_name = NULL;
  bool set_command = FALSE, sos_command = FALSE;
  int new_last_message_length = 0;
  int x = 0, y = 0, ee_len = 0;
  int rc = RC_OK;

  if (curses_started && CURRENT_VIEW != NULL && CURRENT_WINDOW != NULL) {
    getyx(CURRENT_WINDOW, y, x);
  }
  if (ERRORFORMATx == 'E') {
    last_cmd_name = get_command_name(last_command_index, &set_command, &sos_command);
    if (last_cmd_name != NULL) {
      ee_len = 2 + strlen((char *) last_cmd_name) + ((set_command) ? 4 : 0) + ((sos_command) ? 4 : 0);
    }
  }
  /*
   * Always save message text, even if MSGMODE is OFF...
   * If no error number, display text only...
   */
  new_last_message_length = 2 + ((err_num == 0) ? strlen((char *) mess) : strlen((char *) mess) + strlen((char *) error_message[err_num]) + 1) + ee_len;
  if (last_message == NULL) {
    last_message_length = new_last_message_length;
    last_message = (uchar *) malloc(last_message_length * sizeof(uchar));
    if (last_message == NULL) {
      return rc;
    }
  } else {
    if (new_last_message_length > last_message_length) {
      last_message_length = new_last_message_length;
      last_message = (uchar *) realloc(last_message, last_message_length * sizeof(uchar));
      if (last_message == NULL) {
        return rc;
      }
    }
  }
  if (err_num == 0) {
    strcpy((char *) last_message, (char *) mess);
  } else {
    if (last_cmd_name == NULL) {
      sprintf((char *) last_message, "%s %s", error_message[err_num], mess);
    } else {
      if (set_command || sos_command) {
        sprintf((char *) last_message, "%s %s: %s %s", (set_command) ? "set" : "sos", last_cmd_name, error_message[err_num], mess);
      } else {
        sprintf((char *) last_message, "%s: %s %s", last_cmd_name, error_message[err_num], mess);
      }
    }
  }
  /*
   * If msgmode is off, don't display any errors.
   */
  if (CURRENT_VIEW != NULL) {
    if (!CURRENT_VIEW->msgmode_status) {
      return rc;
    }
  }
  /*
   * If running from NOMSG command, don't display any errors.
   */
  if (in_nomsg) {
    return rc;
  }
  if (!curses_started) {
    if (!error_on_screen && !be_quiet) {
      if (number_of_files == 0) {
        fprintf(stderr, "*** Messages from profile file ***\n");
      } else {
        fprintf(stderr, "*** Messages from profile file for %s%s ***\n", CURRENT_FILE->fpath, CURRENT_FILE->fname);
      }
    }
    error_on_screen = TRUE;
    fprintf(stderr, "%s\n", last_message);
    return rc;
  }
  /*
   * If SET ERROROUTPUT is ON, write the error message to stderr
   */
  if (ERROROUTPUTx) {
    fprintf(stderr, "%s\n", last_message);
  }
  /*
   * Append the current message to the end of the error linked list.
   */
  last_error = lll_add(first_error, last_error, sizeof(LINE));
  if (last_error == NULL) {
    return rc;
  }
  last_error->line = (uchar *) malloc((strlen((char *) last_message) + 1) * sizeof(uchar));
  if (last_error->line == NULL) {
    return rc;
  }
  strcpy((char *) last_error->line, (char *) last_message);
  last_error->length = strlen((char *) last_message);
  if (first_error == NULL) {
    first_error = last_error;
  }
  errors_displayed++;
  rc = expose_msgline();
  /*
   * If capturing REXX output, then add a new line to the pseudo file.
   */
  if (CAPREXXOUTx && rexx_output) {
    rexxout_number_lines++;
    rexxout_curr = add_LINE(rexxout_first_line, rexxout_curr, last_message, strlen((char *) last_message), 0, FALSE);
  }
  if (BEEPx && !ignore_bell) {
    beep();
  }
  if (curses_started && CURRENT_VIEW != NULL && CURRENT_WINDOW != NULL) {
    wmove(CURRENT_WINDOW, y, x);
  }
  if (first_screen_display) {
    wrefresh(error_window);
  }
  return rc;
}

static void open_msgline(ushort base, ushort off, ushort rows) {
  int start_row = 0;
  COLOUR_ATTR attr;

  if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL) {
    set_up_default_colours((FILE_DETAILS *) NULL, &attr, ATTR_MSGLINE);
  } else {
    memcpy(&attr, CURRENT_FILE->attr + ATTR_MSGLINE, sizeof(COLOUR_ATTR));
  }
  start_row = calculate_actual_row(base, off, CURRENT_SCREEN.screen_rows, TRUE);
  if (base == POSITION_BOTTOM) {
    start_row = start_row - rows + 1;
  }
  if (error_window != NULL) {
    delwin(error_window);
  }
  error_window = newwin(rows, CURRENT_SCREEN.screen_cols, CURRENT_SCREEN.screen_start_row + start_row, CURRENT_SCREEN.screen_start_col);
  wattrset(error_window, set_colour(&attr));
  keypad(error_window, TRUE);
  return;
}

void clear_msgline(int key) {
  /*
   * Only clear the message line if the supplied key matches that set by SET CLEARERRORKEY.
   * -1 indicates any key can clear
   */
  if (curses_started && (key == CLEARERRORKEYx || CLEARERRORKEYx == -1)) {
    errors_displayed = 0;
    error_on_screen = FALSE;
    if (error_window != (WINDOW *) NULL) {
      delwin(error_window);
      error_window = (WINDOW *) NULL;
    }
    first_error = last_error = lll_free(first_error);
    if (display_screens > 1) {
      redraw_screen((uchar) (other_screen));
    }
    redraw_screen(current_screen);
    doupdate();
  }
  return;
}
void display_prompt(uchar *prompt) {
  open_msgline(CURRENT_VIEW->msgline_base, CURRENT_VIEW->msgline_off, 1);
  wmove(error_window, 0, 0);
  my_wclrtoeol(error_window);
  put_string(error_window, 0, 0, prompt, strlen((char *) prompt));
  wrefresh(error_window);
  error_on_screen = TRUE;
  return;
}

#define NORMAL_PROMPT "Press any key to continue..."
#define IN_MACRO_PROMPT "Press SPACE to terminate macro or any other key to continue..."

int expose_msgline(void) {
  LINE *curr_error = NULL;
  register int i = 0;
  ushort errors_to_display = 0;
  uchar msgline_base = POSITION_TOP;
  short msgline_off = 2;
  ushort msgline_rows = 5, max_rows, start_row;
  int rc = RC_OK;
  uchar *prompt;

  /*
   * If msgmode is off, don't display any errors.
   */
  if (CURRENT_VIEW != NULL) {
    if (!CURRENT_VIEW->msgmode_status) {
      return rc;
    }
    msgline_rows = CURRENT_VIEW->msgline_rows;
    msgline_base = CURRENT_VIEW->msgline_base;
    msgline_off = CURRENT_VIEW->msgline_off;
  }
  curr_error = last_error;
  /*
   * Determine the maximum number of rows we can have in the MSGLINE
   */
  start_row = calculate_actual_row(msgline_base, msgline_off, CURRENT_SCREEN.screen_rows, TRUE);
  if (msgline_base == POSITION_BOTTOM) {
    max_rows = start_row + 1;
  } else {
    max_rows = CURRENT_SCREEN.screen_rows - start_row;
  }
  /*
   * Calculate number of errors. This determines size of window to be created.
   */
  if (msgline_rows == 0) {
    msgline_rows = min(max_rows, errors_displayed);
  } else {
    msgline_rows = min(msgline_rows, errors_displayed);
  }
  errors_to_display = min(max_rows, msgline_rows);
  /*
   * Create the window; errors_to_display rows long.
   */
  open_msgline(msgline_base, msgline_off, errors_to_display);
  /*
   * For all errors that are to be displayed, display them starting from the bottom of the window.
   */
  for (i = errors_to_display - 1; i > -1; i--) {
    wmove(error_window, i, 0);
    my_wclrtoeol(error_window);
    if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL) {
      mvwaddstr(error_window, i, 0, (char *) curr_error->line);
    } else {
      put_string(error_window, (ushort) i, 0, curr_error->line, curr_error->length);
    }
    curr_error = curr_error->prev;
  }
  wnoutrefresh(error_window);
  error_on_screen = TRUE;
  if (errors_to_display && errors_to_display == msgline_rows && (errors_displayed % errors_to_display) == 1 && curr_error != NULL) {
    if (in_macro) {
      prompt = (uchar *) IN_MACRO_PROMPT;
    } else {
      prompt = (uchar *) NORMAL_PROMPT;
    }
    wmove(error_window, msgline_rows - 1, 0);
    my_wclrtoeol(error_window);
    if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL) {
      mvwaddstr(error_window, msgline_rows - 1, 0, (char *) prompt);
    } else {
      put_string(error_window, (ushort) (msgline_rows - 1), 0, (uchar *) prompt, strlen((char *) prompt));
    }
    wrefresh(error_window);
    if (my_getch(error_window) == ' ') {
      rc = RC_TERMINATE_MACRO;
    }
  }
  return rc;
}
