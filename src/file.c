/* FILE.C - File and view related functions.                           */
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

#include <errno.h>

static short write_line(char_t *, length_t, FILE *, short);
static short write_char(char_t, FILE *);

LINE *dir_first_line = NULL;
LINE *dir_last_line = NULL;
line_t dir_number_lines = 0L;

static short process_file_attributes(int restore_attributes, FILE_DETAILS * cf, char_t * filename) {

  if (restore_attributes) {
    return RC_OK;
  } else {
    return RC_OK;
  }
}
short get_file(char_t * filename) {
  LINE *curr = NULL;
  char_t *work_filename;
  VIEW_DETAILS *save_current_view = NULL, *found_file = NULL;
  short rc = RC_OK;
  FILE_DETAILS *save_current_file = NULL;
  bool save_scope = FALSE;
  bool save_stay = FALSE;
  char_t pseudo_file = PSEUDO_REAL;
  length_t len, sp_path_len, sp_fname_len;

  /*
   * Split the filename supplied into directory and filename parts.
   * This is done before allocating a new current_file number.
   */
  if ((rc = splitpath(filename)) != RC_OK) {
    display_error(10, filename, FALSE);
    return (rc);
  }
  /*
   * If the filename portion of the splitpath is empty, then we are
   * editing a directory. So create the new file with the appropriate OS
   * command and set the filename to DIR.DIR.
   */
  if (strcmp((char *) sp_fname, "") == 0) {
    if ((rc = read_directory()) != RC_OK) {
      display_error(10, sp_path, FALSE);
      return (rc);
    }
    strcpy((char *) sp_path, (char *) dir_pathname);
    strcpy((char *) sp_fname, (char *) dir_filename);
  }
  /*
   * Determine if we are editing a PSEUDO file.  Set a temporary flag
   * here for REXXOUT and KEY files; a DIR.DIR file is determined below.
   */
  if (strcmp((char *) dir_filename, (char *) sp_fname) == 0)
    pseudo_file = PSEUDO_DIR;
  else if (strcmp((char *) rexxoutname, (char *) sp_fname) == 0)
    pseudo_file = PSEUDO_REXX;
  else if (strcmp((char *) keyfilename, (char *) sp_fname) == 0)
    pseudo_file = PSEUDO_KEY;
  /*
   * If this is the first file to be edited, don't check to see if the
   * file is already in the ring. Obvious hey!
   */
  if (CURRENT_VIEW == (VIEW_DETAILS *) NULL) {  /* no files in ring yet */
    if ((rc = defaults_for_first_file()) != RC_OK) {
      return (rc);
    }
  } else {
    /*
     * Here we should check if we already have the file to be edited in
     * the ring. If the file is there and it is DIR.DIR, QQUIT out of it
     * otherwise set the current pointer to it and exit.
     * Same applies to REXX output file.
     */
    save_current_view = CURRENT_VIEW;
    if ((found_file = find_file(sp_path, sp_fname)) != (VIEW_DETAILS *) NULL) {
      CURRENT_VIEW = found_file;
      if (CURRENT_FILE->pseudo_file == PSEUDO_DIR || CURRENT_FILE->pseudo_file == PSEUDO_REXX || CURRENT_FILE->pseudo_file == PSEUDO_KEY) {
        free_view_memory(FALSE, FALSE);
        if (CURRENT_VIEW == (VIEW_DETAILS *) NULL) {
          rc = defaults_for_first_file();
          /* FGC: if we don't execute the following line, it may
             crash, try "the -n", then "scr 2 v" and "dir /"
             BULLSHIT: won't work, we should try it in another way!
           */
          save_current_view = CURRENT_VIEW;
        } else {
          save_current_file = CURRENT_FILE;
          rc = defaults_for_other_files(NULL);
        }
        if (rc != RC_OK) {
          /* FGC: fixme: problem: freed up view memory! */
          return (rc);
        }
      } else {
        SCREEN_VIEW(current_screen) = CURRENT_VIEW;
        CURRENT_VIEW->in_ring = TRUE;
        return (RC_OK);
      }
    } else {
      CURRENT_VIEW = save_current_view;
      save_current_file = CURRENT_FILE;
      if ((rc = defaults_for_other_files(NULL)) != RC_OK) {
        return (rc);
      }
    }
  }
  /*
   * Increment the number of files in storage here, so that if there are
   * any problems with reading the file, free_file_memory() function can
   * correctly decrement the number of files.
   */
  number_of_files++;
  /*
   * Allocate memory to file pointer.
   */
  if ((CURRENT_FILE = (FILE_DETAILS *) malloc(sizeof(FILE_DETAILS))) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Set readonly to default
   */
  CURRENT_FILE->readonly = READONLY_OFF;
  /*
   * Allocate space for file's colour attributes...
   */
  if ((CURRENT_FILE->attr = (COLOUR_ATTR *) malloc(ATTR_MAX * sizeof(COLOUR_ATTR))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  memset(CURRENT_FILE->attr, 0, ATTR_MAX * sizeof(COLOUR_ATTR));
  /*
   * Allocate space for file's ecolour attributes...
   */
  if ((CURRENT_FILE->ecolour = (COLOUR_ATTR *) malloc(ECOLOUR_MAX * sizeof(COLOUR_ATTR))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  memset(CURRENT_FILE->ecolour, 0, ECOLOUR_MAX * sizeof(COLOUR_ATTR));
  /*
   * Set up default file attributes.
   */
  default_file_attributes(save_current_file);
  CURRENT_FILE->pseudo_file = pseudo_file;
  /*
   * Get length of path and filename components
   */
  sp_path_len = strlen((char *) sp_path);
  sp_fname_len = strlen((char *) sp_fname);
  /*
   * Copy the filename and path strings split up at the start of the
   * function.
   */
  if ((CURRENT_FILE->fname = (char_t *) malloc(sp_fname_len + 1)) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  strcpy((char *) CURRENT_FILE->fname, (char *) sp_fname);

  if ((CURRENT_FILE->fpath = (char_t *) malloc(sp_path_len + 1)) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  strcpy((char *) CURRENT_FILE->fpath, (char *) sp_path);

  if ((CURRENT_FILE->actualfname = (char_t *) malloc(strlen((char *) filename) + 1)) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  strcpy((char *) CURRENT_FILE->actualfname, (char *) filename);
  if ((CURRENT_FILE->efileid = (char_t *) malloc(sp_path_len + sp_fname_len + 1)) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  strcpy((char *) CURRENT_FILE->efileid, (char *) sp_path);
  strcat((char *) CURRENT_FILE->efileid, (char *) sp_fname);
  /*
   * If we have a "real" file, generate the filename to be used in
   * AUTOSAVE.
   */
  if (!CURRENT_FILE->pseudo_file) {
    if ((CURRENT_FILE->autosave_fname = (char_t *) malloc(sp_path_len + sp_fname_len + 7)) == NULL) {
      free_view_memory(TRUE, TRUE);
      display_error(30, (char_t *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    new_filename(sp_path, sp_fname, CURRENT_FILE->autosave_fname, (char_t *) ".aus");
  }
  if ((work_filename = alloca(sp_path_len + sp_fname_len + 1)) == NULL) {
    free_view_memory(TRUE, TRUE);
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Save the path and filename in a working area.
   */
  strcpy((char *) work_filename, (char *) sp_path);
  strcat((char *) work_filename, (char *) sp_fname);
  /*
   * For PSEUDO files, point the first and last line pointers to the
   * already allocated LINE* linked list...
   */
  if (CURRENT_FILE->pseudo_file) {      /* if DIR.DIR or REXXOUT.$$$ or KEY$$$.$$$ */
    CURRENT_FILE->fmode = 0;
    CURRENT_FILE->modtime = 0;
    switch (CURRENT_FILE->pseudo_file) {
      case PSEUDO_DIR:
        CURRENT_FILE->first_line = dir_first_line;
        CURRENT_FILE->last_line = dir_last_line;
        CURRENT_FILE->number_lines = dir_number_lines;
        break;
      case PSEUDO_REXX:
        CURRENT_FILE->first_line = rexxout_first_line;
        CURRENT_FILE->last_line = rexxout_last_line;
        CURRENT_FILE->number_lines = rexxout_number_lines;
        break;
      case PSEUDO_KEY:
        CURRENT_FILE->first_line = key_first_line;
        CURRENT_FILE->last_line = key_last_line;
        CURRENT_FILE->number_lines = key_number_lines;
        break;
    }
    /*
     * Allow syntax highlighting for pseudo files
     */
    if (CURRENT_FILE->colouring)
      find_auto_parser(CURRENT_FILE);
    return (RC_OK);
  }
  /*
   * Everything below here is relevant only to "real" files
   */
  rc = file_exists(work_filename);
  switch (rc) {
    case THE_FILE_EXISTS:
      if (file_writable(work_filename))
        CURRENT_FILE->disposition = FILE_NORMAL;
      else {
        if (file_readable(work_filename)) {
          CURRENT_FILE->disposition = FILE_READONLY;
          display_error(14, (char_t *) work_filename, FALSE);
        } else {
          display_error(8, work_filename, FALSE);
          free_view_memory(TRUE, TRUE);
          return (RC_ACCESS_DENIED);
        }
      }
      stat((char *) work_filename, &stat_buf);
      CURRENT_FILE->fmode = stat_buf.st_mode;
      CURRENT_FILE->modtime = stat_buf.st_mtime;
      CURRENT_FILE->uid = stat_buf.st_uid;
      CURRENT_FILE->gid = stat_buf.st_gid;
      if ((CURRENT_FILE->fp = fopen((char *) work_filename, "rb")) == NULL) {
        display_error(8, work_filename, FALSE);
        free_view_memory(TRUE, TRUE);
        return (RC_ACCESS_DENIED);
      }
      break;
    case THE_FILE_NAME_TOO_LONG:
      display_error(8, (char_t *) "- file name too long", TRUE);
      free_view_memory(TRUE, TRUE);
      return (RC_ACCESS_DENIED);
      break;
    default:
      CURRENT_FILE->fmode = 0;
      CURRENT_FILE->modtime = 0;
      CURRENT_FILE->disposition = FILE_NEW;
      display_error(20, (char_t *) work_filename, TRUE);
      break;
  }
  /*
   * Reset RC to RC_OK after we used it when checking for file's existence.
   * It is assumed to be RC_OK at this point!
   */
  rc = RC_OK;
  /*
   * first_line is set to "Top of File"
   */
  if ((CURRENT_FILE->first_line = add_LINE(CURRENT_FILE->first_line, NULL, TOP_OF_FILE, strlen((char *) TOP_OF_FILE), 0, FALSE)) == NULL) {
    if (CURRENT_FILE->disposition != FILE_NEW)
      fclose(CURRENT_FILE->fp);
    free_view_memory(TRUE, TRUE);
    return (RC_OUT_OF_MEMORY);
  }

  curr = CURRENT_FILE->first_line;
  /*
   * Read in the existing file...
   */
  CURRENT_FILE->number_lines = 0L;
  if (CURRENT_FILE->disposition != FILE_NEW) {
    if (!display_length) {      /* NOT reading fixed length records... */
      if ((curr = read_file(CURRENT_FILE->fp, curr, work_filename, 1L, 0L, FALSE)) == NULL) {
        if (CURRENT_FILE->disposition != FILE_NEW)
          fclose(CURRENT_FILE->fp);
        free_view_memory(TRUE, TRUE);
        return (RC_ACCESS_DENIED);
      }
    } else {
      if ((curr = read_fixed_file(CURRENT_FILE->fp, curr, work_filename, 1L, 0L)) == NULL) {
        if (CURRENT_FILE->disposition != FILE_NEW)
          fclose(CURRENT_FILE->fp);
        free_view_memory(TRUE, TRUE);
        return (RC_ACCESS_DENIED);
      }
    }
  }
  /*
   * Close the file...
   */
  if (CURRENT_FILE->disposition != FILE_NEW)
    fclose(CURRENT_FILE->fp);
  /*
   * last line is set to "Bottom of File"
   */
  if ((CURRENT_FILE->last_line = add_LINE(CURRENT_FILE->first_line, curr, BOTTOM_OF_FILE, strlen((char *) BOTTOM_OF_FILE), 0, FALSE)) == NULL) {
    free_view_memory(TRUE, TRUE);
    return (RC_OUT_OF_MEMORY);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  /*
   * If TABSIN is ON, we need to run EXPAND ALL.
   * We need to save the current setting of scope so that it can be
   * changed to ALL so that every line will be expanded.
   * Of course if TABSIN OFF was set we DON'T run EXPAND ALL :-)
   */
  if (TABI_ONx) {
    save_scope = CURRENT_VIEW->scope_all;
    save_stay = CURRENT_VIEW->stay;
    CURRENT_VIEW->stay = TRUE;
    rc = execute_expand_compress((char_t *) "ALL", TRUE, FALSE, FALSE, FALSE);
    if (rc == RC_TOF_EOF_REACHED)
      rc = RC_OK;
    CURRENT_VIEW->scope_all = save_scope;
    CURRENT_VIEW->stay = save_stay;
  }
  /*
   * Trim trailing spaces only if THE not started with -u switch
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
  /*
   * If AUTOCOLORING is ON, find the appropriate parser.
   */
  if (CURRENT_FILE->colouring)
    find_auto_parser(CURRENT_FILE);
  return (rc);
}
LINE *read_file(FILE * fp, LINE * curr, char_t * filename, line_t fromline, line_t numlines, bool called_from_get_command) {
#define THE_CR '\r'
#define THE_LF '\n'
#define DOSEOF 26
  length_t i = 0L;
  length_t maxlen = 0;
  short ch = 0;
  LINE *temp = NULL;
  length_t len = 0;
  bool eof_reached = FALSE;
  length_t chars_read = 0;
  length_t line_start = 0;
  length_t total_line_length = 0;
  int extra = 0;
  length_t read_start = 0;
  line_t total_lines_read = 0L, actual_lines_read = 0L;

  temp = curr;

  /*
   * Reset the length of trec_len, as it may have been changed elsewhere.
   */
  trec_len = max_trec_len;
  while (1) {
    line_start = 0;
    chars_read = (length_t) fread(brec, sizeof(char_t), max_line_length, fp);
    if (chars_read + read_start > trec_len) {
      sprintf((char *) trec, "Line %ld exceeds max. width of %ld. File: %s", total_lines_read + 1, max_line_length, filename);
      display_error(29, trec, FALSE);
      if (!called_from_get_command)
        CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
      return (NULL);
    }
    memcpy(trec + read_start, brec, sizeof(char_t) * chars_read);
    if (feof(fp)) {
      eof_reached = TRUE;
      if (chars_read > 0) {
        for (; *(trec + read_start + chars_read - 1) == DOSEOF; chars_read--);
      }
    }
    /*
     * For each character remaining from the previous read in an incomplete
     * line and each character read from the last fread()...
     */
    for (i = read_start; i < chars_read + read_start; i++) {
      /*
       * If the character is a CR or LF we have a new line...
       */
      if (*(trec + i) != THE_CR && *(trec + i) != THE_LF)
        continue;
      {
        /*
         * If we have read all the lines in the file that has been requested,
         * get out.
         */
        if (actual_lines_read == numlines && numlines != 0) {
          line_start = chars_read + read_start;
          eof_reached = TRUE;
          break;
        }
        extra = 0;
        if (*(trec + i) == THE_CR) {
          if (i == chars_read + read_start - 1) {
            *(trec + i + 1) = fgetc(fp);
            if (feof(fp))
              eof_reached = TRUE;
            else
              chars_read++;
          }
          ch = *(trec + i + 1);
          if (ch == THE_LF)
            extra = 1;
        }
        if (actual_lines_read == numlines && numlines == 0 && !called_from_get_command) {
          if (extra == 0) {
            if (*(trec + i) == THE_CR)
              CURRENT_FILE->eolfirst = EOLOUT_CR;
            else
              CURRENT_FILE->eolfirst = EOLOUT_LF;
          } else
            CURRENT_FILE->eolfirst = EOLOUT_CRLF;
        }
        if (++total_lines_read >= fromline) {
          len = i - line_start;
          total_line_length += len;
          if (total_line_length > max_line_length) {
            sprintf((char *) trec, "Line %ld exceeds max. width of %ld. File: %s", total_lines_read, max_line_length, filename);
            display_error(29, trec, FALSE);
            if (!called_from_get_command)
              CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
            return (NULL);
          }
          if (total_line_length > maxlen)
            maxlen = total_line_length;
          if ((temp = add_LINE(CURRENT_FILE->first_line, temp, trec + line_start, len, 0, FALSE)) == NULL) {
            if (!called_from_get_command)
              CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
            return (NULL);
          }
          actual_lines_read++;
        }
        line_start = i + 1 + extra;
        i += extra;
        total_line_length = 0;
      }
    }

    if (line_start != chars_read + read_start) {        /* incomplete line */
      if (line_start > chars_read + read_start)
        read_start = 0;
      else {
        len = chars_read + read_start - line_start;
        if (eof_reached) {
          if (++total_lines_read >= fromline) {
            total_line_length += len;
            if (total_line_length > max_line_length) {
              sprintf((char *) trec, "Line %ld exceeds max. width of %ld. File: %s", total_lines_read, max_line_length, filename);
              display_error(29, trec, FALSE);
              if (!called_from_get_command)
                CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
              return (NULL);
            }
          }
          if ((temp = add_LINE(CURRENT_FILE->first_line, temp, trec + line_start, len, 0, FALSE)) == NULL) {
            if (!called_from_get_command)
              CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
            return (NULL);
          }
          actual_lines_read++;
        } else {
          for (i = 0; i < len; i++)
            *(trec + i) = *(trec + i + line_start);
          read_start = len;
        }
      }
    } else
      read_start = 0;
    if (eof_reached)
      break;
  }

  CURRENT_FILE->max_line_length = maxlen;
  CURRENT_FILE->number_lines += actual_lines_read;
  return (temp);
}
LINE *read_fixed_file(FILE * fp, LINE * curr, char_t * filename, line_t fromline, line_t numlines) {
  LINE *temp = NULL;
  bool eof_reached = FALSE;
  length_t chars_read = 0;
  line_t total_lines_read = 0L, actual_lines_read = 0L;

  temp = curr;

  while (1) {
    chars_read = (length_t) fread(trec, sizeof(char_t), display_length, fp);
    if (feof(fp)) {
      eof_reached = TRUE;
    }
    if (chars_read != 0) {
      /*
       * If we have read all the lines in the file that has been requested,
       * get out.
       */
      if (actual_lines_read == numlines && numlines != 0) {
        eof_reached = TRUE;
        break;
      }
      if (++total_lines_read >= fromline) {
        if ((temp = add_LINE(CURRENT_FILE->first_line, temp, trec, chars_read, 0, FALSE)) == NULL) {
          CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
          return (NULL);
        }
        actual_lines_read++;
      }
    }
    if (eof_reached)
      break;
  }
  CURRENT_FILE->max_line_length = display_length;
  CURRENT_FILE->number_lines += actual_lines_read;
  CURRENT_FILE->eolfirst = EOLOUT_NONE;
  return (temp);
}
short save_file(FILE_DETAILS * cf, char_t * new_fname, bool force, line_t in_lines, line_t start_line, line_t * num_file_lines, bool append, length_t start_col, length_t end_col, bool ignore_scope, bool lines_based_on_scope, bool autosave) {
  char_t *bak_filename = NULL;
  char_t *write_fname = NULL;
  line_t i = 0L;
  line_t abs_num_lines = (in_lines < 0L ? -in_lines : in_lines);
  line_t num_actual_lines = 0L;
  line_t my_num_file_lines = 0L;
  short direction = (in_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  LINE *curr = NULL;
  FILE *fp = NULL;
  length_t col = 0, newcol = 0;
  long off = 0L;
  char_t c = 0;
  short rc = RC_OK;
  bool same_file = TRUE;
  bool save_scope_all = CURRENT_VIEW->scope_all;
  char_t eol[2];
  int eol_len = 0;
  char buf[MAX_FILE_NAME + 1];

  /*
   * Do not attempt to autosave a pseudo file...
   */
  if (cf->pseudo_file && (autosave || CURRENT_FILE->save_alt == 0)
      && blank_field(new_fname)) {
    return (RC_OK);
  }

  switch (cf->eolout) {
    case EOLOUT_CRLF:
      eol[0] = (char_t) '\r';
      eol[1] = (char_t) '\n';
      eol_len = 2;
      break;
    case EOLOUT_LF:
      eol[0] = (char_t) '\n';
      eol_len = 1;
      break;
    case EOLOUT_CR:
      eol[0] = (char_t) '\r';
      eol_len = 1;
      break;
    case EOLOUT_NONE:
      eol[0] = '\0';
      eol_len = 0;
      break;
  }
  if ((write_fname = (char_t *) malloc(MAX_FILE_NAME)) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  if (strcmp((char *) new_fname, "") != 0) { /* new_fname supplied */
    /*
     * If a new filename is specified, use it as the old filename.
     */
    strrmdup(strtrans(new_fname, OSLASH, ISLASH), ISLASH, TRUE);
    /*
     * Split the supplied new filename.
     */
    if ((rc = splitpath(new_fname)) != RC_OK) {
      display_error(10, new_fname, FALSE);
      if (bak_filename != (char_t *) NULL)
        free(bak_filename);
      if (write_fname != (char_t *) NULL)
        free(write_fname);
      return (rc);
    }
    strcpy((char *) write_fname, (char *) sp_path);
    strcat((char *) write_fname, (char *) sp_fname);
    same_file = FALSE;
    /*
     * Test to make sure that the write fname doesn't exist...
     * ...unless we are forcing the write.
     */
    if ((!force) && file_exists(write_fname) == THE_FILE_EXISTS) {
      display_error(31, write_fname, FALSE);
      if (bak_filename != (char_t *) NULL)
        free(bak_filename);
      if (write_fname != (char_t *) NULL)
        free(write_fname);
      return (RC_ACCESS_DENIED);
    }
    /*
     * Test to make sure that we can write the file.
     */
    if (!file_writable(write_fname)) {
      display_error(8, write_fname, FALSE);
      if (bak_filename != (char_t *) NULL)
        free(bak_filename);
      if (write_fname != (char_t *) NULL)
        free(write_fname);
      return (RC_ACCESS_DENIED);
    }
  } else {
    /*
     * We are using the same file name for the new file.
     * Create the name of the current file.
     */
    strcpy((char *) write_fname, (char *) cf->fpath);
    strcat((char *) write_fname, (char *) cf->fname);
    /*
     * If the file to be saved is a "pseudo" file, return with an error...
     */
    if (cf->pseudo_file) {
      display_error(8, write_fname, FALSE);
      if (bak_filename != (char_t *) NULL)
        free(bak_filename);
      if (write_fname != (char_t *) NULL)
        free(write_fname);
      return (RC_ACCESS_DENIED);
    }
    /*
     * Check if the file to be written is a symlink. If so, get the "real"
     * filename and use it from here on...
     */
    rc = readlink((char *) write_fname, buf, sizeof(buf));
    if (rc != (-1)) {
      memcpy((char *) write_fname, buf, rc);
      write_fname[rc] = '\0';
    }
    /*
     * If the file exists, test to make sure we can write it and save a
     * backup copy.
     */
    if (file_exists(write_fname) == THE_FILE_EXISTS) {
      /*
       * Test to make sure that we can write the file.
       */
      if (!file_writable(write_fname)) {
        display_error(8, write_fname, FALSE);
        if (bak_filename != (char_t *) NULL)
          free(bak_filename);
        if (write_fname != (char_t *) NULL)
          free(write_fname);
        return (RC_ACCESS_DENIED);
      }

      if ((rc = process_file_attributes(0, cf, write_fname)) != RC_OK) {
        if (bak_filename != (char_t *) NULL)
          free(bak_filename);
        if (write_fname != (char_t *) NULL)
          free(write_fname);
        return (rc);
      }
      /*
       * If TIMECHECK is ON, check if the modification time of the file is
       * the same as when we read the file or last saved it and we are NOT
       * forcing the write...
       */
      if (CURRENT_FILE->timecheck && !force) {
        stat((char *) write_fname, &stat_buf);
        if (CURRENT_FILE->modtime != stat_buf.st_mtime) {
          display_error(138, (char_t *) "", FALSE);
          if (bak_filename != (char_t *) NULL)
            free(bak_filename);
          if (write_fname != (char_t *) NULL)
            free(write_fname);
          return (RC_ACCESS_DENIED);
        }
      }
      /*
       * Rename the current file to filename[BACKUP_SUFFIXx].
       */
      if (cf->backup != BACKUP_OFF) {
        if ((bak_filename = (char_t *) malloc(strlen((char *) cf->fpath) + strlen((char *) cf->fname) + strlen((char *) BACKUP_SUFFIXx) + 1)) == NULL) {
          display_error(30, (char_t *) "", FALSE);
          if (bak_filename != (char_t *) NULL)
            free(bak_filename);
          if (write_fname != (char_t *) NULL)
            free(write_fname);
          return (RC_OUT_OF_MEMORY);
        }
        new_filename(cf->fpath, cf->fname, bak_filename, BACKUP_SUFFIXx);
        if (cf->fp != NULL) {
          remove_file(bak_filename);
          if (cf->backup == BACKUP_INPLACE) {
            /*
             * Copy the contents of the current file to the BACKUP_SUFFIXx file
             */
            FILE *fp1 = fopen((char *) write_fname, "rb");
            FILE *fp2 = NULL;
            int num = 0;
            char tmp[5120];

            if (fp1 == NULL) {
              display_error(8, write_fname, FALSE);
              if (bak_filename != (char_t *) NULL)
                free(bak_filename);
              if (write_fname != (char_t *) NULL)
                free(write_fname);
              return (RC_ACCESS_DENIED);
            }
            fp2 = fopen((char *) bak_filename, "wb");
            if (fp2 == NULL) {
              display_error(8, bak_filename, FALSE);
              if (bak_filename != (char_t *) NULL)
                free(bak_filename);
              if (write_fname != (char_t *) NULL)
                free(write_fname);
              fclose(fp1);
              return (RC_ACCESS_DENIED);
            }
            while (1) {
              num = fread(tmp, sizeof(char), 5120, fp1);
              if (fwrite(tmp, sizeof(char), num, fp2) != num) {
              }
              if (feof(fp1))
                break;
            }
            fclose(fp1);
            fclose(fp2);
            /*
             * Restore any file attributes that can be restored
             * to the backup file
             */
            process_file_attributes(1, cf, bak_filename);
          } else {
            if (rename((char *) write_fname, (char *) bak_filename) != 0) {
              display_error(8, write_fname, FALSE);
              if (bak_filename != (char_t *) NULL)
                free(bak_filename);
              if (write_fname != (char_t *) NULL)
                free(write_fname);
              return (RC_ACCESS_DENIED);
            }
          }
        }
      }
    }
  }
  /*
   * Open the file we are writing to...
   */
  if (append == TRUE)
    fp = fopen((char *) write_fname, "ab");
  else
    fp = fopen((char *) write_fname, "wb");
  if (fp == NULL) {
    display_error(8, (char_t *) "could not open for writing", FALSE);
    if (bak_filename != (char_t *) NULL)
      free(bak_filename);
    if (write_fname != (char_t *) NULL)
      free(write_fname);
    return (RC_ACCESS_DENIED);
  }
  /*
   * Determine where to start writing from in the linked list.
   */
  curr = lll_find(cf->first_line, cf->last_line, start_line, cf->number_lines);
  /*
   * Save the setting of scope_all if we are ignoring scope. ie full file
   * is being written...
   */
  if (ignore_scope)
    CURRENT_VIEW->scope_all = TRUE;
  /*
   * Now write out the contents of the file array to the new filename.
   */
  rc = RC_OK;
  for (i = 0L, num_actual_lines = 0L;; i++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines)
        break;
    } else {
      if (abs_num_lines == i)
        break;
    }
    rc = processable_line(CURRENT_VIEW, start_line + (line_t) (i * direction), curr);
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
        if (cf->tabsout_on) {
          col = 0;
          off = (long) start_col;
          while (1) {
            newcol = col;
            while ((c = next_char(curr, &off, end_col + 1)) == ' ') {
              newcol++;
              if ((newcol % cf->tabsout_num) == 0) {
                if ((rc = write_char((char_t) '\t', fp)) == RC_DISK_FULL)
                  break;
                col = newcol;
              }
            }
            for (; col < newcol; col++) {
              if ((rc = write_char((char_t) ' ', fp)) == RC_DISK_FULL)
                break;
            }
            if (off == (-1L))   /* end of line */
              break;
            if ((rc = write_char((char_t) c, fp)) == RC_DISK_FULL)
              break;
            col++;
          }
          if (rc)
            break;
        } else {
          if (start_col < curr->length) {
            if ((rc = write_line(curr->line + start_col, min(curr->length - start_col, (end_col - start_col) + 1), fp, CURRENT_FILE->trailing)) == RC_DISK_FULL)
              break;
          } else {
            /*
             * No characters to write, but we need to call this so we can handle
             * trailing blanks for TRAILING_EMPTY setting properly
             */
            if ((rc = write_line((char_t *) "", 0, fp, CURRENT_FILE->trailing)) == RC_DISK_FULL)
              break;
          }
        }
        if (rc)
          break;
        if ((rc = write_line(eol, eol_len, fp, TRAILING_ON)) == RC_DISK_FULL)
          break;
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (rc)
      break;
    if (direction == DIRECTION_BACKWARD)
      curr = curr->prev;
    else
      curr = curr->next;
    my_num_file_lines += (line_t) direction;
    if (curr == NULL)
      break;
  }
  if (num_file_lines)
    *num_file_lines = my_num_file_lines;
  /*
   * Restore the setting of scope_all if we changed it before...
   */
  if (ignore_scope)
    CURRENT_VIEW->scope_all = save_scope_all;

  if (fflush(fp) == EOF) {
    rc = RC_DISK_FULL;
    rewind(fp);
  }
  if (fclose(fp) == EOF)
    rc = RC_DISK_FULL;
  if (rc)
    clearerr(fp);
  /*
   * If an error occurred in writing the file (usuallly a result of a
   * disk full error), get the files back to the way they were before
   * this attempt to write them.
   */
  if (rc) {
    /* remove 'new' file (the one that couldn't be written) */
    remove_file(write_fname);
    if (same_file) {
      if (rename((char *) bak_filename, (char *) write_fname) != 0) {
        display_error(8, write_fname, FALSE);
        if (bak_filename != (char_t *) NULL)
          free(bak_filename);
        if (write_fname != (char_t *) NULL)
          free(write_fname);
        return (RC_ACCESS_DENIED);
      }
    }
  } else {
    if (same_file) {
      if (cf->backup != BACKUP_INPLACE) {
        if (cf->fmode != 0)
          chmod((char *) write_fname, cf->fmode);
        if (cf->disposition != FILE_NEW) {
          if (chown((char *) write_fname, cf->uid, cf->gid)) {
            display_error(84, (char_t *) "ownerships", FALSE);
          }
        }
        process_file_attributes(1, cf, write_fname);
      }
      /*
       * Save the new timestamp of the file.
       */
      stat((char *) write_fname, &stat_buf);
      CURRENT_FILE->modtime = stat_buf.st_mtime;
    }
    /*
     * If a backup file is not to be kept, remove the backup file provided
     * that there hasn't been a problem in writing the file.
     */
    if (cf->backup == BACKUP_TEMP)
      remove_file(bak_filename);
    /*
     * If a new filename was not supplied, free up temporary memory.
     */
  }
  if (bak_filename != (char_t *) NULL)
    free(bak_filename);
  if (write_fname != (char_t *) NULL)
    free(write_fname);

  return (rc);
}
static short write_char(char_t chr, FILE * fp) {
  if (fputc(chr, fp) == chr && ferror(fp) == 0) {
    return (RC_OK);
  }
  clearerr(fp);
  display_error(57, (char_t *) "", FALSE);
  return (RC_DISK_FULL);
}
static short write_line(char_t * line, length_t len, FILE * fp, short trailing) {
  short rc = RC_OK;
  long newlen = len;

  /*
   * If we started THE with -u switch, then DO NOT do anything with
   * TRAILING; force it to ON
   */
  if (display_length != 0)
    trailing = TRAILING_ON;
  /*
   * Determine what we do with trailing blanks. We always write the
   * extra trailing blanks BEFORE the supplied string as 'trailing' is
   * only ever a real value when the end of line string is written
   */
  if (trailing != TRAILING_ON) {
    newlen = 1 + (long) memrevne(line, ' ', len);
  }
  if (fwrite(line, sizeof(char_t), newlen, fp) != newlen) {
    display_error(57, (char_t *) "", FALSE);
    rc = RC_DISK_FULL;
  }
  switch (trailing) {
    case TRAILING_EMPTY:
      if (newlen == 0)
        rc = write_char((char_t) ' ', fp);
      break;
    case TRAILING_SINGLE:
      rc = write_char((char_t) ' ', fp);
      break;
    default:
      break;
  }
  return rc;
}
void increment_alt(FILE_DETAILS * cf) {
  cf->autosave_alt++;
  cf->save_alt++;
  /*
   * We can now test for autosave_alt exceeding the defined limit and
   * carry out an autosave if necessary.
   */
  if (cf->autosave != 0 && cf->autosave_alt >= cf->autosave && cf->autosave_fname) {
    if (save_file(cf, cf->autosave_fname, TRUE, cf->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, TRUE) == RC_OK)
      cf->autosave_alt = 0;
  }
  return;
}
char_t *new_filename(char_t * ofp, char_t * ofn, char_t * nfn, char_t * ext) {

  strcpy((char *) nfn, (char *) ofp);
  strcat((char *) nfn, (char *) ofn);

  strcat((char *) nfn, (char *) ext);
  return (nfn);
}
short remove_aus_file(FILE_DETAILS * cf) {
  char_t *aus_filename = NULL;

  if ((aus_filename = (char_t *) malloc(strlen((char *) cf->fpath) + strlen((char *) cf->fname) + 5)) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  new_filename(cf->fpath, cf->fname, aus_filename, (char_t *) ".aus");
  remove_file(aus_filename);
  free(aus_filename);
  return (RC_OK);
}
short free_view_memory(bool free_file_lines, bool display_the_screen) {
  VIEW_DETAILS *save_current_view = NULL;
  char_t save_current_screen = 0;
  short rc = RC_OK;
  int y = 0, x = 0;
  int scenario = 0;
  row_t save_cmd_line = 0;
  char_t save_prefix = 0;
  short save_gap = 0, save_prefix_width = 0;

  /*
   * Before freeing up anything, determine which scenario is current...
   */
  if (display_screens > 1) {
    if (CURRENT_SCREEN.screen_view->file_for_view == OTHER_SCREEN.screen_view->file_for_view) {
      if (number_of_files > 1)
        scenario = 2;
      else
        scenario = 3;
    } else
      scenario = 4;
  } else {
    if (number_of_files > 1)
      scenario = 1;
    else
      scenario = 0;
  }
  /*
   * Save details of the current view's prefix and cmd_line settings so
   * that if the new view has different settings we can adjust the size
   * and/or position of the new view's windows.
   */
  save_prefix = CURRENT_VIEW->prefix;
  save_prefix_width = CURRENT_VIEW->prefix_width;
  save_gap = CURRENT_VIEW->prefix_gap;
  save_cmd_line = CURRENT_VIEW->cmd_line;
  /*
   * Free the view and the memory for the file if this is the last view
   * for that file...
   */
  if (--CURRENT_FILE->file_views == 0)
    free_file_memory(free_file_lines);
  free_a_view();

  switch (scenario) {
    case 0:
      /*
       *----------------------------------------------------------
       * +---+---+
       * |   a   |
       * |       | --> exit
       * |qq     |
       * +---+---+
       * display_screens = 1 & number_of_files = 1 & same views
       * (scenario 0)
       *----------------------------------------------------------
       * we need to clean up everything as though there we are
       * starting THE from the beginning, because the command could
       * be 'QQ#x newfile" or the "file" being displayed could be
       * DIR.DIR and a new LS command issued.
       *----------------------------------------------------------
       */
      break;
    case 1:
      /*
       *----------------------------------------------------------
       * +---+---+     +---+---+
       * |   a   |  b  |   b   |
       * |       | --> |       |
       * |qq     |     |       |
       * +---+---+     +---+---+
       * display_screens = 1 & number_of_files > 1 & same views
       * (scenario 1)
       *----------------------------------------------------------
       */
      if ((save_prefix & PREFIX_LOCATION_MASK) != (CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK)
          || save_gap != CURRENT_VIEW->prefix_gap || save_prefix_width != CURRENT_VIEW->prefix_width || save_cmd_line != CURRENT_VIEW->cmd_line) {
        set_screen_defaults();
        if (curses_started) {
          if (set_up_windows(current_screen) != RC_OK) {
            return (RC_OK);
          }
        }
      }
      break;
    case 2:
      /*
       *----------------------------------------------------------
       * +---+---+     +---+---+       +---+---+     +---+---+
       * | a | a |  b  | b | b |   or  | a | a | b c | b | b |
       * |   |   | --> |   |   |       |   |   |     |   |   |
       * |   |qq |     |   |   |       |   |qq |     |   |   |
       * +---+---+     +---+---+       +---+---+     +---+---+
       * display_screens > 1 & number_of_files > 1 & same views
       * (scenario 2)
       *----------------------------------------------------------
       * At this point the view associated with the "current" screen
       * has been deleted. vd_current now points to the "next" view.
       * We need to point vd_current at the view in OTHER_SCREEN so
       * it can be deleted properly.
       *----------------------------------------------------------
       */
      CURRENT_VIEW = OTHER_VIEW;
      free_file_memory(free_file_lines);
      free_a_view();
      save_current_screen = current_screen;
      save_current_view = CURRENT_VIEW;
      current_screen = (current_screen == 0) ? 1 : 0;   /* make other screen current */
      if ((rc = defaults_for_other_files(PREVIOUS_VIEW)) != RC_OK) {
        return (rc);
      }
      CURRENT_FILE = save_current_view->file_for_view;
      CURRENT_FILE->file_views++;
      CURRENT_SCREEN.screen_view = CURRENT_VIEW;
      CURRENT_VIEW = save_current_view;
      current_screen = save_current_screen;
      set_screen_defaults();
      if (curses_started) {
        if ((rc = set_up_windows((char_t) (other_screen))) != RC_OK) {
          return (rc);
        }
        if ((rc = set_up_windows(current_screen)) != RC_OK) {
          return (rc);
        }
      }
      pre_process_line(OTHER_SCREEN.screen_view, OTHER_SCREEN.screen_view->focus_line, (LINE *) NULL);
      prepare_view((char_t) (other_screen));
      if (display_the_screen)
        display_screen((char_t) (other_screen));
      break;
    case 3:
      /*
       *----------------------------------------------------------
       * +---+---+
       * | a | a |
       * |   |   | --> exit
       * |   |qq |
       * +---+---+
       * display_screens > 1 & number_of_files = 1 & same views
       * (scenario 3)
       *----------------------------------------------------------
       * we need to clean up everything as though there we are
       * starting THE from the beginning, because the command could
       * be 'QQ#x newfile" or the "file" being displayed could be
       * DIR.DIR and a new LS command issued.
       *----------------------------------------------------------
       */
      CURRENT_FILE->file_views--;
      free_file_memory(free_file_lines);
      free_a_view();
      /*
       * Reset the number of screens to 1 and delete the divider window
       * if it exists
       */
      display_screens = 1;
      current_screen = 0;
      if (divider != (WINDOW *) NULL) {
        delwin(divider);
        divider = NULL;
      }
      break;
    case 4:
      /*
       *----------------------------------------------------------
       * +---+---+     +---+---+       +---+---+     +---+---+
       * | a | b |     | a | a |   or  | a | b |  c  | a | a |
       * |   |   | --> |   |   |       |   |   |     |   |   |
       * |   |qq |     |   |   |       |   |qq |     |   |   |
       * +---+---+     +---+---+       +---+---+     +---+---+
       * display_screens > 1 & number_of_files > 1 & diff views
       * (scenario 4)
       *----------------------------------------------------------
       */
      if ((rc = defaults_for_other_files(OTHER_VIEW)) != RC_OK) {
        return (rc);
      }
      CURRENT_SCREEN.screen_view = CURRENT_VIEW;
      CURRENT_FILE = OTHER_SCREEN.screen_view->file_for_view;
      CURRENT_FILE->file_views++;
      set_screen_defaults();
      if ((rc = set_up_windows(current_screen)) != RC_OK) {
        return (rc);
      }
      break;
    default:
      break;
  }
  /*
   * If we still have at least one view, display it.
   */
  if (number_of_views > 0) {
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    prepare_view(current_screen);
    if (display_the_screen)
      display_screen(current_screen);
    if (curses_started) {
      wmove(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
      if (CURRENT_WINDOW_PREFIX != NULL)
        wmove(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
      getyx(CURRENT_WINDOW, y, x);
      wmove(CURRENT_WINDOW, y, x);
    }
  }
  return (RC_OK);
}
void free_a_view(void) {
  /*
   * If the marked block is within the current view, unset the variables.
   */
  if (MARK_VIEW == CURRENT_VIEW)
    MARK_VIEW = (VIEW_DETAILS *) NULL;
  if (CURRENT_VIEW->preserved_view_details)
    free(CURRENT_VIEW->preserved_view_details);
  CURRENT_VIEW = vll_del(&vd_first, &vd_last, CURRENT_VIEW, DIRECTION_BACKWARD);
  CURRENT_SCREEN.screen_view = CURRENT_VIEW;
  number_of_views--;
  return;
}
short free_file_memory(bool free_file_lines) {
  /*
   * If the file name is not NULL, free it...
   */
  if (CURRENT_FILE->fname != NULL) {
    free(CURRENT_FILE->fname);
    CURRENT_FILE->fname = (char_t *) NULL;
  }
  /*
   * If the actual file name is not NULL, free it...
   */
  if (CURRENT_FILE->actualfname != NULL) {
    free(CURRENT_FILE->actualfname);
    CURRENT_FILE->actualfname = (char_t *) NULL;
  }
  /*
   * If the origianl file name is not NULL, free it...
   */
  if (CURRENT_FILE->efileid != NULL) {
    free(CURRENT_FILE->efileid);
    CURRENT_FILE->efileid = (char_t *) NULL;
  }
  /*
   * If the file path is not NULL, free it...
   */
  if (CURRENT_FILE->fpath != NULL) {
    free(CURRENT_FILE->fpath);
    CURRENT_FILE->fpath = (char_t *) NULL;
  }
  /*
   * If the autosave file name is not NULL, free it...
   */
  if (CURRENT_FILE->autosave_fname != NULL) {
    free(CURRENT_FILE->autosave_fname);
    CURRENT_FILE->autosave_fname = (char_t *) NULL;
  }
  /*
   * If the file display attributes is not NULL, free it...
   */
  if (CURRENT_FILE->attr != NULL) {
    free(CURRENT_FILE->attr);
    CURRENT_FILE->attr = (COLOUR_ATTR *) NULL;
  }
  /*
   * If the file ecolour attributes is not NULL, free it...
   */
  if (CURRENT_FILE->ecolour != NULL) {
    free(CURRENT_FILE->ecolour);
    CURRENT_FILE->ecolour = (COLOUR_ATTR *) NULL;
  }
  /*
   * Free the linked list of all lines in the file...
   */
  if (free_file_lines) {
    CURRENT_FILE->first_line = CURRENT_FILE->last_line = lll_free(CURRENT_FILE->first_line);
    switch (CURRENT_FILE->pseudo_file) {
      case PSEUDO_DIR:
        dir_first_line = dir_last_line = NULL;
        dir_number_lines = 0L;
        break;
      case PSEUDO_REXX:
        rexxout_first_line = rexxout_last_line = NULL;
        break;
      case PSEUDO_KEY:
        key_first_line = key_last_line = NULL;
        break;
      default:
        break;
    }
  }
  /*
   * Free the linked list of all pending prefix commands...
   */
  pll_free(CURRENT_FILE->first_ppc);
  /*
   * Free the linked list of reserved lines...
   */
  rll_free(CURRENT_FILE->first_reserved);
  /*
   * Free the linked list of EDITV variables...
   */
  lll_free(CURRENT_FILE->editv);
  /*
   * Free the preserved file details (if any)
   */
  if (CURRENT_FILE->preserved_file_details) {
    free(CURRENT_FILE->preserved_file_details);
    CURRENT_FILE->preserved_file_details = NULL;
  }
  /*
   * Free the FILE_DETAILS structure...
   */
  if (CURRENT_FILE != NULL) {
    free(CURRENT_FILE);
    CURRENT_FILE = (FILE_DETAILS *) NULL;
  }
  /*
   * Before decrementing the count of files, deregister the ring.number_of_files
   * implied function.
   */
  if (rexx_support) {
    char_t tmp[20];

    sprintf((char *) tmp, "ring.%ld", number_of_files);
    MyRexxDeregisterFunction(tmp);
  }
  number_of_files--;
  return (RC_OK);
}
short read_directory(void) {
  struct dirfile *dpfirst = NULL, *dplast = NULL, *dp;
  char_t str_attr[12];
  char_t str_date[12];
  char_t str_time[6];
  VIEW_DETAILS *found_view = NULL;
  short rc = RC_OK;
  LINE *curr = NULL;
  char_t tmp[50];
  int len;
  char_t dir_rec[MAX_FILE_NAME + 50];

  /*
   * Get all file info for the selected files into structure. If no file
   * name specified, force it to '*'.
   */
  if (strcmp((char *) sp_fname, "") == 0)
    rc = getfiles(sp_path, (char_t *) "*", &dpfirst, &dplast);
  else
    rc = getfiles(sp_path, sp_fname, &dpfirst, &dplast);
  if (rc != RC_OK) {
    display_error((unsigned short) ((rc == RC_FILE_NOT_FOUND) ? 9 : rc), sp_path, FALSE);
    return (RC_FILE_NOT_FOUND);
  }
  if (dpfirst == dplast) {
    return (RC_FILE_NOT_FOUND);
  }
  /*
   * dir_path is set up here so that subsequent sos_edit commands can use
   * the directory path as a prefix to the edit files filename.
   */
  strcpy((char *) dir_path, (char *) sp_path);
  strcpy((char *) dir_files, (char *) sp_fname);
  /*
   * sort the array of file structures.
   */
  switch (DEFSORTx) {
    case DIRSORT_DATE:
      qsort(dpfirst, dplast - dpfirst, sizeof(struct dirfile), date_comp);
      break;
    case DIRSORT_TIME:
      qsort(dpfirst, dplast - dpfirst, sizeof(struct dirfile), time_comp);
      break;
    case DIRSORT_NAME:
      qsort(dpfirst, dplast - dpfirst, sizeof(struct dirfile), name_comp);
      break;
    case DIRSORT_SIZE:
      qsort(dpfirst, dplast - dpfirst, sizeof(struct dirfile), size_comp);
      break;
    case DIRSORT_DIR:
      qsort(dpfirst, dplast - dpfirst, sizeof(struct dirfile), dir_comp);
      break;
    default:                   /* DIRSORT_NONE - no sorting */
      break;
  }
  /*
   * Free up the existing linked list (if any)
   */
  dir_first_line = dir_last_line = lll_free(dir_first_line);
  dir_number_lines = 0L;
  if ((found_view = find_file(dir_pathname, dir_filename)) != (VIEW_DETAILS *) NULL) {
    found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
    found_view->file_for_view->number_lines = 0L;
  }
  /*
   * first_line is set to "Top of File"
   */
  if ((dir_first_line = add_LINE(dir_first_line, NULL, TOP_OF_FILE, strlen((char *) TOP_OF_FILE), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * last line is set to "Bottom of File"
   */
  if ((dir_last_line = add_LINE(dir_first_line, dir_first_line, BOTTOM_OF_FILE, strlen((char *) BOTTOM_OF_FILE), 0, FALSE)) == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  curr = dir_first_line;
  /*
   * write out the formatted contents of the file structures.
   */
  for (dp = dpfirst; dp < dplast; dp++, dir_number_lines++) {
    len = sprintf((char *) tmp, "%ld", dp->fsize);
    if (len > 8) {
      sprintf((char *) tmp, "%7ldk", dp->fsize / 1024);
    }
    if (dp->lname != NULL) {
      sprintf((char *) dir_rec, "%s%8s %s %s %s -> %s", file_attrs(dp->fattr, str_attr, dp->facl), tmp, file_date(dp, str_date), file_time(dp, str_time), dp->fname, dp->lname);
    } else {
      sprintf((char *) dir_rec, "%s%8s %s %s %s", file_attrs(dp->fattr, str_attr, dp->facl), tmp, file_date(dp, str_date), file_time(dp, str_time), dp->fname);
    }
    if ((curr = add_LINE(dir_first_line, curr, dir_rec, strlen((char *) dir_rec), 0, FALSE)) == NULL) {
      return (RC_OUT_OF_MEMORY);
    }
    free(dp->fname);
  }
  free(dpfirst);

  return (RC_OK);
}
VIEW_DETAILS *find_file(char_t * fp, char_t * fn) {
  VIEW_DETAILS *save_current_view = NULL, *found_file = NULL;

  save_current_view = CURRENT_VIEW;
  CURRENT_VIEW = vd_first;
  while (CURRENT_VIEW != (VIEW_DETAILS *) NULL) {
    if (strcmp((char *) CURRENT_FILE->fname, (char *) fn) == 0 && strcmp((char *) CURRENT_FILE->fpath, (char *) fp) == 0) {
      found_file = CURRENT_VIEW;
      CURRENT_VIEW = save_current_view;
      return (found_file);
    }
    CURRENT_VIEW = CURRENT_VIEW->next;
  }
  CURRENT_VIEW = save_current_view;
  return ((VIEW_DETAILS *) NULL);
}
VIEW_DETAILS *find_pseudo_file(char_t file) {
  VIEW_DETAILS *cv;

  cv = vd_first;
  while (cv) {
    if (cv->file_for_view && cv->file_for_view->pseudo_file == file) {
      return (cv);
    }
    cv = cv->next;
  }
  return ((VIEW_DETAILS *) NULL);
}
static short process_command_line(char_t * profile_command_line, line_t line_number) {
  short rc = RC_OK;
  short len = 0;
  bool strip = FALSE;

  /*
   * If the first line of the macro file does not contain the comment
   * 'NOREXX' abort further processing of the macro file.
   */
  if (memcmp(profile_command_line, "/*NOREXX*/", 10) != 0 && line_number == 1) {
    return (RC_NOREXX_ERROR);
  }
  /*
   * If the line is a comment, return with RC_OK.
   */
  if (memcmp(profile_command_line, "/*", 2) == 0) {     /* is a comment line */
    return (RC_OK);
  }
  /*
   * If the line begins and ends with a quote, single or double, strip
   * the quotes.
   */
  len = strlen((char *) profile_command_line);
  if (*(profile_command_line) == '\'' && *(profile_command_line + len - 1) == '\'')
    strip = TRUE;
  if (*(profile_command_line) == '"' && *(profile_command_line + len - 1) == '"')
    strip = TRUE;
  if (strip) {
    *(profile_command_line + len - 1) = '\0';
    profile_command_line++;
  }
  rc = command_line(profile_command_line, COMMAND_ONLY_FALSE);

  return (rc);
}
short execute_command_file(FILE * fp) {
  length_t i;
  char_t ch;
  short rc = RC_OK;
  line_t line_number = 0;

  memset(profile_command_line, ' ', MAX_LENGTH_OF_LINE);
  i = 0;
  while (1) {
    ch = fgetc(fp);
    if (feof(fp))
      break;
    if (ch == '\n') {
      line_number++;
      profile_command_line[i] = '\0';
      rc = process_command_line(profile_command_line, line_number);
      if (rc == RC_SYSTEM_ERROR || rc == RC_NOREXX_ERROR)
        break;
      if (number_of_files == 0)
        break;
      i = 0;
      memset(profile_command_line, ' ', MAX_LENGTH_OF_LINE);
      continue;
    }
    if (ch == '\r') {
      profile_command_line[i] = ch;
      i++;
      ch = fgetc(fp);
      if (feof(fp))
        break;
      if (ch == '\n') {
        --i;
        line_number++;
        profile_command_line[i] = '\0';
        rc = process_command_line(profile_command_line, line_number);
        if (rc == RC_SYSTEM_ERROR || rc == RC_NOREXX_ERROR)
          break;
        if (number_of_files == 0)
          break;
        i = 0;
        memset(profile_command_line, ' ', MAX_LENGTH_OF_LINE);
        continue;
      }
    }
    profile_command_line[i] = ch;
    i++;
  }
  return (rc);
}

char_t *read_file_into_memory(char_t * filename, int *buffer_size) {
  FILE *fp = NULL;
  char_t *buffer = NULL;

  if ((stat((char *) filename, &stat_buf) == 0)
      && S_ISDIR(stat_buf.st_mode)) {
    display_error(8, (char_t *) "specified file is a directory", FALSE);
    return NULL;
  }
  if ((buffer = (char_t *) malloc((stat_buf.st_size) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return NULL;
  }
  /*
   * Open the TLD file in text mode, so on DOSish systems we don't get
   * CRLFs.
   */
  fp = fopen((char *) filename, "r");
  if (fp == NULL) {
    display_error(8, filename, FALSE);
    free(buffer);
    return NULL;
  }
  /*
   * Read the full file into memory. Use the returned value from
   * fread() to cater for CR/LF translations if appropriate.
   */
  *buffer_size = fread(buffer, sizeof(char_t), stat_buf.st_size, fp);
  fclose(fp);
  return (buffer);
}
