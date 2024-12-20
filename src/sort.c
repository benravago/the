// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

#define MAX_SORT_FIELDS 1000

#define SF_ERROR    0
#define SF_START    1
#define SF_ORDER    2
#define SF_LEFT     3

struct sort_field {
  uchar order;                  /* A - ascending, D - descending */
  long left_col;                /* left column */
  long right_col;               /* right column */
};
typedef struct sort_field SORT_FIELD;

uchar *sort_field_1;
uchar *sort_field_2;

SORT_FIELD sort_fields[MAX_SORT_FIELDS];

short num_fields;

static int cmp(const void *, const void *);

static int cmp(const void *first, const void *second) {
  long i = 0, j = 0;
  short rc = RC_OK;
  long len = 0;
  long right_col = 0, left_col = 0;
  LINE *one = *(LINE **) first;
  LINE *two = *(LINE **) second;

  /*
   * For each sort field defined in the array sort_fields,
   * compare the value of both lines for the specified column range.
   */
  for (i = 0; i < num_fields; i++) {
    /*
     * Calculate the length of the sort field.
     */
    len = sort_fields[i].right_col - sort_fields[i].left_col + 1;
    /*
     * Set the two temporary fields to blanks.
     */
    memset(sort_field_1, ' ', len);
    memset(sort_field_2, ' ', len);
    /*
     * For the first line to be compared,
     * extract the portion of the line that corresponds with the current sort column...
     */
    right_col = min(sort_fields[i].right_col, one->length);
    left_col = min(sort_fields[i].left_col, one->length);
    /*
     * If the sort column lies after the end of the line,
     * leave the contents of the sort field blank.
     */
    if (sort_fields[i].left_col <= one->length) {
      memcpy(sort_field_1, one->line + left_col - 1, right_col - left_col + 1);
    }
    /*
     * For the next line to be compared,
     * extract the portion of the line that corresponds with the current sort column...
     */
    right_col = min(sort_fields[i].right_col, two->length);
    left_col = min(sort_fields[i].left_col, two->length);
    /*
     * If the sort column lies after the end of the line,
     * leave the contents of the sort field blank.
     */
    if (sort_fields[i].left_col <= two->length) {
      memcpy(sort_field_2, two->line + left_col - 1, right_col - left_col + 1);
    }
    /*
     * If CASE IGNORE is on for the current view,
     * set both sort fields to uppercase for the comparison.
     */
    if (CURRENT_VIEW->case_sort == CASE_IGNORE) {
      for (j = 0; j < len; j++) {
        if (islower(sort_field_1[j])) {
          sort_field_1[j] = toupper(sort_field_1[j]);
        }
        if (islower(sort_field_2[j])) {
          sort_field_2[j] = toupper(sort_field_2[j]);
        }
      }
    }
    /*
     * If the two sort fields are equal,
     * continue the sort with the next sort field value.
     * If the sort fields are different,
     * return with the the comparison value (if ASCENDING)
     * or the comparison value negated (if DESCENDING).
     */
    if ((rc = strncmp((char *) sort_field_1, (char *) sort_field_2, len)) != 0) {
      return ((sort_fields[i].order == 'A') ? rc : -rc);
    }
  }
  /*
   * To get to here,
   * the result of sorting on all fields has resulted in both lines being equal.
   * Return with 0 to indicate this.
   */
  return (0);
}

#define STATE_REAL   0
#define STATE_SHADOW 1
#define SOR_PARAMS  3+(MAX_SORT_FIELDS*3)

short execute_sort(uchar *params) {
  register int i = 0;
  uchar *word[SOR_PARAMS + 1];
  uchar strip[SOR_PARAMS];
  unsigned short num_params = 0;
  LINE **lfirst = NULL, **lp = NULL;
  LINE **origfirst = NULL, **origlp = NULL;
  LINE *curr = NULL, *first = NULL;
  LINE *curr_prev = NULL, *curr_next = NULL;
  long true_line = 0L, dest_line = 0L;
  long abs_num_lines = 0L;
  long j = 0L;
  long num_actual_lines = 0L;
  long num_sorted_lines = 0L, save_num_sorted_lines = 0L;
  short rc = RC_OK, direction = DIRECTION_FORWARD;
  long left_col = 0, right_col = 0, max_column_width = 0;
  uchar order = 'A';
  TARGET target;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL | TARGET_SPARE;
  bool lines_based_on_scope = FALSE;
  short state = STATE_REAL;
  int errornum = 1;

  /*
   * Validate first argument as a target...
   */
  initialise_target(&target);
  rc = validate_target(params, &target, target_type, get_true_line(TRUE), TRUE, TRUE);
  if (rc != RC_OK) {
    free_target(&target);
    return (RC_INVALID_OPERAND);
  }
  true_line = (target.num_lines < 0L) ? target.last_line : target.true_line;
  direction = DIRECTION_FORWARD;
  abs_num_lines = ((target.num_lines < 0L) ? -target.num_lines : target.num_lines);
  lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
  /*
   * Don't need to do anything if < 2 lines to be sorted.
   */
  if (abs_num_lines < 2L) {
    display_error(55, (uchar *) "", FALSE);
    free_target(&target);
    return (RC_OK);
  }
  /*
   * Parse the remainder of the arguments and set up the sort_fields[] array with valid values.
   */
  if (target.spare != (-1)) {
    for (i = 0; i < SOR_PARAMS; i++) {
      strip[i] = STRIP_BOTH;
    }
    num_params = param_split(strtrunc(target.rt[target.spare].string), word, SOR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  }
  /*
   * Process parameters differently, depending on the number...
   *
   * 0 parameter (target only) - if BOX BLOCK, then the sort field will be the block columns,
   *                             otherwise ZONE settings will be used.
   * 1 parameters (target & order) - same as above but also validate the ordering value.
   *
   * > 1 parameters (target & sort fields) - validate each parameter.
   */
  switch (num_params) {
    case 0:
    case 1:
      sort_fields[0].left_col = CURRENT_VIEW->zone_start;
      sort_fields[0].right_col = CURRENT_VIEW->zone_end;
      sort_fields[0].order = order;
      num_fields = 1;
      if (target.rt[0].target_type == TARGET_BLOCK_CURRENT && MARK_VIEW->mark_type == M_BOX) {
        sort_fields[0].left_col = MARK_VIEW->mark_start_col;
        sort_fields[0].right_col = MARK_VIEW->mark_end_col;
      }
      /*
       * No more processing if only 1 parameter.
       */
      if (num_params == 0) {
        break;
      }
      /*
       * Processing for 2 parameters; validate ordering value.
       */
      if (equal((uchar *) "ascending", word[0], 1) || equal((uchar *) "descending", word[0], 1)) {
        order = word[0][0];
        if (islower(order)) {
          order = toupper(order);
        }
        sort_fields[0].order = order;
        break;
      }
      /*
       * If the parameter is not Ascending or Descending, display error.
       */
      display_error(1, (uchar *) word[0], FALSE);
      free_target(&target);
      return (RC_INVALID_OPERAND);
      break;
    /*
     * More than 1 parameters; sort field(s) are being specified.
     */
    default:
      i = 0;
      num_fields = 0;
      state = SF_START;
      for (;;) {
        switch (state) {
          case SF_START:
            if (num_fields == MAX_SORT_FIELDS) {
              state = SF_ERROR;
              errornum = 75;
              break;
            }
            if (equal((uchar *) "ascending", word[i], 1) || equal((uchar *) "descending", word[i], 1)) {
              order = word[i][0];
              if (islower(order)) {
                order = toupper(order);
              }
              sort_fields[num_fields].order = order;
              state = SF_ORDER;
              i++;
              break;
            }
            left_col = atol((char *) word[i]);
            if (left_col == 0) {
              state = SF_ERROR;
              break;
            }
            sort_fields[num_fields].order = order;
            sort_fields[num_fields].left_col = left_col;
            state = SF_LEFT;
            i++;
            break;
          case SF_ORDER:
            left_col = atol((char *) word[i]);
            if (left_col < 1) {
              state = SF_ERROR;
              break;
            }
            sort_fields[num_fields].left_col = left_col;
            state = SF_LEFT;
            i++;
            if (i == num_params) {
              state = SF_ERROR;
            }
            break;
          case SF_LEFT:
            right_col = atol((char *) word[i]);
            if (right_col < 1) {
              state = SF_ERROR;
              break;
            }
            sort_fields[num_fields].right_col = right_col;
            if (right_col < left_col) {
              state = SF_ERROR;
              break;
            }
            state = SF_START;
            i++;
            num_fields++;
            break;
          default:
            state = SF_ERROR;
            break;
        }
        /*
         * If we have an error, display a message...
         */
        if (state == SF_ERROR) {
          switch (errornum) {
            case 75:
              display_error(75, (uchar *) "", FALSE);
              break;
            default:
              display_error(1, (uchar *) word[i], FALSE);
              break;
          }
          free_target(&target);
          return (RC_INVALID_OPERAND);
        }
        /*
         * If we have run out of parameters...
         */
        if (i == num_params) {
          /*
           * ...then if we have the correct number of parameters, OK.
           */
          if (state == SF_START) {
            break;
          } else {
            /*
             * ...otherwise display an error.
             */
            display_error(1, strtrunc(target.rt[target.spare].string), FALSE);
            free_target(&target);
            return (RC_INVALID_OPERAND);
          }
        }
      }
      break;
  }
  /*
   * Determine the maximum length of a sort field.
   */
  for (i = 0; i < num_fields; i++) {
    max_column_width = max(max_column_width, sort_fields[i].right_col - sort_fields[i].left_col + 1);
  }
  /*
   * Allocate memory for each of the temporary sort fields to the length
   * of the maximum field width.
   */
  if ((sort_field_1 = (uchar *) malloc(max_column_width)) == NULL) {
    display_error(30, (uchar *) "", FALSE);
    free_target(&target);
    return (RC_OUT_OF_MEMORY);
  }
  if ((sort_field_2 = (uchar *) malloc(max_column_width)) == NULL) {
    display_error(30, (uchar *) "", FALSE);
    free_target(&target);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Assign the values of the newly allocated array to the LINE pointers for the target lines.
   */
  first = curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * Allocate memory for num_lines of LINE pointers and for a copy of original lines.
   */
  if ((lfirst = (LINE **) malloc(abs_num_lines * sizeof(LINE *))) == NULL) {
    display_error(30, (uchar *) "", FALSE);
    free_target(&target);
    return (RC_OUT_OF_MEMORY);
  }
  if ((origfirst = (LINE **) malloc(3 * abs_num_lines * sizeof(LINE *))) == NULL) {
    display_error(30, (uchar *) "", FALSE);
    free_target(&target);
    return (RC_OUT_OF_MEMORY);
  }
  lp = lfirst;
  origlp = origfirst;
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
    rc = processable_line(CURRENT_VIEW, true_line + (long) (j * direction), curr);
    switch (rc) {
      case LINE_SHADOW:
        break;
      case LINE_TOF:
      case LINE_EOF:
        num_actual_lines++;
        break;
      default:
        lp[num_sorted_lines] = curr;
        *origlp++ = curr;
        *origlp++ = curr->next;
        *origlp++ = curr->prev;
        num_actual_lines++;
        num_sorted_lines++;
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
  save_num_sorted_lines = num_sorted_lines;
  origlp = origfirst;

  /*
   * Don't need to do anything if < 2 lines to be sorted.
   */
  if (num_sorted_lines < 2L) {
    display_error(55, (uchar *) "", FALSE);
  } else {
    /*
     * Sort the target array...
     */
    qsort(lfirst, num_sorted_lines, sizeof(LINE *), cmp);
    /*
     * Merge  the sorted array pointers into the linked list...
     */
    lp = lfirst;
    origlp = origfirst;
    curr = first;
    for (j = 0L, num_actual_lines = 0L, num_sorted_lines = 0L;; j++) {
      if (lines_based_on_scope) {
        if (num_actual_lines == abs_num_lines) {
          break;
        }
      } else {
        if (abs_num_lines == j) {
          break;
        }
      }
      rc = processable_line(CURRENT_VIEW, true_line + (long) (j * direction), curr);
      switch (rc) {
        case LINE_SHADOW:
          curr_prev = curr->prev;
          curr_next = curr->next;
          if (state == STATE_REAL && num_sorted_lines != 0) {
            lp[num_sorted_lines - 1L]->next = origlp[((num_sorted_lines - 1L) * 3L) + 1];
            curr->prev = lp[num_sorted_lines - 1L];
          }
          state = STATE_SHADOW;
          if (direction == DIRECTION_BACKWARD) {
            curr = curr_prev;
          } else {
            curr = curr_next;
          }
          break;
        case LINE_TOF:
        case LINE_EOF:
          num_actual_lines++;
          if (direction == DIRECTION_BACKWARD) {
            curr = curr->prev;
          } else {
            curr = curr->next;
          }
          break;
        default:
          if (num_sorted_lines == 0L) {
            lp[num_sorted_lines]->next = lp[num_sorted_lines + 1L];
            lp[num_sorted_lines]->prev = origlp[(num_sorted_lines * 3L) + 2L];
            lp[num_sorted_lines]->prev->next = lp[num_sorted_lines];
          } else {
            if (num_sorted_lines == save_num_sorted_lines - 1L) {
              lp[num_sorted_lines]->next = origlp[(num_sorted_lines * 3L) + 1L];
              lp[num_sorted_lines]->prev = lp[num_sorted_lines - 1L];
              lp[num_sorted_lines]->next->prev = lp[num_sorted_lines];
            } else {
              lp[num_sorted_lines]->next = lp[num_sorted_lines + 1L];
              lp[num_sorted_lines]->prev = lp[num_sorted_lines - 1L];
            }
          }
          if (state == STATE_SHADOW) {
            lp[num_sorted_lines]->prev = origlp[(num_sorted_lines * 3L) + 2L];
            lp[num_sorted_lines]->prev->next = lp[num_sorted_lines];
          }
          state = STATE_REAL;
          if (direction == DIRECTION_BACKWARD) {
            curr = origlp[(num_sorted_lines * 3L) + 2L];
          } else {
            curr = origlp[(num_sorted_lines * 3L) + 1L];
          }
          num_actual_lines++;
          num_sorted_lines++;
          break;
      }
      /*
       * Proceed to the next record, even if the current record not in scope.
       */
      if (curr == NULL) {
        break;
      }
    }
    /*
     * If STAY is OFF, change the current and focus lines by the number of lines calculated from the target.
     */
    if (!CURRENT_VIEW->stay && abs_num_lines != 0L) { /* stay is off */
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW, NULL, dest_line, direction);
    }
    /*
     * Increment alteration count...
     */
    increment_alt(CURRENT_FILE);
    sprintf((char *) temp_cmd, "%ld line(s) sorted", abs_num_lines);
    display_error(0, temp_cmd, TRUE);
  }
  /*
   * Free up the memory used for the sort fields and the target array.
   */
  free(sort_field_1);
  free(sort_field_2);
  free(lfirst);
  free(origfirst);
  free_target(&target);
  return (RC_OK);
}

