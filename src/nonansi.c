// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * This file contains all calls to non-ansi conforming routines.
 */

#include "the.h"
#include "proto.h"

#include <pwd.h>
#include <errno.h>

/*
 * Replace non-ANSI defs with ANSI ones
 */

short file_readable(char_t * filename) {
  if (access((char *) filename, R_OK) == (-1)) {
    return (FALSE);
  }
  return (TRUE);
}

short file_writable(char_t * filename) {
  if (file_exists(filename) != THE_FILE_EXISTS) {
    return (TRUE);
  }
  if (access((char *) filename, W_OK) == (-1)) {
    return (FALSE);
  }
  return (TRUE);
}

short file_exists(char_t * filename) {
  int rc;

  if (access((char *) filename, F_OK) == (-1)) {
    rc = errno;
    switch (rc) {

      case ENAMETOOLONG:
        rc = THE_FILE_NAME_TOO_LONG;
        break;

      default:
        rc = THE_FILE_UNKNOWN;
        break;
    }
    return ((short) rc);
  } else {
    return (THE_FILE_EXISTS);
  }
}

short remove_file(char_t * filename) {
  if (filename == NULL) {
    return (RC_OK);
  }
  if (unlink((char *) filename) == (-1)) {
    return (RC_ACCESS_DENIED);
  }
  return (RC_OK);
}

void convert_equals_in_filename(char_t * outfilename, char_t * infilename) {
  /*
   * Support an = in the following circumstances:
   *  In filename      Current           Current         Substitutes
   *                   \oldp\fred.c      \oldp\fred
   * ---------------------------------------------------------------
   * 1) =\abc.def      \oldp\abc.def     \oldp\abc.def     fpath
   * 2) \apath\=.x     \apath\fred.x     \apath\fred.x     fname
   * 3) \apath\abc.=   \apath\abc.c      \apath\abc.       ftype
   * 4) =.x            \oldp\fred.x      \oldp\fred.x      fpath and fname
   * 5) abc.=          abc.c             abc.              ftype
   * 6) =              \oldp\fred.c      \oldp\fred        fpath, fname and ftype
   * 7) \apath\=       \apath\fred.c     \apath\fred       filename
   */

  char_t in_filename[MAX_FILE_NAME + 1];
  char_t current_filename[MAX_FILE_NAME + 1];
  char_t *in_ftype, *in_fpath, *in_fname;
  char_t *current_ftype, *current_fpath, *current_fname;
  line_t last_pos;

  /*
   * If we don't have a current file, or there are no equivalence chars
   * just copy the incoming filename to the outgoing filename...
   */
  if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL || strzreveq(infilename, (char_t) EQUIVCHARx) == (-1)) {
    strcpy((char *) outfilename, (char *) infilename);
    return;
  }
  /*
   * Split the incoming file name into 2 or 3 pieces; fpath/filename or
   * fpath/fname/ftype.
   */
  strcpy((char *) in_filename, (char *) strrmdup(strtrans(infilename, OSLASH, ISLASH), EQUIVCHARx, TRUE));
  in_fpath = in_filename;
  last_pos = strzreveq(in_fpath, (char_t) ISLASH);
  if (last_pos == (-1)) {
    in_fpath = NULL;
    in_fname = in_filename;
  } else {
    in_fpath[last_pos] = '\0';
    in_fname = in_fpath + last_pos + 1;
  }
  last_pos = strzreveq(in_fname, (char_t) '.');
  if (last_pos == (-1)) {
    in_ftype = NULL;
  } else {
    in_ftype = in_fname + last_pos + 1;
    in_fname[last_pos] = '\0';
  }
  /*
   * Split the current filename and path into its component parts
   */
  strcpy((char *) current_filename, (char *) CURRENT_FILE->fpath);
  strcat((char *) current_filename, (char *) CURRENT_FILE->fname);
  current_fpath = current_filename;
  last_pos = strzreveq(current_fpath, (char_t) ISLASH);
  if (last_pos == (-1)) {
    current_fpath = NULL;
    current_fname = current_filename;
  } else {
    current_fpath[last_pos] = '\0';
    current_fname = current_fpath + last_pos + 1;
  }
  last_pos = strzreveq(current_fname, (char_t) '.');
  if (last_pos == (-1)) {
    current_ftype = NULL;
  } else {
    current_ftype = current_fname + last_pos + 1;
    current_fname[last_pos] = '\0';
  }
  /*
   * Now its time to put the new file name together
   */
  strcpy((char *) outfilename, "");
  if (in_fpath && !equal(in_fpath, EQUIVCHARstr, 1)) {
    strcat((char *) outfilename, (char *) in_fpath);
  } else {
    strcat((char *) outfilename, (char *) current_fpath);
  }
  strcat((char *) outfilename, (char *) ISTR_SLASH);
  if (in_fname && !equal(in_fname, EQUIVCHARstr, 1)) {
    strcat((char *) outfilename, (char *) in_fname);
  } else {
    strcat((char *) outfilename, (char *) current_fname);
  }
  if (in_ftype && !equal(in_ftype, EQUIVCHARstr, 1)) {
    strcat((char *) outfilename, ".");
    strcat((char *) outfilename, (char *) in_ftype);
  } else {
    if (current_ftype) {
      strcat((char *) outfilename, ".");
      strcat((char *) outfilename, (char *) current_ftype);
    }
  }
  return;
}

short splitpath(char_t * filename) {
  short len = 0;
  char_t work_filename[MAX_FILE_NAME + 1];
  char_t conv_filename[MAX_FILE_NAME + 1];

  if (strlen((char *) filename) > MAX_FILE_NAME) {
    return (RC_BAD_FILEID);
  }
  /*
   * Save the current directory.
   */
  if (getcwd((char *) curr_path, MAX_FILE_NAME) == NULL) {
    return (RC_BAD_FILEID);
  }
  strcpy((char *) sp_path, "");
  strcpy((char *) sp_fname, "");
  convert_equals_in_filename(conv_filename, filename);
  if (strlen((char *) conv_filename) > MAX_FILE_NAME) {
    return (RC_BAD_FILEID);
  }
  strcpy((char *) work_filename, (char *) conv_filename);
  /*
   * If the supplied filename is empty, set the path = cwd and filename
   * equal to blank.
   */
  if (strcmp((char *) filename, "") == 0) {
    if (getcwd((char *) sp_path, MAX_FILE_NAME) == NULL) {
      return (RC_BAD_FILEID);
    }
    strcpy((char *) sp_fname, "");
  }
  /*
   * Check if the first character is tilde; translate HOME env variable
   * if there is one. Obviously only applicable to UNIX.
   */
  if (*(conv_filename) == '~') {
    if (*(conv_filename + 1) == ISLASH || *(conv_filename + 1) == '\0') {
      char_t *home = (char_t *) getenv("HOME");

      if (((home == NULL) ? 0 : strlen((char *) home)) + strlen((char *) conv_filename) > MAX_FILE_NAME) {
        return (RC_BAD_FILEID);
      }
      strcpy((char *) work_filename, (char *) home);
      strcat((char *) work_filename, (char *) (conv_filename + 1));
    } else {
      struct passwd *pwd;

      strcpy((char *) sp_path, (char *) conv_filename + 1);
      if ((len = strzeq(sp_path, ISLASH)) != (-1)) {
        sp_path[len] = '\0';
      }
      if ((pwd = getpwnam((char *) sp_path)) == NULL) {
        return (RC_BAD_FILEID);
      }
      strcpy((char *) work_filename, pwd->pw_dir);
      if (len != (-1)) {
        strcat((char *) work_filename, (char *) (conv_filename + 1 + len));
      }
    }
  }
  /*
   * First determine if the supplied filename is a directory.
   */
  if ((stat((char *) work_filename, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode)) {
    strcpy((char *) sp_path, (char *) work_filename);
    strcpy((char *) sp_fname, "");
  } else {                      /* here if the file doesn't exist or is not a directory */
    len = strzreveq(work_filename, ISLASH);
    switch (len) {

      case (-1):
        if (getcwd((char *) sp_path, MAX_FILE_NAME) == NULL) {
          return (RC_BAD_FILEID);
        }
        strcpy((char *) sp_fname, (char *) work_filename);
        break;

      case 0:
        strcpy((char *) sp_path, (char *) work_filename);
        sp_path[1] = '\0';
        strcpy((char *) sp_fname, (char *) work_filename + 1 + len);
        break;

      default:
        strcpy((char *) sp_path, (char *) work_filename);
        sp_path[len] = '\0';
        strcpy((char *) sp_fname, (char *) work_filename + 1 + len);
        break;
    }
  }
  /*
   * Change directory to the supplied path, if possible and store the expanded path.
   * If an error, restore the current path.
   */
  if (chdir((char *) sp_path) != 0) {
    chdir((char *) curr_path);
    return (RC_FILE_NOT_FOUND);
  }
  if (getcwd((char *) sp_path, MAX_FILE_NAME) == NULL) {
    return (RC_BAD_FILEID);
  }
  chdir((char *) curr_path);
  /*
   * Append the OS directory character to the path if it doesn't already end in the character.
   */
  len = strlen((char *) sp_path);
  if (len > 0 && sp_path[len - 1] != ISLASH) {
    strcat((char *) sp_path, (char *) ISTR_SLASH);
  }
  return (RC_OK);
}

/*
 * Function  : Reads the contents of the clipboard into the file.
 * Parameters: pointer to line after which lines are to be added
 * Return    : 0 on success.
 */
LINE *getclipboard(LINE * now, int from_get) {
  LINE *curr = now;
  /* !!! seems to be only for PDCurses */
  display_error(82, (char_t *) "CLIP:", FALSE);
  curr = NULL;
  return curr;
}

#define CLIP_TYPE_LINE   1
#define CLIP_TYPE_BOX    2
#define CLIP_TYPE_STREAM 3

short setclipboard(FILE_DETAILS * cf, char_t * new_fname, bool force, line_t in_lines, line_t start_line_in, line_t end_line_in, line_t * num_file_lines, bool append, length_t start_col_in, length_t end_col_in, bool ignore_scope, bool lines_based_on_scope, int target_type) {
  line_t i = 0L;
  line_t abs_num_lines = (in_lines < 0L ? -in_lines : in_lines);
  line_t num_actual_lines = 0L;
  line_t my_num_file_lines = 0L;
  line_t current_line;
  short direction = (in_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  LINE *curr = NULL;
  short rc = RC_OK;
  bool save_scope_all = CURRENT_VIEW->scope_all;
  char_t *eol = (char_t *) "\n";
  int eol_len = 1;
  long clip_size = 1024;
  char_t *ptr = NULL;
  long len = 0, pos = 0;
  int clip_type;
  length_t start_col = start_col_in, end_col = end_col_in;
  line_t start_line = start_line_in, end_line = end_line_in;

  /*
   * CUA block can have start position after end position, so fix this up so
   * start position is less than end position
   */
  if (MARK_VIEW && MARK_VIEW->mark_type == M_CUA && (start_line * max_line_length) + start_col > (end_line * max_line_length) + end_col) {
    start_line = end_line_in;
    start_col = end_col_in;
    end_line = start_line_in;
    end_col = start_col_in;
    /*
     * Now switch the "in" values...
     */
    start_line_in = start_line;
    end_line_in = end_line;
    start_col_in = start_col;
    end_col_in = end_col;
    /*
     * Fix lines and direction
     */
    direction = DIRECTION_FORWARD;
    in_lines = abs_num_lines;
  }
  /*
   * Determine where to start writing from in the linked list.
   */
  curr = lll_find(cf->first_line, cf->last_line, start_line, cf->number_lines);
  /*
   * Save the setting of scope_all if we are ignoring scope. ie full file
   * is being written...
   */
  if (ignore_scope) {
    CURRENT_VIEW->scope_all = TRUE;
  }
  /*
   * Allocate an initial amount of memory for the clipboard buffer.
   * It will be increased later if required.
   */
  ptr = (char_t *) malloc(clip_size);
  if (!ptr) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }

  rc = RC_OK;
  /*
   * We need to work out which part of the line needs to
   * be copied to the clipboard buffer, depending on what
   * type of target has been selected.
   * Effectively there are 3 different combinations of
   * lines to put in the clipboard:
   * 1) full lines - every line is the complete line
   *    BLOCK target with M_LINE type
   *    all other non-block targets
   * 2) boxes - every line is the same partial size
   *    M_BOX, M_WORD and M_COLUMN BLOCK target types
   *    plus M_STREAM and M_CUA when start_line = end_line
   * 3) streams - irregular line lengths
   *    first line starts at start_col goes to end of line
   *    last line starts at 0 and goes to end_col
   *    all other lines are complete lines
   */
  if (target_type == TARGET_BLOCK_CURRENT) {
    if (MARK_VIEW->mark_type == M_LINE) {
      clip_type = CLIP_TYPE_LINE;
    } else if (MARK_VIEW->mark_type == M_BOX || MARK_VIEW->mark_type == M_WORD || MARK_VIEW->mark_type == M_COLUMN) {
      clip_type = CLIP_TYPE_BOX;
    } else if (MARK_VIEW->mark_type == M_CUA && start_line == end_line) {
      clip_type = CLIP_TYPE_BOX;
    } else if (MARK_VIEW->mark_type == M_STREAM && start_line == end_line) {
      clip_type = CLIP_TYPE_BOX;
    } else {
      clip_type = CLIP_TYPE_STREAM;
    }
  } else {
    clip_type = CLIP_TYPE_LINE;
  }
  /*
   * Processe each line...
   */
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines) {
        break;
      }
    } else {
      if (abs_num_lines == i) {
        break;
      }
    }
    current_line = start_line + (line_t) (i * direction);
    rc = processable_line(CURRENT_VIEW, current_line, curr);
    switch (rc) {

      case LINE_SHADOW:
        rc = 0;
        break;

      case LINE_TOF:
      case LINE_EOF:
        rc = 0;
        num_actual_lines++;
        break;

      default:
        rc = 0;
        /*
         * Depending on the type of line we need to work out
         * which columns to copy...
         */
        switch (clip_type) {

          case CLIP_TYPE_LINE:
            break;

          case CLIP_TYPE_BOX:
            break;

          case CLIP_TYPE_STREAM:
            if (current_line == start_line) {
              start_col = start_col_in;
              end_col = curr->length - 1;
            } else if (current_line == end_line) {
              start_col = 0;
              end_col = end_col_in;
            } else {
              start_col = 0;
              end_col = curr->length - 1;
            }
            break;
        }
        if (start_col < curr->length) {
          len = min(curr->length - start_col, (end_col - start_col) + 1);
          if (pos + len > clip_size) {
            ptr = (char_t *) realloc((char *) ptr, clip_size * 2);
            clip_size *= 2;
          }
          memcpy((char *) ptr + pos, (char *) curr->line + start_col, len);
          pos += len;
        }
        if (pos + 1 + eol_len > clip_size) {
          ptr = (char_t *) realloc((char *) ptr, clip_size * 2);
          if (!ptr) {
            display_error(30, (char_t *) "", FALSE);
            return (RC_OUT_OF_MEMORY);
          }
          clip_size *= 2;
        }
        memcpy((char *) ptr + pos, (char *) eol, eol_len);
        pos += eol_len;
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (rc) {
      break;
    }
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    my_num_file_lines += (line_t) direction;
    if (curr == NULL) {
      break;
    }
  }
  /*
   * Nul terminate the string.
   */
  *(ptr + pos) = '\0';
  /*
   * Restore the setting of scope_all if we changed it before...
   */
  if (ignore_scope) {
    CURRENT_VIEW->scope_all = save_scope_all;
  }
  display_error(82, (char_t *) "CLIP:", FALSE);
  rc = RC_INVALID_ENVIRON;

  free((void *) ptr);
  return (rc);
}

/*
 * These THE_curs_set() functions required as PDCurses has been changed so that curs_set(1) no longer
 * results in an underline cursor, it uses the cursor shape when the application
 * starts, so if you have a block cursor on starting THE, you would not be able to get
 * an underline cursor.
 */

void draw_cursor(bool visible) {
  if (visible) {
    if (INSERTMODEx) {
      curs_set(1);              /* First set to displayed... */
      curs_set(2);              /* ...then try to make it more visible */
    } else {
      curs_set(1);              /* underline cursor */
    }
  } else {
    curs_set(0);                /* cursor off */
  }
  return;
}

