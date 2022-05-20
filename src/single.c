/* SINGLE.C - Processing for single input mode                         */
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

static LENGTHTYPE tmp_len;
static CHARTYPE tmp_str[2 * MAX_FILE_NAME + 100];

/*
 * Most Unixen; determine where select() is
 */
#include <sys/select.h>

static int fifo_fd;

int initialise_fifo(LINE * first_file_name, LINETYPE startup_line, LENGTHTYPE startup_column, bool ro) {
  LINE *current_file_name;
  int am_client = 0, rc;
  char *ronly;

  if (file_exists(fifo_name) == THE_FILE_EXISTS) {
    /*
     * The FIFO exists, so we assume we are the client here...
     */
    if (file_writable(fifo_name)) {
      fifo_fd = open((DEFCHAR *) fifo_name, O_WRONLY);
      if (fifo_fd == (-1)) {
        display_error(0, (CHARTYPE *) "Warning: Unable to run in single instance mode: open() failed", FALSE);
        display_error(0, (CHARTYPE *) strerror(errno), FALSE);
      } else {
        current_file_name = first_file_name;
        while (current_file_name != NULL) {
          if ((rc = splitpath(current_file_name->line)) != RC_OK) {
            display_error(10, current_file_name->line, FALSE);
          } else {
            /*
             * If started with -r, set READONLY FORCE
             */
            if (ro)
              ronly = "#readonly force";
            else
              ronly = "";
            /*
             * If line and/or column specified on command line, use
             * them to reposition file...
             */
            if (startup_line != 0L || startup_column != 0) {
              tmp_len = sprintf((DEFCHAR *) tmp_str, "x %s%s#cursor goto %ld %ld%s", sp_path, sp_fname, (startup_line) ? startup_line : 1, (startup_column) ? startup_column : 1, ronly);
            } else {
              tmp_len = sprintf((DEFCHAR *) tmp_str, "x %s%s%s", sp_path, sp_fname, ronly);
            }
            if (write(fifo_fd, &tmp_len, sizeof(tmp_len)) == (-1)) {
              display_error(0, (CHARTYPE *) strerror(errno), FALSE);
            }
            if (write(fifo_fd, tmp_str, tmp_len) == (-1)) {
              display_error(0, (CHARTYPE *) strerror(errno), FALSE);
            }
          }
          current_file_name = current_file_name->next;
        }
        close(fifo_fd);
        /*
         * We are the client, so return with 1 to enable the caller to stop
         */
        am_client = 1;
      }
    } else {
      display_error(0, (CHARTYPE *) "Warning: Unable to run in single instance mode: fifo not writable", FALSE);
      display_error(0, (CHARTYPE *) strerror(errno), FALSE);
    }
  } else {
    /*
     * The FIFO doesn't exists, so we assume we are the server here...
     */
    if (mkfifo((DEFCHAR *) fifo_name, S_IWUSR | S_IRUSR) == (-1)) {
      display_error(0, (CHARTYPE *) "Warning: Unable to run in single instance mode: mkfifo() failed", FALSE);
      display_error(0, (CHARTYPE *) strerror(errno), FALSE);
    } else {
      fifo_fd = open((DEFCHAR *) fifo_name, O_RDWR);
      if (fifo_fd == -1) {
        display_error(0, (CHARTYPE *) "Warning: Unable to run in single instance mode open() failed:", FALSE);
        display_error(0, (CHARTYPE *) strerror(errno), FALSE);
      } else {
        single_instance_server = TRUE;
      }
    }
    /*
     * We are the server, so return with 0 to enable the caller to continue...
     */
  }
  return am_client;
}

int process_fifo_input(int key) {
  int s;
  fd_set readfds;
  int curses_fd;
  bool le_status = CURRENT_VIEW->linend_status;
  CHARTYPE le_value = CURRENT_VIEW->linend_value;
  VIEW_DETAILS *le_view;

  if (key == -1) {
    /*
     *
     * Add curses input and the input fifo
     */
    curses_fd = fileno(stdin);
    FD_ZERO(&readfds);
    FD_SET(curses_fd, &readfds);
    FD_SET(fifo_fd, &readfds);
    if ((s = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) < 0) {
      return key;
    }
    if (s == 0) {               /* no requests pending - should never happen! */
      return key;
    }
    if (FD_ISSET(fifo_fd, &readfds)) {
      if (read(fifo_fd, (char *) &tmp_len, sizeof(tmp_len)) < 0) {
        return key;
      }
      /*
       * Sleep for 100 milliseconds to ensure the remainder of the
       * data is in the fifo. Yuck!
       */
      napms(100);
      if (read(fifo_fd, tmp_str, tmp_len * sizeof(CHARTYPE)) < 0) {
        return key;
      }
      /*
       * Save the current linend.
       * Force LINEND to # (which is what initialise_lifo() sets).
       * Run the command
       * set LINEND back to default.
       */
      tmp_str[tmp_len] = '\0';
      le_status = CURRENT_VIEW->linend_status;
      le_value = CURRENT_VIEW->linend_value;
      le_view = CURRENT_VIEW;
      CURRENT_VIEW->linend_status = TRUE;
      CURRENT_VIEW->linend_value = '#';
      (void) command_line(tmp_str, TRUE);
      le_view->linend_status = le_status;
      le_view->linend_value = le_value;
      THERefresh((CHARTYPE *) "");
      key = 0;
    }
  }
  return key;
}

void close_fifo(void) {
  close(fifo_fd);
  remove_file(fifo_name);
  return;
}
