/* PRINT.C - Printing related functions                                */
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

static void print_shadow_line(FILE *, CHARTYPE *, LINETYPE);

void print_line(bool close_spooler, LINETYPE true_line, LINETYPE num_lines, short pagesize, CHARTYPE * text, CHARTYPE * line_term, short target_type) {
/*--------------------------- local data ------------------------------*/
  static bool spooler_open = FALSE;
  static FILE *pp;
  short rc = RC_OK;
  LINETYPE j = 0L;
  LINE *curr = NULL;
  short line_number = 0;
  LINETYPE num_excluded = 0L;
  LINETYPE num_actual_lines = 0L;
  LINETYPE abs_num_lines = (num_lines < 0L ? -num_lines : num_lines);
  short direction = (num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  unsigned short y = 0, x = 0;
  bool lines_based_on_scope = (target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
  LINETYPE start = 0L, end = 0L, len = 0L;
  CHARTYPE *ptr = NULL;

/*--------------------------- processing ------------------------------*/

  if (close_spooler) {
    if (spooler_open) {
      spooler_open = FALSE;
      pclose(pp);
      return;
    }
  }

  if (!spooler_open) {
    pp = popen((DEFCHAR *) spooler_name, "w");
    if (pp == NULL) {
      return;
    }
    spooler_open = TRUE;
  }

  if (num_lines == 0L) {
    fprintf(pp, "%s%s", text, line_term);
    return;
  }
/*---------------------------------------------------------------------*/
/* Once we get here, we are to print lines from the file.              */
/*---------------------------------------------------------------------*/
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (curses_started) {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      getyx(CURRENT_WINDOW_FILEAREA, y, x);
    else
      getyx(CURRENT_WINDOW, y, x);
  }
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  for (j = 0L, num_actual_lines = 0L;; j++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines)
        break;
    } else {
      if (abs_num_lines == j)
        break;
    }
    rc = processable_line(CURRENT_VIEW, true_line + (LINETYPE) (j * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        num_excluded++;
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        if (num_excluded != 0) {
          print_shadow_line(pp, line_term, num_excluded);
          num_excluded = 0L;
        }
        switch (target_type) {
          case TARGET_BLOCK_CURRENT:
            switch (MARK_VIEW->mark_type) {
              case M_LINE:
                start = 0;
                end = (curr->length) - 1;
                len = end - start + 1L;
                ptr = curr->line;
                break;
              case M_BOX:
              case M_WORD:
              case M_COLUMN:
                pre_process_line(CURRENT_VIEW, true_line + (LINETYPE) (j * direction), curr);
                start = MARK_VIEW->mark_start_col - 1;
                end = MARK_VIEW->mark_end_col - 1;
                len = end - start + 1L;
                ptr = rec + start;
                break;
              case M_STREAM:
              case M_CUA:
                pre_process_line(CURRENT_VIEW, true_line + (LINETYPE) (j * direction), curr);
                start = 0;
                end = (curr->length) - 1;
                if (true_line + (LINETYPE) (j * direction) == MARK_VIEW->mark_start_line)
                  start = MARK_VIEW->mark_start_col - 1;
                if (true_line + (LINETYPE) (j * direction) == MARK_VIEW->mark_end_line)
                  end = MARK_VIEW->mark_end_col - 1;
                len = end - start + 1L;
                ptr = curr->line + start;
                break;
            }
            break;
          default:
            if (curr->length == 0)
              len = 0L;
            else {
              start = (LINETYPE) CURRENT_VIEW->zone_start - 1;
              end = (LINETYPE) min((curr->length) - 1, CURRENT_VIEW->zone_end - 1);
              ptr = curr->line + start;
              if (start > end)
                len = 0L;
              else
                len = end - start + 1L;
            }
            break;
        }
        fwrite((DEFCHAR *) ptr, sizeof(CHARTYPE), len, pp);
        fprintf(pp, "%s", line_term);
        line_number++;
        if (line_number == pagesize && pagesize != 0) {
          fputc('\f', pp);
          line_number = 0;
        }
        num_actual_lines++;
        break;
    }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
    if (direction == DIRECTION_BACKWARD)
      curr = curr->prev;
    else
      curr = curr->next;
    if (curr == NULL)
      break;
  }
/*---------------------------------------------------------------------*/
/* If we have a shadow line remaining, print it...                     */
/*---------------------------------------------------------------------*/
  if (num_excluded != 0) {
    print_shadow_line(pp, line_term, num_excluded);
    num_excluded = 0L;
  }
/*---------------------------------------------------------------------*/
/* If STAY is OFF, change the current and focus lines by the number    */
/* of lines calculated from the target.                                */
/*---------------------------------------------------------------------*/
  if (!CURRENT_VIEW->stay) {    /* stay is off */
    CURRENT_VIEW->focus_line = min(CURRENT_VIEW->focus_line + num_lines - 1L, CURRENT_FILE->number_lines + 1L);
    CURRENT_VIEW->current_line = min(CURRENT_VIEW->current_line + num_lines - 1L, CURRENT_FILE->number_lines + 1L);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  if (curses_started) {
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      wmove(CURRENT_WINDOW_FILEAREA, y, x);
    else
      wmove(CURRENT_WINDOW, y, x);
  }
  return;
}

static void make_shadow_line(char *buf, LINETYPE num_excluded, int width) {
#define LINES_NOT_DISPLAYED " line(s) not displayed "
  int numlen = 0, first = 0;
  char numbuf[33];              /* 10 + length of LINES_NOT_DISPLAYED */

  numlen = sprintf(numbuf, " %ld%s", num_excluded, LINES_NOT_DISPLAYED);
  if (numlen > width)
    numlen = width;
  /* distribute pad characters */
  first = (width - numlen) >> 1;
  memset(buf, '-', first);
  buf += first;
  memcpy(buf, numbuf, numlen);
  buf += numlen;
  /* fill up to end */
  memset(buf, '-', width - first - numlen);
  /* terminate string */
  buf[width - first - numlen] = '\0';
  return;
}

static void print_shadow_line(FILE * pp, CHARTYPE * line_term, LINETYPE num_excluded) {
  register int width = 0;
  char buf[512];

  if (CURRENT_VIEW->shadow) {
    width = min(sizeof(buf) - 1, CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
    make_shadow_line(buf, num_excluded, width);
    fwrite(buf, width, 1, pp);
    fputs((DEFCHAR *) line_term, pp);
  }
  return;
}

short setprintername(char *pn) {
  short rc = RC_OK;

  return (rc);
}

short setfontcpi(int cpi) {
  return (RC_OK);
}

short setfontlpi(int lpi) {
  return (RC_OK);
}

short setpagesize(int fs) {
  return (RC_OK);
}

short setfontname(char *font) {
  return (RC_OK);
}

short setorient(char ori) {
  return (RC_OK);
}
