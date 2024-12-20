// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

#define STATE_START         0
#define STATE_DELIM         1
#define STATE_STRING        2
#define STATE_BOOLEAN       3
#define STATE_NEXT          4
#define STATE_POINT         5
#define STATE_ABSOLUTE      6
#define STATE_RELATIVE      7
#define STATE_POSITIVE      8
#define STATE_NEGATIVE      9
#define STATE_REGEXP_START 10
#define STATE_REGEXP       11
#define STATE_SPARE        12
#define STATE_QUIT         98
#define STATE_ERROR        99

static bool is_blank(LINE *);

/*
 * Return the length of ptr that matches from the minlen of type.
 * e.g. chan, changed, 3 will result in 4
 * If no match, return 0
 */
static int target_type_match(uchar *ptr, uchar *type, int minlen) {
  int i, result = 0, maxlen;

  maxlen = strlen((char *) type);
  for (i = minlen; i <= maxlen; i++) {
    if (memcmpi(type, ptr, i) == 0 && (*(ptr + (i)) == ' ' || *(ptr + (i)) == '\0' || *(ptr + (i)) == '\t')) {
      result = i;
    }
  }
  return result;
}

#define SCP_PARAMS  2

short split_change_params(uchar *cmd_line, uchar **old_str, uchar **new_str, TARGET *target, long *num, long *occ) {
  uchar *word[SCP_PARAMS + 1];
  uchar strip[SCP_PARAMS];
  long i = 0, j = 0;
  uchar *target_start = NULL;
  short rc = RC_OK;
  uchar delim = ' ';
  long len = strlen((char *) cmd_line), idx = 0;
  long target_type = TARGET_NORMAL | TARGET_BLOCK_CURRENT | TARGET_ALL | TARGET_SPARE;
  unsigned short num_params = 0;
  uchar buffer[100];

  /*
   * First, determine the delimiter; the first non-blank character in the argument string.
   */
  for (i = 0; i < len; i++) {
    if (*(cmd_line + i) != ' ') {
      delim = *(cmd_line + i);
      idx = i + 1;
      break;
    }
  }
  /*
   * Set up default values for the return values...
   */
  *old_str = cmd_line + idx;
  *new_str = (uchar *) "";
  target_start = (uchar *) "";
  *num = *occ = 1L;
  target->num_lines = 1L;
  target->true_line = get_true_line(TRUE);
  /*
   * Set up default values for the return values...
   */
  for (i = idx, j = 0; i < len; i++) {
    if (*(cmd_line + i) == delim) {
      j++;
      switch (j) {
        case 1:
          *(cmd_line + i) = '\0';
          *new_str = cmd_line + i + 1;
          break;
        case 2:
          *(cmd_line + i) = '\0';
          target_start = cmd_line + i + 1;
          break;
        default:
          break;
      }
    }
    if (j == 2) {
      break;
    }
  }
  /*
   * Check to see if there is a target, if not return here.
   */
  if (blank_field(target_start)) {
    return (RC_OK);
  }
  /*
   * Parse and validate the target...
   */
  if ((rc = validate_target(target_start, target, target_type, get_true_line(TRUE), TRUE, TRUE)) != RC_OK) {
    return (rc);
  }
  /*
   * Check to see if there are further arguments after the target...
   */
  if (target->spare == (-1)) {
    return (RC_OK);
  }
  /*
   * Validate the arguments following the target...
   */
  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(strtrunc(target->rt[target->spare].string), word, SCP_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 1 || num_params == 2) {
    if (strcmp((char *) word[0], "*") == 0) {
      *num = max_line_length;
    } else {
      if ((rc = valid_positive_integer_against_maximum(word[0], MAX_WIDTH_NUM)) != 0) {
        if (rc == 4) {
          sprintf((char *) buffer, "%s", word[0]);
        } else {
          sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
        }
        display_error(rc, buffer, FALSE);
        return (RC_INVALID_OPERAND);
      } else {
        *num = min(atol((char *) word[0]), max_line_length);
      }
    }
  }
  if (num_params == 2) {
    if ((rc = valid_positive_integer_against_maximum(word[1], MAX_WIDTH_NUM)) != 0) {
      if (rc == 4) {
        sprintf((char *) buffer, "%s", word[1]);
      } else {
        sprintf((char *) buffer, "- MUST be <= %ld", MAX_WIDTH_NUM);
      }
      display_error(rc, buffer, FALSE);
      return (RC_INVALID_OPERAND);
    } else {
      *occ = atol((char *) word[1]);
    }
  }
  return (RC_OK);
}

short parse_target(uchar *target_spec, long true_line, TARGET *target, long target_types, bool display_parse_error, bool allow_error_display, bool column_target) {
  short num_targets = 0;
  uchar boolean = ' ';
  short state = STATE_NEXT;
  long len = 0, inc = 0, target_length = strlen((char *) target_spec), off = 0;
  uchar delim = ' ';
  long i = 0;
  long j = 0;
  long k = 0;
  long str_start = 0, str_end = 0;
  short rc = RC_OK;
  long potential_spare_start = 0;
  bool negative = FALSE;
  uchar *ptr = NULL;
  long lineno = 0L;
  char regexp[7] = "REGEXP";

  /*
   * Copy the incoming target specification...
   */
  if ((target->string = (uchar *) strdup((char*)target_spec)) == NULL) {
    if (allow_error_display) {
      display_error(30, (uchar *) "", FALSE);
    }
    return (RC_OUT_OF_MEMORY);
  }
  ptr = target->string;
  /*
   * Parse the target...
   */
  switch (target_types) {
    case TARGET_FIND:
    case TARGET_NFIND:
    case TARGET_FINDUP:
    case TARGET_NFINDUP:
      for (i = 0; i < target_length; i++) {
        if (*(ptr + i) == ' ') {
          *(ptr + i) = CURRENT_VIEW->arbchar_single;
        } else if (*(ptr + i) == '_') {
          *(ptr + i) = ' ';
        }
      }
      target->rt = (RTARGET *) malloc(sizeof(RTARGET));
      if (target->rt == NULL) {
        if (allow_error_display) {
          display_error(30, (uchar *) "", FALSE);
        }
        return (RC_OUT_OF_MEMORY);
      }
      memset(target->rt, 0, sizeof(RTARGET));
      target->num_targets = 1;
      target->rt[0].not_target = (target_types == TARGET_NFIND || target_types == TARGET_NFINDUP) ? TRUE : FALSE;
      target->rt[0].negative = (target_types == TARGET_FINDUP || target_types == TARGET_NFINDUP) ? TRUE : FALSE;
      target->rt[0].boolean = ' ';
      target->rt[0].found = FALSE;
      target->rt[0].length = target_length;
      target->rt[0].target_type = TARGET_STRING;
      target->rt[0].numeric_target = 0L;
      target->rt[0].have_compiled_re = FALSE;
      target->rt[0].string = (uchar *) malloc((target_length * sizeof(uchar)) + 1);
      if (target->rt[0].string == (uchar *) NULL) {
        if (allow_error_display) {
          display_error(30, (uchar *) "", FALSE);
        }
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) target->rt[0].string, (char *) ptr);
      return (RC_OK);
      break;
    default:
      break;
  }
  for (;;) {
    inc = 1;
    switch (state) {
      case STATE_NEXT:
        if (target->rt == NULL) {
          target->rt = (RTARGET *) malloc((num_targets + 1) * sizeof(RTARGET));
        } else {
          target->rt = (RTARGET *) realloc(target->rt, (num_targets + 1) * sizeof(RTARGET));
        }
        if (target->rt == NULL) {
          if (allow_error_display) {
            display_error(30, (uchar *) "", FALSE);
          }
          return (RC_OUT_OF_MEMORY);
        }
        memset(&target->rt[num_targets], 0, sizeof(RTARGET));
        target->rt[num_targets].not_target = FALSE;
        target->rt[num_targets].boolean = boolean;
        target->rt[num_targets].found = FALSE;
        target->rt[num_targets].length = 0;
        target->rt[num_targets].string = NULL;
        target->rt[num_targets].negative = target->rt[0].negative;      /* was FALSE; */
        target->rt[num_targets].target_type = TARGET_ERR;
        target->rt[num_targets].numeric_target = 0L;
        target->rt[num_targets].have_compiled_re = FALSE;
        if (target->spare != (-1)) {
          state = STATE_SPARE;
        } else {
          state = STATE_START;
        }
        inc = 0;
        break;
      case STATE_START:
        switch (*(ptr + i)) {
          case '~':
          case '^':
            if (target->rt[num_targets].not_target) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            target->rt[num_targets].not_target = TRUE;
            break;
          case ' ':
          case '\t':
            break;
          case '-':
            target->rt[num_targets].negative = TRUE;
            state = STATE_NEGATIVE;
            break;
          case 'r':
          case 'R':
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            for (k = 0; k < 6; k++) {
              if (i + k > target_length) {
                state = STATE_ERROR;
                break;
              }
              if (toupper(*(ptr + i + k)) == regexp[k]) {
                continue;
              } else if (*(ptr + (i + k)) == ' ' || *(ptr + (i + k)) == '\0' || *(ptr + (i + k)) == '\t') {
                state = STATE_REGEXP_START;
                break;
              } else {
                state = STATE_ERROR;
                break;
              }
            }
            switch (state) {
              case STATE_START:
                if (*(ptr + (i + k)) == ' ' || *(ptr + (i + k)) == '\0' || *(ptr + (i + k)) == '\t') {
                  state = STATE_REGEXP_START;
                  inc = k;
                  break;
                } else {
                  state = STATE_ERROR;
                  inc = 0;
                  break;
                }
                break;
              case STATE_REGEXP_START:
                inc = k;
                break;
              default:
                state = STATE_ERROR;
                inc = 0;
                break;
            }
            potential_spare_start = i + k + 1;
            break;
          case '+':
            if (target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            // inc = 1;
            target->rt[num_targets].negative = FALSE;
            state = STATE_POSITIVE;
            break;
          case '.':
            if (target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            state = STATE_POINT;
            str_start = i;
            break;
          case '*':
            state = STATE_BOOLEAN;
            target->rt[num_targets].target_type = TARGET_RELATIVE;
            target->rt[num_targets].string = (uchar *) malloc(3);
            if (target->rt[num_targets].string == NULL) {
              if (allow_error_display) {
                display_error(30, (uchar *) "", FALSE);
              }
              return (RC_OUT_OF_MEMORY);
            }
            if (target->rt[num_targets].negative) {
              if (column_target) {
                target->rt[num_targets].numeric_target = (long) CURRENT_VIEW->zone_start - true_line - 1L;
              } else {
                target->rt[num_targets].numeric_target = true_line * (-1L);
              }
              strcpy((char *) target->rt[num_targets].string, "-*");
            } else {
              if (column_target) {
                target->rt[num_targets].numeric_target = (long) ((long) CURRENT_VIEW->zone_end - true_line) + 1L;
              } else {
                target->rt[num_targets].numeric_target = (CURRENT_FILE->number_lines - true_line) + 2L;
              }
              strcpy((char *) target->rt[num_targets].string, "*");
            }
            inc = 1;
            potential_spare_start = i + 1;
            num_targets++;
            break;
          case ':':
          case ';':
            state = STATE_ABSOLUTE;
            delim = *(ptr + i);
            str_start = i + 1;
            break;
          /* following are delimiters; make sure you change the other lists */
          case '/':
          case '\\':
          case '@':
          case '`':
          case '#':
          case '$':
          case '%':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '"':
          case '\'':
          case '<':
          case '>':
          case ',':
          case 255:
            state = STATE_STRING;
            str_start = i + 1;
            delim = *(ptr + i);
            break;
          case 'a':
          case 'A':
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            if (target_length - i < 3) {
              target->rt[num_targets].target_type = TARGET_ERR;
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            /*
             * ALL target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "all", 3);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_ALL;
              state = STATE_BOOLEAN;
              num_targets++;
              potential_spare_start = i + inc;
              break;
            }
            /*
             * ALTERED target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "altered", 3);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_ALTERED;
              potential_spare_start = i + inc;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
          case 'b':
          case 'B':
            if (target_length - i < 5) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            /*
             * BLANK target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "blank", 5);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_BLANK;
              potential_spare_start = i + inc;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            /*
             * BLOCK target
             */
            if (memcmpi((uchar *) "block", ptr + i, 5) == 0 && (*(ptr + (i + 5)) == ' ' || *(ptr + (i + 5)) == '\0' || *(ptr + (i + 5)) == '\t')) {
              target->rt[num_targets].target_type = TARGET_BLOCK;
              inc = 5;
              potential_spare_start = i + 5;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
          case 'n':
          case 'N':
            if (target_length - i < 3) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            /*
             * NEW target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "new", 3);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_NEW;
              potential_spare_start = i + inc;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
          case 'c':
          case 'C':
            if (target_length - i < 3) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            /*
             * CHANGED target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "changed", 3);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_CHANGED;
              potential_spare_start = i + inc;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
          case 't':
          case 'T':
            if (target_length - i < 3) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            /*
             * TAGGED target
             */
            inc = target_type_match((uchar *) ptr + i, (uchar *) "tagged", 3);
            if (inc != 0) {
              target->rt[num_targets].target_type = TARGET_TAGGED;
              potential_spare_start = i + inc;
              state = STATE_BOOLEAN;
              num_targets++;
              break;
            }
            if (target->rt[num_targets].not_target || target->rt[num_targets].negative) {
              state = STATE_ERROR;
              inc = 0;
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if (UNTAAx) {
              state = STATE_ABSOLUTE;
            } else {
              state = STATE_RELATIVE;
            }
            str_start = i;
            delim = '\0';
            inc = 0;
            break;
          default:
            state = STATE_ERROR;
            inc = 0;
            break;
        }
        break;
      case STATE_REGEXP_START:
        switch (*(ptr + i)) {
          case ' ':
          case '\t':
            break;
          /* following are delimiters; make sure you change the other lists */
          case '/':
          case '\\':
          case '@':
          case '`':
          case '#':
          case '$':
          case '%':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '"':
          case '\'':
          case '<':
          case '>':
          case ',':
          case 255:
            state = STATE_REGEXP;
            str_start = i + 1;
            delim = *(ptr + i);
            break;
          default:
            state = STATE_ERROR;
            inc = 0;
            break;
        }
        break;
      case STATE_REGEXP:
        switch (*(ptr + i)) {
          /* following are delimiters; make sure you change the other lists */
          case '/':
          case '\\':
          case '@':
          case '`':
          case '#':
          case '$':
          case '%':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '"':
          case '\'':
          case '<':
          case '>':
          case ',':
          case 255:
          case '\0':
            if (*(ptr + i) == delim || *(ptr + i) == '\0') {
              state = STATE_BOOLEAN;
              str_end = i;
              len = str_end - str_start;
              target->rt[num_targets].string = (uchar *) malloc(len + 1);
              if (target->rt[num_targets].string == NULL) {
                if (allow_error_display) {
                  display_error(30, (uchar *) "", FALSE);
                }
                return (RC_OUT_OF_MEMORY);
              }
              memcpy(target->rt[num_targets].string, ptr + str_start, len);
              target->rt[num_targets].string[len] = '\0';
              target->rt[num_targets].target_type = TARGET_REGEXP;
              potential_spare_start = i + 1;
              num_targets++;
            }
            break;
          default:
            break;
        }
        break;
      case STATE_STRING:
        switch (*(ptr + i)) {
          /* following are delimiters; make sure you change the other lists */
          case '/':
          case '\\':
          case '@':
          case '`':
          case '#':
          case '$':
          case '%':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '"':
          case '\'':
          case '<':
          case '>':
          case ',':
          case 255:
          case '\0':
            if (*(ptr + i) == delim || *(ptr + i) == '\0') {
              state = STATE_BOOLEAN;
              str_end = i;
              len = str_end - str_start;
              target->rt[num_targets].string = (uchar *) malloc(len + 1);
              if (target->rt[num_targets].string == NULL) {
                if (allow_error_display) {
                  display_error(30, (uchar *) "", FALSE);
                }
                return (RC_OUT_OF_MEMORY);
              }
              memcpy(target->rt[num_targets].string, ptr + str_start, len);
              target->rt[num_targets].string[len] = '\0';
              target->rt[num_targets].length = len;
              target->rt[num_targets].target_type = TARGET_STRING;
              potential_spare_start = i + 1;
              num_targets++;
            }
            break;
          default:
            break;
        }
        break;
      case STATE_BOOLEAN:
        switch (*(ptr + i)) {
          case '\0':
            break;
          case ' ':
          case '\t':
            break;
          case '&':
          case '|':
            state = STATE_NEXT;
            boolean = *(ptr + i);
            break;
          default:
            if (target_types & TARGET_SPARE) {
              str_start = potential_spare_start;
              state = STATE_NEXT;
              target->spare = 0;        /* just to ensure state is set */
              break;
            }
            state = STATE_ERROR;
            inc = 0;
            break;
        }
        break;
      case STATE_SPARE:
        switch (*(ptr + i)) {
          case '\0':
            str_end = i;
            len = str_end - str_start;
            target->rt[num_targets].string = (uchar *) malloc(len + 1);
            if (target->rt[num_targets].string == NULL) {
              if (allow_error_display) {
                display_error(30, (uchar *) "", FALSE);
              }
              return (RC_OUT_OF_MEMORY);
            }
            memcpy(target->rt[num_targets].string, ptr + str_start, len);
            target->rt[num_targets].string[len] = '\0';
            target->rt[num_targets].length = len;
            target->rt[num_targets].target_type = TARGET_SPARE;
            target->spare = num_targets;
            num_targets++;
            *(ptr + str_start) = '\0';  /* so target string does not include spare */
            break;
          default:
            break;
        }
        break;
      case STATE_ABSOLUTE:
      case STATE_RELATIVE:
        if (target->rt[num_targets].not_target) {
          state = STATE_ERROR;
          inc = 0;
          break;
        }
        switch (*(ptr + i)) {
          case '\0':
          case ' ':
          case '\t':
            str_end = i;
            len = str_end - str_start;
            target->rt[num_targets].string = (uchar *) malloc(len + 2);
            if (target->rt[num_targets].string == NULL) {
              if (allow_error_display) {
                display_error(30, (uchar *) "", FALSE);
              }
              return (RC_OUT_OF_MEMORY);
            }
            if (delim != '\0') {
              target->rt[num_targets].string[0] = delim;
              off = 1;
            } else {
              off = 0;
            }
            memcpy(target->rt[num_targets].string + off, ptr + str_start, len);
            target->rt[num_targets].string[len + off] = '\0';
            target->rt[num_targets].length = len + off;
            target->rt[num_targets].target_type = (state == STATE_ABSOLUTE) ? TARGET_ABSOLUTE : TARGET_RELATIVE;
            target->rt[num_targets].numeric_target = atol((char *) target->rt[num_targets].string + off);
            if (target->rt[num_targets].negative) {
              target->rt[num_targets].numeric_target *= (-1L);
            }
            if (state == STATE_ABSOLUTE) {
              if (column_target) {
                if (target->rt[num_targets].numeric_target < true_line) {
                  target->rt[num_targets].negative = TRUE;
                  target->rt[num_targets].numeric_target = max(target->rt[num_targets].numeric_target, max(1, CURRENT_VIEW->zone_start - 1));
                } else {
                  target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target, min(max_line_length + 1, CURRENT_VIEW->zone_end + 1));
                }
              } else {
                if (target->rt[num_targets].numeric_target < true_line) {
                  target->rt[num_targets].negative = TRUE;
                } else {
                  target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target, (CURRENT_FILE->number_lines + 1L));
                }
              }
            } else {
              if (column_target) {
                if (target->rt[num_targets].negative) {
                  target->rt[num_targets].numeric_target = max(target->rt[num_targets].numeric_target, ((long) CURRENT_VIEW->zone_start - true_line - 1L));
                } else {
                  target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target, ((long) CURRENT_VIEW->zone_end - true_line + 1L));
                }
              } else {
                if (target->rt[num_targets].negative) {
                  target->rt[num_targets].numeric_target = max((target->rt[num_targets].numeric_target), (true_line == 0L) ? (0L) : (true_line * (-1L)));
                } else {
                  target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target, (CURRENT_FILE->number_lines - true_line + 1L));
                }
              }
            }
            potential_spare_start = i;
            num_targets++;
            state = STATE_BOOLEAN;
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            break;
          default:
            state = STATE_ERROR;
            inc = 0;
            break;
        }
        break;
      case STATE_NEGATIVE:
      case STATE_POSITIVE:
        switch (*(ptr + i)) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            state = STATE_RELATIVE;
            delim = (state == STATE_NEGATIVE) ? '-' : '+';
            str_start = i;
            inc = 0;
            break;
          /* following are delimiters; make sure you change the other lists */
          case '/':
          case '\\':
          case '@':
          case '`':
          case '#':
          case '$':
          case '%':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '"':
          case '\'':
          case '<':
          case '>':
          case ',':
          case 255:
          case '\0':
            state = STATE_START;
            inc = 0;
            break;
          case '*':
            state = STATE_START;
            inc = 0;
            break;
          case 'b':
          case 'B':
          case 'n':
          case 'N':
          case 't':
          case 'T':
          case 'c':
          case 'C':
          case 'a':
          case 'A':
          case 's':
          case 'S':
            /*
             * For -BLANK, -NEW, etc
             */
            state = STATE_START;
            inc = 0;
            break;
          default:
            state = STATE_ERROR;
            inc = 0;
            break;
        }
        break;
      case STATE_POINT:
        switch (*(ptr + i)) {
          case ' ':
          case '\t':
            state = STATE_BOOLEAN;
          /* fall through */
          case '&':
          case '|':
          case '\0':
            target->rt[num_targets].target_type = TARGET_POINT;
            str_end = i;
            len = str_end - str_start;
            target->rt[num_targets].string = (uchar *) malloc(len + 1);
            if (target->rt[num_targets].string == NULL) {
              if (allow_error_display) {
                display_error(30, (uchar *) "", FALSE);
              }
              return (RC_OUT_OF_MEMORY);
            }
            memcpy(target->rt[num_targets].string, ptr + str_start, len);
            target->rt[num_targets].string[len] = '\0';
            target->rt[num_targets].length = len;
            if (find_named_line(target->rt[num_targets].string, &lineno, TRUE) == NULL) {
              if (allow_error_display) {
                display_error(17, (uchar *) target->rt[num_targets].string, FALSE);
              }
              return (RC_TARGET_NOT_FOUND);
            }
            target->rt[num_targets].numeric_target = lineno;
            if (target->rt[num_targets].numeric_target < true_line) {
              target->rt[num_targets].negative = TRUE;
            } else {
              target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target, (CURRENT_FILE->number_lines + 1L));
            }
            num_targets++;
            potential_spare_start = i;
            if (*(ptr + i) == ' ' || *(ptr + i) == '\t') {
              break;
            }
            boolean = *(ptr + i);
            state = STATE_NEXT;
            break;
          default:
            break;
        }
        break;
      case STATE_ERROR:
        for (j = 0; j < num_targets; j++) {
          target->rt[j].target_type = TARGET_ERR;
        }
        state = STATE_QUIT;
        break;
    }
    if (state == STATE_QUIT) {
      break;
    }
    i += inc;
    if (i > target_length) {    /* this allows for testing '\0' as delimiter */
      break;
    }
  }
  target->num_targets = num_targets;
  if (num_targets == 0 || target->rt[0].target_type == TARGET_ERR) {
    if (display_parse_error && allow_error_display) {
      display_error(1, ptr, FALSE);
    }
    return (RC_INVALID_OPERAND);
  }
  /*
   * Time to validate the targets we have parsed...
   *
   * Valid combinations are:
   *
   * TARGET_ALL       (1 target only)
   *                  ALL  only
   * TARGET_BLOCK     (1 target only)
   *                 BLOCK  only
   *                  this section sets target_type to TARGET_BLOCK_ANY
   *                  or TARGET_BLOCK_CURRENT
   * TARGET_BLANK     (BLANK can be upper or lower case)
   *                  BLANK
   *                  -BLANK
   *                  ~BLANK
   *                  ~-BLANK
   * TARGET_NEW       (NEW can be upper or lower case)
   *                  NEW
   *                  -NEW
   *                  ~NEW
   *                  ~-NEW
   * TARGET_CHANGED   (CHANGED can be upper or lower case)
   *                  CHANGED
   *                  -CHANGED
   *                  ~CHANGED
   *                  ~-CHANGED
   * TARGET_TAGGED    (TAGGED can be upper or lower case)
   *                  TAGGED
   *                  -TAGGED
   *                  ~TAGGED
   *                  ~-TAGGED
   * TARGET_STRING    (various valid delimiters)
   *                  /string/
   *                  -/string/
   *                  ~/string/
   *                  ~-/string/
   * TARGET_POINT
   *                  .xxxxxx
   *                  ~.xxxxxx
   * TARGET_ABSOLUTE
   *                  :99
   *                  ;99
   * TARGET_RELATIVE
   *                  99
   *                  +99
   *                  -99
   *                  *
   *                  +*
   *                  -*
   * TARGET_REGEXP    (various valid delimiters)
   *                  /regexp/
   *
   * Any of the above target types may be or'd together.
   *
   * For each of the targets, check its validity...
   */
  negative = target->rt[0].negative;
  for (i = 0; i < num_targets - ((target->spare == (-1)) ? 0 : 1); i++) {
    switch (target->rt[i].target_type) {
      case TARGET_BLOCK:
        if (num_targets - ((target->spare == (-1)) ? 0 : 1) != 1) {
          rc = RC_INVALID_OPERAND;
          break;
        }
        if (target_types & TARGET_BLOCK_ANY) {
          target->rt[i].target_type = TARGET_BLOCK_ANY;
        } else {
          if (target_types & TARGET_BLOCK_CURRENT) {
            target->rt[i].target_type = TARGET_BLOCK_CURRENT;
          } else {
            rc = RC_INVALID_OPERAND;
          }
        }
        break;
      case TARGET_ALL:
        if (num_targets - ((target->spare == (-1)) ? 0 : 1) != 1) {
          rc = RC_INVALID_OPERAND;
          break;
        }
        if (target_types & target->rt[i].target_type) {
          break;
        }
        rc = RC_INVALID_OPERAND;
        break;
      case TARGET_REGEXP:
        if (num_targets - ((target->spare == (-1)) ? 0 : 1) != 1) {
          rc = RC_INVALID_OPERAND;
          break;
        }
        if (!(target_types & target->rt[i].target_type)) {
          rc = RC_INVALID_OPERAND;
          break;
        }
        /*
         * Compile the RE
         */
        memset(&target->rt[i].pattern_buffer, 0, sizeof(regex_t));
        if (0 != (rc = regcomp(&target->rt[i].pattern_buffer, (char *) target->rt[i].string, 0))) {
          /*
           * If ptr returns something, it is an error string
           * Display it if we are allowed to...
           */
          if (display_parse_error && allow_error_display) {
            rc = regerror(rc, &target->rt[i].pattern_buffer, (char *) trec, trec_len);
            sprintf((char *) trec + rc, "in %s", target->rt[i].string);
            display_error(216, (uchar *) trec, FALSE);
            return RC_INVALID_OPERAND;
          }
        }
        target->rt[i].have_compiled_re = TRUE;
        break;
      default:
        if (target->rt[i].negative != negative) {
          rc = RC_INVALID_OPERAND;
          break;
        }
        if (target_types & target->rt[i].target_type) {
          break;
        }
        rc = RC_INVALID_OPERAND;
        break;
    }
    if (rc == RC_INVALID_OPERAND) {
      break;
    }
  }
  /*
   * Display an error if anything found amiss and we are directed to display an error...
   */
  if (rc != RC_OK && display_parse_error && allow_error_display) {
    display_error(1, ptr, FALSE);
  }
  return (rc);
}

void initialise_target(TARGET *target) {
  memset(target, 0, sizeof(TARGET));
  target->spare = (-1);
  return;
}

void free_target(TARGET *target) {
  short i = 0;

  if (target->string == NULL && target->num_targets == 0 && target->rt == NULL) {
    return;
  }
  for (i = 0; i < target->num_targets; i++) {
    if (target->rt[i].string != NULL) {
      free(target->rt[i].string);
    }
    if (target->rt[i].have_compiled_re) {
      regfree(&target->rt[i].pattern_buffer);
    }
  }
  if (target->string != NULL) {
    free(target->string);
  }
  if (target->rt != NULL) {
    free(target->rt);
  }
  target->string = NULL;
  target->num_targets = 0;
  target->rt = NULL;
  return;
}

short find_target(TARGET *target, long true_line, bool display_parse_error, bool allow_error_display) {
  short rc = RC_OK;
  LINE *curr = NULL;
  LINE tmpcurr;
  long num_lines = 0L;
  long line_number = 0L;
  long first_found_column = 0;
  short status = RC_OK;
  int i;

  /*
   * Check single targets first (ALL and BLOCK)
   *
   * Check if first, and only target, is BLOCK...
   */
  switch (target->rt[0].target_type) {
    case TARGET_ALL:
      target->true_line = 1L;
      target->last_line = CURRENT_FILE->number_lines;
      target->num_lines = CURRENT_FILE->number_lines;
      return (RC_OK);
      break;
    case TARGET_BLOCK_ANY:
      if (MARK_VIEW == NULL) {
        if (allow_error_display) {
          display_error(44, (uchar *) "", FALSE);
        }
        rc = RC_TARGET_NOT_FOUND;
      } else {
        target->num_lines = MARK_VIEW->mark_end_line - MARK_VIEW->mark_start_line + 1L;
        target->true_line = MARK_VIEW->mark_start_line;
        target->last_line = MARK_VIEW->mark_end_line;
      }
      return rc;
      break;
    case TARGET_BLOCK_CURRENT:
      if (MARK_VIEW == NULL) {
        if (allow_error_display) {
          display_error(44, (uchar *) "", FALSE);
        }
        rc = RC_TARGET_NOT_FOUND;
      } else {
        if (MARK_VIEW != CURRENT_VIEW) {
          if (allow_error_display) {
            display_error(45, (uchar *) "", FALSE);
          }
          rc = RC_TARGET_NOT_FOUND;
        } else {
          /*
           * CUA blocks can have end_line < start_line
           */
          if (MARK_VIEW->mark_type == M_CUA && MARK_VIEW->mark_end_line < MARK_VIEW->mark_start_line) {
            target->num_lines = MARK_VIEW->mark_start_line - MARK_VIEW->mark_end_line + 1L;
            target->true_line = MARK_VIEW->mark_start_line;
            target->last_line = MARK_VIEW->mark_end_line;
          } else {
            target->num_lines = MARK_VIEW->mark_end_line - MARK_VIEW->mark_start_line + 1L;
            target->true_line = MARK_VIEW->mark_start_line;
            target->last_line = MARK_VIEW->mark_end_line;
          }
        }
      }
      return rc;
      break;
    default:
      break;
  }
  /*
   * All other targets are potentially repeating targets...
   */
  rc = RC_TARGET_NOT_FOUND;
  line_number = true_line;
  /*
   * Find the focus line
   */
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
  /*
   * If this is a search and we are on the focus line use the current working data, so make up a dummy LINE.
   */
  if (target->search_semantics && CURRENT_VIEW->current_window == WINDOW_FILEAREA) {
    memset(&tmpcurr, 0, sizeof(LINE));
    tmpcurr.line = rec;
    tmpcurr.length = rec_len;
    tmpcurr.next = curr->next;
    tmpcurr.prev = curr->prev;
    curr = &tmpcurr;
  }
  num_lines = 0L;
  for (;;) {
    /*
     * For all repeating targets,
     * see if the combined targets are found on the line we are currently processing
     */
    status = find_rtarget_target(curr, target, true_line, line_number, &num_lines);
    if (status != RC_TARGET_NOT_FOUND) {
      break;
    }
    /*
     * We can determine the direction of execution based on the first target,
     * as all targets must have the same direction to have reached here.
     */
    if (target->rt[0].negative) {
      /*
       * We didn't find the combined targets on this line, so get the previous line
       */
      curr = curr->prev;
      line_number--;
      if (curr) {
        /*
         * We have a real line,
         * so set the target focus_column to after the end of the line (if SEARCHing)
         * or to the start of the line (if LOCATEing)
         *
         * When setting "after the end of line", we have to take into consideration the length of the needle,
         * in case it ends in space (we need to be able to find a string with a trailing space).
         */
        if (target->search_semantics) {
          target->focus_column = curr->length + 1;
        } else {
          target->focus_column = 0;
        }
      }
    } else {
      /*
       * Get the next line if locating/searching forward
       */
      curr = curr->next;
      line_number++;
    }
    if (curr == NULL) {
      break;
    }
  }
  if (status == RC_OK) {
    num_lines = ((target->rt[0].negative) ? -num_lines : num_lines);
    target->num_lines = num_lines;
    target->true_line = true_line;
    if (target->rt[0].negative) {
      curr = curr->next;
      target->last_line = find_next_in_scope(CURRENT_VIEW, curr, ++line_number, DIRECTION_FORWARD);
    } else {
      curr = curr->prev;
      target->last_line = find_next_in_scope(CURRENT_VIEW, curr, --line_number, DIRECTION_BACKWARD);
    }
    /*
     * We found at least one of our potential targets.
     * When SEARCHing, we need to know where to move the cursor.
     * We find the lowest column of the targets that have been found
     * and set target->focus_column to that column.
     */
    if (target->search_semantics) {
      for (i = 0; i < target->num_targets; i++) {
        if (target->rt[i].found) {
          if (first_found_column == 0) {
            first_found_column = target->rt[i].start;
          } else if (target->rt[i].start < first_found_column) {
            first_found_column = target->rt[i].start;
          }
        }
      }
      target->focus_column = first_found_column;
    }
    rc = RC_OK;
  } else if (status == RC_TARGET_NOT_FOUND) {
    if (allow_error_display) {
      display_error(17, target->string, FALSE);
    }
    rc = RC_TARGET_NOT_FOUND;
  } else {
    rc = status;
  }
  return (rc);
}

short find_column_target(uchar *line, long len, TARGET *target, long true_column, bool display_parse_error, bool allow_error_display) {
  short rc = RC_OK;
  long column_number = 0L;
  long num_columns = 0L;
  bool status = FALSE;

  /*
   * All column targets are potentially repeating targets...
   */
  rc = RC_TARGET_NOT_FOUND;
  status = FALSE;
  column_number = true_column;
  num_columns = 0;
  for (;;) {
    status = find_rtarget_column_target(line, len, target, true_column, column_number, &num_columns);
    if (status) {
      break;
    }
    /*
     * We can determine the direction of execution based on the first target,
     * as all targets must have the same direction to have reached here.
     */
    if (target->rt[0].negative) {
      if (column_number-- == (long) CURRENT_VIEW->zone_start - 2L) {
        status = FALSE;
        break;
      }
    } else {
      if (column_number++ == (long) CURRENT_VIEW->zone_end + 2L) {
        status = FALSE;
        break;
      }
    }
  }
  if (status) {
    num_columns = ((target->rt[0].negative) ? -num_columns : num_columns);
    target->num_lines = num_columns;
    target->true_line = (long) true_column;
    target->last_line = (long) column_number;
    rc = RC_OK;
  } else {
    if (allow_error_display) {
      display_error(17, target->string, FALSE);
    }
    rc = RC_TARGET_NOT_FOUND;
  }
  return (rc);
}

static bool is_blank(LINE *curr) {
  long i = 0;
  bool rc = TRUE;

  if (CURRENT_VIEW->zone_start > curr->length) {
    return (TRUE);
  }
  for (i = CURRENT_VIEW->zone_start - 1; i < min(CURRENT_VIEW->zone_end, curr->length); i++) {
    if (*(curr->line + i) != ' ') {
      rc = FALSE;
      break;
    }
  }
  return (rc);
}

THELIST *find_line_name(LINE *curr, uchar *name) {
  THELIST *list_curr = NULL;

  if (curr == NULL || curr->first_name == NULL) {
    return ((THELIST *) NULL);
  }
  /*
   * Look for the passed in name...
   */
  list_curr = curr->first_name;
  while (list_curr != NULL) {
    if (strcmp((char *) list_curr->data, (char *) name) == 0) {
      return (list_curr);
    }
    list_curr = list_curr->next;
  }
  return ((THELIST *) NULL);
}

LINE *find_named_line(uchar *name, long *retline, bool respect_scope) {
  long lineno = 0;
  LINE *curr = NULL;

  /*
   * Find the line number in the current file of the named line specified
   */
  curr = CURRENT_FILE->first_line;
  while (curr != (LINE *) NULL) {
    /*
     * Check the line's name if we are not respecting scope or if we are respecting scope and the line is in scope.
     */
    if (!respect_scope || (respect_scope && (IN_SCOPE(CURRENT_VIEW, curr) || CURRENT_VIEW->scope_all))) {
      /*
       * If we don't have a first_name, ignore the line
       */
      if (curr->first_name != NULL) {
        if (find_line_name(curr, name) != NULL) {
          *retline = lineno;
          return (curr);
        }
      }
    }
    lineno++;
    curr = curr->next;
  }
  return ((LINE *) NULL);
}

/*
 * Finds a string (needle: in rt->string) in another string (haystack: in curr->line)
 * If SEARCHing backwards, we need to copy the string and reverse the needle and
 * the haystack.
 * e.g.
 * SEARCH -/abc/
 * where curr->line = "1234abc56" and current column position is 9 (start_col is 0 based)
 * needle becomes: "cba"
 * haystack becomes: "65cba4321"
 * and start_col becomes: 0 (len - start_col - 1)
 */
short find_string_target(LINE *curr, RTARGET *rt, long start_col, int search_semantics) {
  uchar *haystack = curr->line;
  uchar *needle;
  long needle_length = 0, haystack_length = 0;
  long real_start = 0, real_end = 0;
  bool use_trec = FALSE;
  short rc = RC_OK;
  long loc = (-1);
  long str_length = 0;

  /*
   * Allocate some temporary space
   */
  needle = (uchar *) alloca(rt->length + 1);
  if (needle == NULL) {
    display_error(30, (uchar *) "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  /*
   * Copy the supplied string target rather than point to it,
   * as we don't want to change the value of the target if it is a HEX string.
   */
  strcpy((char *) needle, (char *) rt->string);
  /*
   * If HEX is on, convert the target from a HEX format to uchar.
   */
  if (CURRENT_VIEW->hex == TRUE) {
    rc = convert_hex_strings(needle);
    switch (rc) {
      case -1:                 /* invalid hex value */
        display_error(32, needle, FALSE);
        return (RC_INVALID_OPERAND);
        break;
      case -2:                 /* memory exhausted */
        display_error(30, (uchar *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
        break;
      default:
        break;
    }
    needle_length = rc;
  } else {
    needle_length = strlen((char *) needle);
  }
  /*
   * Set the length of the string to be the actual length of the string target.
   */
  rt->length = needle_length;
  /*
   * Determine if we need to copy the contents of the line into trec.
   * The reasons we need to do this are:
   * - the length of the needle is 0
   * - the last character of needle is a space
   */
  if (needle_length == 0) {
    use_trec = TRUE;
  } else {
    if (*(needle + (needle_length - 1)) == ' ') {
      use_trec = TRUE;
    }
  }
  if (use_trec) {
    memset(trec, ' ', max_line_length);
    memcpy(trec, curr->line, curr->length);
    haystack = trec;
    haystack_length = min(max_line_length, max(CURRENT_VIEW->zone_start, curr->length) + needle_length);
  } else {
    haystack = curr->line;
    haystack_length = curr->length;
  }
  /*
   * Calculate the bounds to search in based on length of haystack and ZONE settings.
   * If the haystack is empty, no need to search.
   */
  if (haystack_length > 0) {
    if (search_semantics && rt->negative) {
      real_end = max(start_col, min(haystack_length, CURRENT_VIEW->zone_end - 1));
      real_start = min(start_col, CURRENT_VIEW->zone_end - 1);
    } else {
      real_end = min(haystack_length - 1, CURRENT_VIEW->zone_end - 1);
      real_start = max(start_col, CURRENT_VIEW->zone_start - 1);
    }
    /*
     * Find the needle in the haystack if real_end > real_start
     */
    if (real_end >= real_start) {
      if (search_semantics && rt->negative) {
        long i;

        for (i = 0; loc == (-1) && real_start >= CURRENT_VIEW->zone_start - 1; i++, real_start--) {
          loc = memfind(haystack + real_start, needle, (real_end - real_start + 1), needle_length, (bool) ((CURRENT_VIEW->case_locate == CASE_IGNORE) ? TRUE : FALSE), CURRENT_VIEW->arbchar_status, CURRENT_VIEW->arbchar_single, CURRENT_VIEW->arbchar_multiple, &str_length);
          if (loc != (-1) && loc + real_start - 1 == start_col) {
            loc = -1;
          }
        }
        if (loc != (-1)) {
          real_start++;
        }
      } else {
        loc = memfind(haystack + real_start, needle, (real_end - real_start + 1), needle_length, (bool) ((CURRENT_VIEW->case_locate == CASE_IGNORE) ? TRUE : FALSE), CURRENT_VIEW->arbchar_status, CURRENT_VIEW->arbchar_single, CURRENT_VIEW->arbchar_multiple, &str_length);
      }
    }
  }
  if (loc == (-1)) {
    rc = RC_TARGET_NOT_FOUND;
  } else {
    rt->start = loc + real_start;
    rt->found_length = str_length;
    rc = RC_OK;
  }
  return (rc);
}

short find_regexp(LINE *curr, RTARGET *rt) {
  uchar *haystack = NULL;
  long len, i, haystack_length = 0, real_start = 0, real_end = 0;
  regmatch_t match;

  /*
   * Search for the compiled RE
   */
  haystack = curr->line;
  haystack_length = curr->length;
  /*
   * Calculate the bounds to search in based on length of haystack and ZONE settings.
   * If the haystack is empty, no need to search.
   */
  if (haystack_length == 0) {
    haystack = (uchar *) "";
    real_end = real_start = 0;
    len = 0;
  } else {
    real_end = min(haystack_length - 1, CURRENT_VIEW->zone_end - 1);
    real_start = max(0, CURRENT_VIEW->zone_start - 1);
    len = real_end - real_start + 1;
  }
  for (i = 0; i < len;) {
    if (0 == regexec(&rt->pattern_buffer, (char *) haystack + i, 1, &match, 0)) {
      if (match.rm_eo > (len - i)) {
        break;                  // match but past limit
      }
      rt->length = match.rm_eo - match.rm_so;
      rt->start = real_start + i;       /* ?? */
      rt->found_length = rt->length;
      return RC_OK;
      break;
    } else {
      i++;
    }
  }
  return RC_TARGET_NOT_FOUND;
}

short find_rtarget_target(LINE *curr, TARGET *target, long true_line, long line_number, long *num_lines) {
  short i = 0;
  bool target_found = FALSE, status = FALSE;
  long multiplier = 0;
  short rc = RC_OK;
  long start_col;

  /*
   * If the line is not in scope and scope is respected, return FALSE.
   */
  if (!(IN_SCOPE(CURRENT_VIEW, curr)) && !target->ignore_scope) {
    if (!CURRENT_VIEW->scope_all && !TOF(line_number) && !BOF(line_number)) {
      return (RC_TARGET_NOT_FOUND);
    }
  }
  if (line_number != true_line) {
    *num_lines = *num_lines + 1L;
  }
  for (i = 0; i < target->num_targets - ((target->spare == (-1)) ? 0 : 1); i++) {
    target_found = FALSE;
    multiplier = (target->rt[i].negative) ? -1L : 1L;
    switch (target->rt[i].target_type) {
      case TARGET_BLANK:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        target_found = is_blank(curr);
        break;
      case TARGET_NEW:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        target_found = curr->flags.new_flag;
        break;
      case TARGET_CHANGED:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        target_found = curr->flags.changed_flag;
        break;
      case TARGET_ALTERED:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        target_found = (curr->flags.changed_flag | curr->flags.new_flag);
        break;
      case TARGET_TAGGED:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        target_found = curr->flags.tag_flag;
        break;
      case TARGET_POINT:
        if (curr->first_name == NULL) {
          break;
        }
        if (find_line_name(curr, target->rt[i].string) != NULL) {
          target_found = TRUE;
        }
        break;
      case TARGET_STRING:
        /*
         * If we are doing a SEARCH, start at focus column otherwise start at 0 for locate
         * As focus_column is 1 based, then using focus_column unaltered is the same as adding 1,
         * so in a backwards search we have to subtract 1 for the previous column,
         * and 1 to adjust from 1 based to 0 based (-2 in total).
         */
        if (target->search_semantics) {
          if (target->rt[0].negative) {
            if (target->focus_column == -1L) {
              start_col = curr->length + strlen((char *) target->rt[i].string);
            } else {
              start_col = target->focus_column - 2;
            }
          } else {
            if (target->focus_column == -1L) {
              start_col = 0;
            } else {
              start_col = target->focus_column;
            }
          }
        } else {
          start_col = 0;
        }
        /*
         * If we are processing the focus line we return immediately;
         * we can't match on the focus line, unless we are running a SEARCH
         */
        if (true_line == line_number && target->search_semantics == FALSE) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        /*
         * When we are SEARCHing we can't stop because the PREVIOUS line is NULL;
         * there still could be strings on the line after the focus column.
         * BUT HOW DO WE STOP ??? SEEMS OK!
         */
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        if (start_col == (-1)) {
          rc = RC_TARGET_NOT_FOUND;
        } else {
          rc = find_string_target(curr, &target->rt[i], start_col, target->search_semantics);
        }
        switch (rc) {
          case RC_OK:
            target->rt[i].found = target_found = TRUE;
            break;
          case RC_TARGET_NOT_FOUND:
            /*
             * Target not found. If this is a search, we need to reset the focus_column to start searching at col 0
             */
            target->focus_column = -1L;
            break;
          default:
            return (rc);
            break;
        }
        break;
      case TARGET_RELATIVE:
        /*
         * If the command is TAG or ALL then we mark the nth line so a match here is based on
         */
        if (target->all_tag_command) {
          if (target->rt[i].numeric_target == 0) {
            return (RC_TARGET_NOT_FOUND);
            break;
          }
          if (*num_lines % target->rt[i].numeric_target == 0) {
            target_found = TRUE;
          }
          break;
        }
        if ((*num_lines * multiplier) == target->rt[i].numeric_target) {
          target_found = TRUE;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            target_found = TRUE;
            break;
          }
        } else {
          if (curr->next == NULL) {
            target_found = TRUE;
            break;
          }
        }
        break;
      case TARGET_ABSOLUTE:
        if (line_number == target->rt[i].numeric_target) {
          target_found = TRUE;
        }
        break;
      case TARGET_REGEXP:
        if (true_line == line_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (target->rt[0].negative) {
          if (curr->prev == NULL) {
            break;
          }
        } else {
          if (curr->next == NULL) {
            break;
          }
        }
        rc = find_regexp(curr, &target->rt[i]);
        switch (rc) {
          case RC_OK:
            target->rt[i].found = target_found = TRUE;
            break;
          case RC_TARGET_NOT_FOUND:
            break;
          default:
            return (rc);
            break;
        }
        break;
      default:
        break;
    }
    if (target->rt[i].not_target) {
      target->rt[i].found = target_found = (target_found) ? FALSE : TRUE;
    }
    switch (target->rt[i].boolean) {
      case ' ':
        status = target_found;
        break;
      case '&':
        status &= target_found;
        break;
      case '|':
        status |= target_found;
        break;
    }
  }
  return ((status) ? RC_OK : RC_TARGET_NOT_FOUND);
}

bool find_rtarget_column_target(uchar *line, long len, TARGET *target, long true_column, long column_number, long *num_columns) {
  short i = 0;
  bool target_found = FALSE, status = FALSE;
  long multiplier = 0;
  LINE curr;

  if (column_number != true_column) {
    *num_columns = *num_columns + 1L;
  }
  for (i = 0; i < target->num_targets - ((target->spare == (-1)) ? 0 : 1); i++) {
    target_found = FALSE;
    multiplier = (target->rt[i].negative) ? -1L : 1L;
    switch (target->rt[i].target_type) {
      case TARGET_BLANK:
        if (true_column == column_number) {
          target_found = ((target->rt[i].not_target) ? TRUE : FALSE);
          break;
        }
        if (column_number < CURRENT_VIEW->zone_start || column_number > CURRENT_VIEW->zone_end) {
          target_found = FALSE;
          break;
        }
        if (column_number > len) {
          target_found = TRUE;
          break;
        }
        if (*(line + column_number - 1) == ' ') { /* should be blank word */
          target_found = TRUE;
        } else {
          target_found = FALSE;
        }
        break;
      case TARGET_STRING:
        if (column_number < CURRENT_VIEW->zone_start || column_number > CURRENT_VIEW->zone_end || column_number > len) {
          target_found = FALSE;
          break;
        }
        /*
         * We need to determine if the string target starts in the column; column_number.
         */
        curr.line = line;
        curr.length = len;

        /*
         * If locating backwards, start the search 2 positions to the left of the current true_column.
         * This is because true_column is 1 based, and our searching is 0 based,
         * so the first decrement is to adjust 1-based to 0-based,
         * and the second decrement is to put the cursor 1 position to the left of our focus column,
         * otherwise it will possibly match.
         */
        if (target->rt[i].negative) {
          true_column -= 2;
        }
        if (find_string_target(&curr, &target->rt[i], true_column, THE_SEARCH_SEMANTICS) == RC_OK && target->rt[i].start + 1 == column_number) {
          target_found = TRUE;
        }
        break;
      case TARGET_ABSOLUTE:
        if (column_number == target->rt[i].numeric_target) {
          target_found = TRUE;
        }
        break;
      case TARGET_RELATIVE:
        if ((*num_columns * multiplier) == target->rt[i].numeric_target) {
          target_found = TRUE;
        }
        break;
      default:
        break;
    }
    if (target->rt[i].not_target) {
      target_found = (target_found) ? FALSE : TRUE;
    }
    switch (target->rt[i].boolean) {
      case ' ':
        status = target_found;
        break;
      case '&':
        status &= target_found;
        break;
      case '|':
        status |= target_found;
        break;
    }
  }
  return (status);
}

long find_next_in_scope(VIEW_DETAILS *view, LINE *in_curr, long line_number, short direction) {
  LINE *curr = in_curr;

  if (in_curr == NULL) {
    curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, line_number, CURRENT_FILE->number_lines);
  }
  for (;; line_number += (long) direction) {
    if (IN_SCOPE(view, curr)) {
      break;
    }
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    if (curr == NULL) {
      break;
    }
  }
  return (line_number);
}

long find_last_not_in_scope(VIEW_DETAILS *view, LINE *in_curr, long line_number, short direction) {
  LINE *curr = in_curr;
  long offset = 0L;

  if (in_curr == NULL) {
    curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, line_number, CURRENT_FILE->number_lines);
  }
  for (;; line_number += (long) direction) {
    if (IN_SCOPE(view, curr)) {
      break;
    }
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
      offset = (-1L);
    } else {
      curr = curr->prev;
      offset = 1L;
    }
    if (curr == NULL) {
      break;
    }
  }
  return (line_number + offset);
}

short validate_target(uchar *string, TARGET *target, long target_type, long true_line, bool display_parse_error, bool allow_error_display) {
  short rc = RC_OK;

  rc = parse_target(string, true_line, target, target_type, display_parse_error, allow_error_display, FALSE);
  if (rc != RC_OK) {
    return (rc);
  }
  rc = find_target(target, true_line, display_parse_error, allow_error_display);
  if (rc != RC_OK) {
    return (RC_TARGET_NOT_FOUND);
  }
  return (RC_OK);
}

void calculate_scroll_values(uchar curr_screen, VIEW_DETAILS *curr_view, short *number_focus_rows, long *new_focus_line, long *new_current_line, bool *limit_of_screen, bool *limit_of_file, bool *leave_cursor, short direction) {
  short i = 0;
  unsigned short y = 0;

  *limit_of_screen = *limit_of_file = FALSE;
  *number_focus_rows = 0;
  *new_focus_line = (-1L);
  y = screen[curr_screen].rows[WINDOW_FILEAREA];
  switch (direction) {
    case DIRECTION_FORWARD:
      /*
       * Determine the new focus line and the number of rows to adjust the cursor position.
       */
      for (i = 0; i < screen[curr_screen].rows[WINDOW_FILEAREA]; i++) {
        if (screen[curr_screen].sl[i].line_number == curr_view->focus_line) {
          y = i;
          continue;
        }
        if (screen[curr_screen].sl[i].line_number != (-1L) && y != screen[curr_screen].rows[WINDOW_FILEAREA]) {
          *number_focus_rows = i - y;
          *new_focus_line = screen[curr_screen].sl[i].line_number;
          break;
        }
      }
      /*
       * If we have NOT set a new focus line (because we are on the bottom of the screen)
       * the new focus line is the next line in scope (if SHADOW is OFF).
       * If SHADOW is ON, the new focus line is determined by the status of the current focus line.
       */
      if (*new_focus_line == (-1L)) {
        if (curr_view->shadow) {
          *new_focus_line = screen[curr_screen].sl[y].line_number + ((screen[curr_screen].sl[y].number_lines_excluded == 0) ? 1L : (long) screen[curr_screen].sl[y].number_lines_excluded);
        } else {
          if (screen[curr_screen].sl[y].current->next != NULL) {
            *new_focus_line = find_next_in_scope(curr_view, screen[curr_screen].sl[y].current->next, screen[curr_screen].sl[y].line_number + 1L, direction);
          }
        }
      }
      /*
       * Determine the new current line and the number of rows to adjust the cursor position.
       */
      *leave_cursor = TRUE;
      *new_current_line = (-1L);
      for (i = curr_view->current_row + 1; i < screen[curr_screen].rows[WINDOW_FILEAREA]; i++) {
        if (screen[curr_screen].sl[i].line_type == LINE_LINE || screen[curr_screen].sl[i].line_type == LINE_TOF || screen[curr_screen].sl[i].line_type == LINE_EOF) {
          *new_current_line = screen[curr_screen].sl[i].line_number;
          break;
        }
        if (screen[curr_screen].sl[i].line_type == LINE_SHADOW) {
          *leave_cursor = FALSE;
        }
      }
      /*
       * If we have NOT set a new current line (only way this can happen is if all lines after the current line are RESERVED, SCALE or TABLINE)
       * and the cursor is on the current line) the new current line is the next line in scope.
       */
      if (*new_current_line == (-1L)) {
        if (screen[curr_screen].sl[y].current->next != NULL) {
          *new_current_line = find_next_in_scope(curr_view, screen[curr_screen].sl[y].current->next, screen[curr_screen].sl[y].line_number + 1L, direction);
        }
      }
      /*
       * Set flags for bottom_of_screen and bottom_of_file as appropriate.
       */
      if (*number_focus_rows == 0) {
        *limit_of_screen = TRUE;
      }
      if (screen[curr_screen].sl[y].line_type == LINE_EOF) {
        *limit_of_file = TRUE;
      }
      break;
    case DIRECTION_BACKWARD:
      /*
       * Determine the new focus line and the number of rows to adjust the cursor position.
       */
      for (i = screen[curr_screen].rows[WINDOW_FILEAREA] - 1; i > -1; i--) {
        if (screen[curr_screen].sl[i].line_number == curr_view->focus_line) {
          y = i;
          continue;
        }
        if (screen[curr_screen].sl[i].line_number != (-1L) && y != screen[curr_screen].rows[WINDOW_FILEAREA]) {
          *number_focus_rows = y - i;
          *new_focus_line = screen[curr_screen].sl[i].line_number;
          break;
        }
      }
      /*
       * If we have NOT set a new focus line (because we are on the top of the screen)
       * the new focus line is the prev line in scope (if SHADOW is OFF).
       * If SHADOW is ON, the new focus line is determined by the status of the current focus line.
       */
      if (*new_focus_line == (-1L)) {
        if (curr_view->shadow) {
          if (screen[curr_screen].sl[y].line_type == LINE_SHADOW) {
            *new_focus_line = screen[curr_screen].sl[y].line_number - 1L;
          } else {
            if (screen[curr_screen].sl[y].current->prev != NULL) {
              *new_focus_line = find_next_in_scope(curr_view, screen[curr_screen].sl[y].current->prev, screen[curr_screen].sl[y].line_number - 1L, direction);
              if (*new_focus_line != screen[curr_screen].sl[y].line_number - 1L) {
                *new_focus_line = *new_focus_line + 1;
              }
            }
          }
        } else {
          if (screen[curr_screen].sl[y].current->prev != NULL) {
            *new_focus_line = find_next_in_scope(curr_view, screen[curr_screen].sl[y].current->prev, screen[curr_screen].sl[y].line_number - 1L, direction);
          }
        }
      }
      /*
       * Determine the new current line and the number of rows to adjust the cursor position.
       */
      *leave_cursor = TRUE;
      *new_current_line = (-1L);
      for (i = curr_view->current_row - 1; i > -1; i--) {
        if (screen[curr_screen].sl[i].line_type == LINE_LINE || screen[curr_screen].sl[i].line_type == LINE_TOF || screen[curr_screen].sl[i].line_type == LINE_EOF) {
          *new_current_line = screen[curr_screen].sl[i].line_number;
          break;
        }
        if (screen[curr_screen].sl[i].line_type == LINE_SHADOW) {
          *leave_cursor = FALSE;
        }
      }
      /*
       * If we have NOT set a new current line (only way this can happen is if all lines before the current line are RESERVED, SCALE or TABLINE)
       * and the cursor is on the current line) the new current line is the previous line in scope.
       */
      if (*new_current_line == (-1L)) {
        if (screen[curr_screen].sl[y].current->prev != NULL) {
          *new_current_line = find_next_in_scope(curr_view, screen[curr_screen].sl[y].current->prev, screen[curr_screen].sl[y].line_number - 1L, direction);
        } else {
          *new_current_line = *new_focus_line;
        }
      }
      /*
       * Set flags for top_of_screen and top_of_file as appropriate.
       */
      if (*number_focus_rows == 0) {
        *limit_of_screen = TRUE;
      }
      if (screen[curr_screen].sl[y].line_type == LINE_TOF) {
        *limit_of_file = TRUE;
      }
      break;
  }
  return;
}

short find_last_focus_line(uchar curr_screen, unsigned short *newrow) {
  short i = 0;
  short row = (-1);
  short rc = RC_OK;

  for (i = screen[curr_screen].rows[WINDOW_FILEAREA] - 1; i > -1; i--) {
    if (screen[curr_screen].sl[i].line_number != (-1L)) {
      *newrow = row = i;
      break;
    }
  }
  if (row == (-1)) {
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}

short find_first_focus_line(uchar curr_screen, unsigned short *newrow) {
  short i = 0;
  short row = (-1);
  short rc = RC_OK;

  for (i = 0; i < screen[curr_screen].rows[WINDOW_FILEAREA]; i++) {
    if (screen[curr_screen].sl[i].line_number != (-1L)) {
      *newrow = row = i;
      break;
    }
  }
  if (row == (-1)) {
    rc = RC_INVALID_OPERAND;
  }
  return (rc);
}

uchar find_unique_char(uchar *str) {
  short i = 0;

  for (i = 254; i > 0; i--) {
    if (strzeq(str, (uchar) i) == (-1)) {
      return ((uchar) i);
    }
  }
  return (0);
}

