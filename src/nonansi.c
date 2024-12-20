// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

#include <pwd.h>
#include <errno.h>

/*
 * Replace non-ANSI defs with ANSI ones
 */

short file_readable(uchar *filename) {
  if (access((char *) filename, R_OK) == (-1)) {
    return (FALSE);
  }
  return (TRUE);
}

short file_writable(uchar *filename) {
  if (file_exists(filename) != THE_FILE_EXISTS) {
    return (TRUE);
  }
  if (access((char *) filename, W_OK) == (-1)) {
    return (FALSE);
  }
  return (TRUE);
}

short file_exists(uchar *filename) {
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

short remove_file(uchar *filename) {
  if (filename == NULL) {
    return (RC_OK);
  }
  if (unlink((char *) filename) == (-1)) {
    return (RC_ACCESS_DENIED);
  }
  return (RC_OK);
}

  /*
   * Support an = in the following circumstances:
   *
   *    In filename      Current           Current         Substitutes
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
void convert_equals_in_filename(uchar *outfilename, uchar *infilename) {
  uchar in_filename[MAX_FILE_NAME + 1];
  uchar current_filename[MAX_FILE_NAME + 1];
  uchar *in_ftype, *in_fpath, *in_fname;
  uchar *current_ftype, *current_fpath, *current_fname;
  long last_pos;

  /*
   * If we don't have a current file, or there are no equivalence chars
   * just copy the incoming filename to the outgoing filename...
   */
  if (CURRENT_VIEW == NULL || CURRENT_FILE == NULL || strzreveq(infilename, (uchar) EQUIVCHARx) == (-1)) {
    strcpy((char *) outfilename, (char *) infilename);
    return;
  }
  /*
   * Split the incoming file name into 2 or 3 pieces; fpath/filename or fpath/fname/ftype.
   */
  strcpy((char *) in_filename, (char *) strrmdup(strtrans(infilename, OSLASH, ISLASH), EQUIVCHARx, TRUE));
  in_fpath = in_filename;
  last_pos = strzreveq(in_fpath, (uchar) ISLASH);
  if (last_pos == (-1)) {
    in_fpath = NULL;
    in_fname = in_filename;
  } else {
    in_fpath[last_pos] = '\0';
    in_fname = in_fpath + last_pos + 1;
  }
  last_pos = strzreveq(in_fname, (uchar) '.');
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
  last_pos = strzreveq(current_fpath, (uchar) ISLASH);
  if (last_pos == (-1)) {
    current_fpath = NULL;
    current_fname = current_filename;
  } else {
    current_fpath[last_pos] = '\0';
    current_fname = current_fpath + last_pos + 1;
  }
  last_pos = strzreveq(current_fname, (uchar) '.');
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

short splitpath(uchar *filename) {
  short len = 0;
  uchar work_filename[MAX_FILE_NAME + 1];
  uchar conv_filename[MAX_FILE_NAME + 1];

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
   * If the supplied filename is empty, set the path = cwd and filename equal to blank.
   */
  if (strcmp((char *) filename, "") == 0) {
    if (getcwd((char *) sp_path, MAX_FILE_NAME) == NULL) {
      return (RC_BAD_FILEID);
    }
    strcpy((char *) sp_fname, "");
  }
  /*
   * Check if the first character is tilde; translate HOME env variable if there is one.
   * Obviously only applicable to UNIX.
   */
  if (*(conv_filename) == '~') {
    if (*(conv_filename + 1) == ISLASH || *(conv_filename + 1) == '\0') {
      uchar *home = (uchar *) getenv("HOME");

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
  if ((stat((char *) work_filename, &stat_buf) == 0) && (S_ISDIR(stat_buf.st_mode))) {
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

