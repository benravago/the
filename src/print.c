// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Printing related functions
 */

#include "the.h"
#include "proto.h"

static void print_shadow_line(FILE *, char_t *, line_t);

void print_line(bool close_spooler, line_t true_line, line_t num_lines, short pagesize, char_t * text, char_t * line_term, short target_type) {
  static bool spooler_open = FALSE;
  static FILE *pp;
  short rc = RC_OK;
  line_t j = 0L;
  LINE *curr = NULL;
  short line_number = 0;
  line_t num_excluded = 0L;
  line_t num_actual_lines = 0L;
  line_t abs_num_lines = (num_lines < 0L ? -num_lines : num_lines);
  short direction = (num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
  unsigned short y = 0, x = 0;
  bool lines_based_on_scope = (target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
  line_t start = 0L, end = 0L, len = 0L;
  char_t *ptr = NULL;

  if (close_spooler) {
    if (spooler_open) {
      spooler_open = FALSE;
      pclose(pp);
      return;
    }
  }

  if (!spooler_open) {
    pp = popen((char *) spooler_name, "w");
    if (pp == NULL) {
      return;
    }
    spooler_open = TRUE;
  }

  if (num_lines == 0L) {
    fprintf(pp, "%s%s", text, line_term);
    return;
  }
  /*
   * Once we get here, we are to print lines from the file.
   */
  post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
  if (curses_started) {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      getyx(CURRENT_WINDOW_FILEAREA, y, x);
    } else {
      getyx(CURRENT_WINDOW, y, x);
    }
  }
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  for (j = 0L, num_actual_lines = 0L;; j++) {
    if (lines_based_on_scope) {
      if (num_actual_lines == abs_num_lines) {
        break;
      }
    } else {
      if (abs_num_lines == j) {
        break;
      }
    }
    rc = processable_line(CURRENT_VIEW, true_line + (line_t) (j * direction), curr);
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
                pre_process_line(CURRENT_VIEW, true_line + (line_t) (j * direction), curr);
                start = MARK_VIEW->mark_start_col - 1;
                end = MARK_VIEW->mark_end_col - 1;
                len = end - start + 1L;
                ptr = rec + start;
                break;

              case M_STREAM:
              case M_CUA:
                pre_process_line(CURRENT_VIEW, true_line + (line_t) (j * direction), curr);
                start = 0;
                end = (curr->length) - 1;
                if (true_line + (line_t) (j * direction) == MARK_VIEW->mark_start_line) {
                  start = MARK_VIEW->mark_start_col - 1;
                }
                if (true_line + (line_t) (j * direction) == MARK_VIEW->mark_end_line) {
                  end = MARK_VIEW->mark_end_col - 1;
                }
                len = end - start + 1L;
                ptr = curr->line + start;
                break;
            }
            break;

          default:
            if (curr->length == 0) {
              len = 0L;
            } else {
              start = (line_t) CURRENT_VIEW->zone_start - 1;
              end = (line_t) min((curr->length) - 1, CURRENT_VIEW->zone_end - 1);
              ptr = curr->line + start;
              if (start > end) {
                len = 0L;
              } else {
                len = end - start + 1L;
              }
            }
            break;
        }
        fwrite((char *) ptr, sizeof(char_t), len, pp);
        fprintf(pp, "%s", line_term);
        line_number++;
        if (line_number == pagesize && pagesize != 0) {
          fputc('\f', pp);
          line_number = 0;
        }
        num_actual_lines++;
        break;
    }
    /*
     * Proceed to the next record, even if the current record not in scope.
     */
    if (direction == DIRECTION_BACKWARD) {
      curr = curr->prev;
    } else {
      curr = curr->next;
    }
    if (curr == NULL) {
      break;
    }
  }
  /*
   * If we have a shadow line remaining, print it...
   */
  if (num_excluded != 0) {
    print_shadow_line(pp, line_term, num_excluded);
    num_excluded = 0L;
  }
  /*
   * If STAY is OFF, change the current and focus lines by the number
   * of lines calculated from the target.
   */
  if (!CURRENT_VIEW->stay) {    /* stay is off */
    CURRENT_VIEW->focus_line = min(CURRENT_VIEW->focus_line + num_lines - 1L, CURRENT_FILE->number_lines + 1L);
    CURRENT_VIEW->current_line = min(CURRENT_VIEW->current_line + num_lines - 1L, CURRENT_FILE->number_lines + 1L);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  if (curses_started) {
    y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND) {
      wmove(CURRENT_WINDOW_FILEAREA, y, x);
    } else {
      wmove(CURRENT_WINDOW, y, x);
    }
  }
  return;
}

#define LINES_NOT_DISPLAYED " line(s) not displayed "

static void make_shadow_line(char *buf, line_t num_excluded, int width) {
  int numlen = 0, first = 0;
  char numbuf[33];              /* 10 + length of LINES_NOT_DISPLAYED */

  numlen = sprintf(numbuf, " %ld%s", num_excluded, LINES_NOT_DISPLAYED);
  if (numlen > width) {
    numlen = width;
  }
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

static void print_shadow_line(FILE * pp, char_t * line_term, line_t num_excluded) {
  register int width = 0;
  char buf[512];

  if (CURRENT_VIEW->shadow) {
    width = min(sizeof(buf) - 1, CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
    make_shadow_line(buf, num_excluded, width);
    fwrite(buf, width, 1, pp);
    fputs((char *) line_term, pp);
  }
  return;
}

