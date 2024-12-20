// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

#include <fnmatch.h>

#define STATE_START             0
#define STATE_SINGLE_QUOTE      1
#define STATE_DOUBLE_QUOTE      2
#define STATE_BACKSLASH_SINGLE  3
#define STATE_BACKSLASH_DOUBLE  4
#define STATE_IGNORE            5
#define STATE_CASE              6
#define STATE_OPTION            7
#define STATE_NUMBER            8
#define STATE_IDENTIFIER        9
#define STATE_STRING           10
#define STATE_COMMENT          11
#define STATE_HEADER           12
#define STATE_LABEL            13
#define STATE_MATCH            14
#define STATE_COLUMN           15
#define STATE_KEYWORD          16
#define STATE_POSTCOMPARE      17
#define STATE_FILE_HEADER      18
#define STATE_DELIMITER        19
#define STATE_FUNCTION         20
#define STATE_SPACE            21
#define STATE_SLASH            22
#define STATE_MARKUP           23
#define STATE_START_TAG        24
#define STATE_END_TAG          25
#define STATE_DIRECTORY        26
#define STATE_ENV              27

#define REXX_NUMBER_RE   "-?\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*\\([eE][-+]?[0-9]+\\)?"
#define COBOL_NUMBER_RE  "-?\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*"
#define C_NUMBER_RE      "-?\\(\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*\\|[0][0-7]+\\|0[xX][0-9a-fA-F]+\\)\\([dDeE][-+]?[0-9]+\\)?[uU]?[lL]?"

static int number_blanks = 0;
static uchar *work;
static uchar tmp[1000];

static uchar THE_CHAR_SPACE = ' ';
static uchar THE_CHAR_STAR = '*';
static uchar THE_CHAR_COLON = ':';
static uchar THE_CHAR_BANG = '!';
static uchar THE_CHAR_SLASH = '/';
static uchar THE_CHAR_HASH = '#';

typedef struct {
  int start_search_row;
  int start_search_column;
  int start_row;
  int start_column;
  int end_row;
  int end_column;
  int type;
} comment_loc;

static int get_alternate(uchar *alt, uchar *alternate) {
  int rc = 0;

  if (alt[0] >= '1' && alt[0] <= '9') {
    *alternate = alt[0] - '1' + 26;
  } else if (alt[0] >= 'a' && alt[0] <= 'z') {
    *alternate = alt[0] - 'a';
  } else if (alt[0] >= 'A' && alt[0] <= 'Z') {
    *alternate = alt[0] - 'A';
  } else {
    rc = 1;
  }
  return rc;
}

/*
 * For the supplied displayed line find a paired comment delimiter
 * Only look for delimiters if there is no syntax highlighting already
 * or the syntax highlighting is for an incomplete string
 */
static long find_comment(uchar scrno, SHOW_LINE *scurr, FILE_DETAILS *fd, long start, long length, uchar *ptr, uchar *start_delim, uchar *end_delim, int *type) {
  int i, j;
  uchar *ptr_start = start_delim, *ptr_end = end_delim;
  uchar ch1, ch2, ch3;

  for (i = start; i < length; i++) {
    if (scurr->highlight_type[i] == THE_SYNTAX_NONE || scurr->highlight_type[i] == THE_SYNTAX_INCOMPLETESTRING || scurr->highlight_type[i] == THE_SYNTAX_COMMENT || scurr->highlight_type[i] == THE_SYNTAX_POSTCOMPARE || scurr->highlight_type[i] == THE_SYNTAX_MARKUP) {
      /* if the current syntax setting for the current character is NOT set, then we can test for a comment... */
      if (start_delim) {
        /*
         * We are checking for a starting delimiter...
         */
        ptr_start = start_delim;
        if (fd->parser->case_sensitive) {
          ch1 = ptr[i];
          ch2 = *ptr_start;
        } else {
          ch1 = tolower(ptr[i]);
          ch2 = tolower(*ptr_start);
        }
        if (ch1 == ch2) {
          /*
           * We have a match for the first character of the start delimiter;
           * check if the rest of the delimiter matches...
           */
          for (j = i; j <= length; j++) {
            ch2 = (fd->parser->case_sensitive) ? *ptr_start : tolower(*ptr_start);
            ch3 = (fd->parser->case_sensitive) ? ptr[j] : tolower(ptr[j]);
            if (ch2 == '\0') {
              *type = STATE_START_TAG;
              return i;
            }
            if (ch3 != ch2) {
              break;
            }
            ptr_start++;
          }
        }
      }
      if (end_delim) {
        /*
         * We are checking for an ending delimiter...
         */
        ptr_end = end_delim;
        if (fd->parser->case_sensitive) {
          ch1 = ptr[i];
          ch2 = *ptr_end;
        } else {
          ch1 = tolower(ptr[i]);
          ch2 = tolower(*ptr_end);
        }
        if (ch1 == ch2) {
          /*
           * We have a match for the first character of the end delimiter;
           * check if the rest of the delimiter matches...
           */
          for (j = i; j <= length; j++) {
            ch2 = (fd->parser->case_sensitive) ? *ptr_end : tolower(*ptr_end);
            ch3 = (fd->parser->case_sensitive) ? ptr[j] : tolower(ptr[j]);
            if (ch2 == '\0') {
              *type = STATE_END_TAG;
              return i;
            }
            if (ch3 != ch2) {
              break;
            }
            ptr_end++;
          }
        }
      }
    }
  }
  return -1;
}

static long find_line_comments(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  PARSE_COMMENTS *curr = fd->parser->first_comments;
  long i, pos = 0, len = scurr->length;
  uchar *ptr;
  int state;
  uchar ch1, ch2;

  for (; curr != NULL; curr = curr->next) {
    /* this loop is executed for each defined single line comment */
    if (!curr->line_comment) {
      continue;
    }
    ptr = curr->start_delim;
    state = STATE_START;
    switch (curr->column) {
      case 0:                  /* 'any' column */
        for (i = 0; i < len; i++) {
          if (scurr->highlight_type[i] == THE_SYNTAX_NONE) {
            /* if the current syntax setting for the current character is NOT set, then we can test for a comment... */
            if (fd->parser->case_sensitive) {
              ch1 = work[i];
              ch2 = *ptr;
            } else {
              ch1 = tolower(work[i]);
              ch2 = tolower(*ptr);
            }
            switch (state) {
              case STATE_START:
                if (ch1 == ch2) {
                  state = STATE_COMMENT;
                  pos = i;
                  ptr++;
                  break;
                }
                break;
              case STATE_COMMENT:
                if (ch2 == '\0') {
                  state = STATE_MATCH;
                  break;
                }
                if (ch1 != ch2) {
                  state = STATE_START;
                  ptr = curr->start_delim;
                  break;
                }
                ptr++;
                break;
            }
            if (state == STATE_MATCH) {
              break;
            }
          }
        }
        if (state == STATE_MATCH || (state == STATE_COMMENT && *ptr == '\0')) {
          return (long) pos;
        }
        break;
      case MAX_INT:            /* 'firstnonblank' */
        for (i = 0; i < len; i++) {
          if (scurr->highlight_type[i] == THE_SYNTAX_NONE) {
            /* if the current syntax setting for the current character is NOT set, then we can test for a comment... */
            if (fd->parser->case_sensitive) {
              ch1 = work[i];
              ch2 = *ptr;
            } else {
              ch1 = tolower(work[i]);
              ch2 = tolower(*ptr);
            }
            switch (state) {
              case STATE_START:
                if (ch1 != THE_CHAR_SPACE) {
                  if (ch1 == ch2) {
                    state = STATE_COMMENT;
                    pos = i;
                    ptr++;
                  } else {
                    state = STATE_IGNORE;
                  }
                }
                break;
              case STATE_COMMENT:
                if (ch1 == ch2) {
                  ptr++;
                  break;
                }
                if (ch2 == '\0') {
                  state = STATE_MATCH;
                  break;
                }
                state = STATE_IGNORE;
                break;
            }
            if (state == STATE_MATCH || state == STATE_IGNORE) {
              break;
            }
          }
        }
        if (state == STATE_MATCH || (state == STATE_COMMENT && *ptr == '\0')) {
          return (long) pos;
        }
        break;
      default:                 /* specific column */
        for (i = curr->column; i < len; i++) {
          if (scurr->highlight_type[i] == THE_SYNTAX_NONE) {
            /* if the current syntax setting for the current character is NOT set, then we can test for a comment... */
            if (fd->parser->case_sensitive) {
              ch1 = work[i];
              ch2 = *ptr;
            } else {
              ch1 = tolower(work[i]);
              ch2 = tolower(*ptr);
            }
            if (ch2 == '\0') {
              return (long) curr->column;
            }
            if (ch2 != ch1) {
              break;
            }
            ptr++;
          }
        }
        break;
    }
  }
  return -1L;
}

static short parse_line_comments(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  long i, len = scurr->length;
  long vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  long start_line_comment = -1;
  chtype comment_colour;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    comment_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_COMMENTS);
  } else if (scurr->is_current_line) {
    comment_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_COMMENTS);
  } else {
    comment_colour = set_colour(fd->ecolour + ECOLOUR_COMMENTS);
  }
  start_line_comment = find_line_comments(scrno, fd, scurr);
  if (start_line_comment != -1) {
    for (i = start_line_comment; i < len; i++) {
      if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
        scurr->highlighting[i - vcol] = comment_colour;
      }
      scurr->highlight_type[i] = THE_SYNTAX_COMMENT;
      if (work[i] != THE_CHAR_SPACE) {
        number_blanks++;
        work[i] = THE_CHAR_SPACE;
      }
    }
  }
  return RC_OK;
}

/*
 * Sets the starting location to search for the next paired comment delimiter
 */
static int reset_paired_comments_locations(uchar scrno, FILE_DETAILS *fd, comment_loc *locations, int type) {
  long col;
  int row;
  SHOW_LINE *scurr;

  /*
   * Based on the location of the end delimiter, set the search row/column to start after it
   */
  if (type == STATE_START_TAG) {
    row = locations->start_row;
    col = locations->start_column;
  } else {
    row = locations->end_row;
    col = locations->end_column;
  }
  /*
   * Now we know where the end of the last search was, work out where the next one starts from
   */
  scurr = screen[scrno].sl + row;
  if (col + 1 > scurr->length) {        /* we are past the end of the current line... */
    if (row + 1 > screen[scrno].rows[WINDOW_FILEAREA]) {
      /* we are at the end of the display, let the caller know */
      return 1;
    }
    locations->start_search_row = row + 1;
    locations->start_search_column = 0;
  } else {
    locations->start_search_row = row;
    locations->start_search_column = col + 1;
  }
  return 0;
}

/*
 * Using the location of the start and end of the paired comments,
 * set the syntax highlighting to all characters within that range
 */
static int set_paired_comments(uchar scrno, FILE_DETAILS *fd, comment_loc *locations) {
  int i;
  long start, end, j;
  SHOW_LINE *scurr;
  chtype comment_colour;
  long vcol = SCREEN_VIEW(scrno)->verify_col - 1;

  for (i = locations->start_row; i <= locations->end_row; i++) {
    scurr = screen[scrno].sl + i;
    if (scurr->line_type == LINE_LINE && scurr->length > 0) {
      /*
       * Only test if this is a valid line
       */
      if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
        comment_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_COMMENTS);
      } else if (scurr->is_current_line) {
        comment_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_COMMENTS);
      } else {
        comment_colour = set_colour(fd->ecolour + ECOLOUR_COMMENTS);
      }
      /*
       * Determine the columns affected
       */
      if (locations->start_row == locations->end_row) {
        /* all on one row */
        start = locations->start_column;
        end = min(scurr->length - 1, locations->end_column);
      } else if (i == locations->start_row) {
        /* first line */
        start = locations->start_column;
        end = scurr->length - 1;
      } else if (i == locations->end_row) {
        /* last line */
        start = 0;
        end = min(scurr->length - 1, locations->end_column);
      } else {
        /* in the middle */
        start = 0;
        end = scurr->length - 1;
      }
      for (j = start; j <= end; j++) {
        if (j >= vcol && j - vcol < THE_MAX_SCREEN_WIDTH) {
          scurr->highlighting[j - vcol] = comment_colour;
        }
        scurr->highlight_type[j] = THE_SYNTAX_COMMENT;
      }
    }
  }
  return 0;
}

/*
 * Based on the supplied starting location,
 * look for a paired comment delimiter from the current location
 * to the end of the displayed portion of the file
 */
static int find_paired_comment_delim(uchar scrno, FILE_DETAILS *fd, int state, comment_loc *locations, uchar *start_delim, uchar *end_delim) {
  uchar *starter = NULL, *ender = NULL;
  long i;
  long pos = 0;
  int type = 0;
  SHOW_LINE *scurr;

  /*
   * We never get called with STATE_START_TAG!
   *
   * If the supplied state is STATE_END_TAG, then starting at the supplied start location (row and column)
   * check each character backwardsforwards until a matching end_delim is found.
   * When found, set ??? and return STATE_END_TAG
   * Otherwise return STATE_IGNORE
   *
   * If the supplied state is STATE_START then starting at the supplied start location (row and column)
   * check each character forwards until a matching start_delim or end_delim is found.
   * When found, set ??? and return STATE_END_TAG or STATE_START_TAG (depending on what was found)
   * Otherwise return STATE_IGNORE
   */
  if (state == STATE_START_TAG) {
    starter = start_delim;
  } else if (state == STATE_END_TAG) {
    ender = end_delim;
  } else {
    starter = start_delim;
    ender = end_delim;
  }
  pos = locations->start_search_column;
  for (i = locations->start_search_row; i < screen[scrno].rows[WINDOW_FILEAREA]; i++) {
    scurr = screen[scrno].sl + i;
    if (scurr->line_type == LINE_LINE && scurr->length > 0) {
      pos = find_comment(scrno, scurr, fd, pos, scurr->length, scurr->contents, starter, ender, &type);
      if (pos == -1) {
        /*
         * No match on this line, start at beginning of next line
         */
        pos = 0;
      } else {
        /*
         * We have a match at pos
         */
        locations->type = type;
        if (type == STATE_START_TAG) {
          locations->start_row = i;
          locations->start_column = pos;
        } else {
          locations->end_row = i;
          locations->end_column = pos + strlen((char *) end_delim) - 1;
        }
        return type;
      }
    }
  }
  return STATE_IGNORE;
}

short parse_paired_comments(uchar scrno, FILE_DETAILS *fd) {
  PARSE_COMMENTS *curr_comments = fd->parser->first_comments;
  int type = 0;
  comment_loc locations;

  /*
   * Starting with the first displayed line of the file,
   * search until the end of the displayed part of the file for a start or end paired comment.
   * Only look for matches where the highlight_type for a displayed character is THE_SYNTAX_NONE
   *
   * If we find a match for a 'start of comment' then search for the 'end of comment'
   * from the character after the match to the end of the current line, and on subsequent lines forward.
   * Ignore the highlight_type when looking for the 'end of comment' ONLY on the same line as the 'start of comment'.
   * For all other lines ignore characters with highlight_type != THE_SYNTAX_NONE.
   * If the 'end of comment' is found, set highlight_type for all characters between 'start of comment'
   * and 'end of comment' and repeat the above from the character position after the 'end of comment'
   * If we did not find 'end of comment', set highlight_type from 'start of comment' to end of display,
   * and break - no more parsing to be done
   *
   * If we find a match for an 'end of comment' then search for the 'start of comment' from the character
   * before the match to the start of the current line, and on subsequent lines backwards.
   * Ignore the highlight_type when looking for the 'start of comment' ONLY on the same line as the 'start of comment'.
   * For all other lines ignore characters with highlight_type != THE_SYNTAX_NONE.
   *
   * Repeat the above until we have parsed every line in the view
   *
   * -
   * Find the specified line comments
   */
  for (; curr_comments != NULL; curr_comments = curr_comments->next) {
    if (curr_comments->line_comment) {
      continue;
    }
    type = STATE_START;
    memset(&locations, 0, sizeof(locations));
    for (;;) {
      type = find_paired_comment_delim(scrno, fd, type, &locations, curr_comments->start_delim, curr_comments->end_delim);
      if (type == STATE_IGNORE) { /* no more comment delimiters */
        break;
      }
      if (type == STATE_START_TAG) {
        /*
         * Reset locations struct to start searching from start delimiter
         */
        if (reset_paired_comments_locations(scrno, fd, &locations, STATE_START_TAG)) {
          break;
        }
        /* look forward for the matching end delimiter */
        type = find_paired_comment_delim(scrno, fd, STATE_END_TAG, &locations, curr_comments->start_delim, curr_comments->end_delim);
        if (type == STATE_END_TAG) {
          set_paired_comments(scrno, fd, &locations);
          /*
           * Reset locations struct to start searching from end
           */
          type = STATE_START;
          if (reset_paired_comments_locations(scrno, fd, &locations, STATE_END_TAG)) {
            break;
          }
        } else {
          /*
           * must have received a STATE_IGNORE,
           * so comment everything from start location to end of display and get out
           */
          locations.end_row = screen[scrno].rows[WINDOW_FILEAREA] - 1;
          locations.end_column = THE_MAX_SCREEN_WIDTH;
          set_paired_comments(scrno, fd, &locations);
          break;
        }
      } else if (type == STATE_END_TAG) {
        locations.start_row = 0;        /* should this be first row displayed ? */
        locations.start_column = 0;
        /* set all code from the end delimiter back to start of display to a comment */
        set_paired_comments(scrno, fd, &locations);
        /*
         * Reset locations struct to start searching from end
         */
        type = STATE_START;
        if (reset_paired_comments_locations(scrno, fd, &locations, STATE_END_TAG)) {
          break;
        }
      }
    }
  }
  return RC_OK;
}

static short parse_strings(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  int i;
  long first_quote, len = scurr->length;
  long vcol, off = 0;
  chtype string_colour, incomplete_string_colour;
  uchar single_quote = '\'';
  uchar double_quote = '\"';
  uchar backslash = '\\';
  uchar tab_character = '\t';
  int state;
  bool check_single_quote = FALSE;
  bool check_double_quote = FALSE;
  bool backslash_single_quote = FALSE;
  bool backslash_double_quote = FALSE;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    string_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_STRINGS);
    incomplete_string_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_INC_STRING);
  } else if (scurr->is_current_line) {
    string_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_STRINGS);
    incomplete_string_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_INC_STRING);
  } else {
    string_colour = set_colour(fd->ecolour + ECOLOUR_STRINGS);
    incomplete_string_colour = set_colour(fd->ecolour + ECOLOUR_INC_STRING);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  if (fd->parser->check_single_quote) {
    check_single_quote = TRUE;
  }
  if (fd->parser->check_double_quote) {
    check_double_quote = TRUE;
  }
  if (fd->parser->backslash_single_quote) {
    backslash_single_quote = TRUE;
  }
  if (fd->parser->backslash_double_quote) {
    backslash_double_quote = TRUE;
  }
  if (check_single_quote || check_double_quote) {
    first_quote = len + 1;
    state = STATE_START;
    for (i = 0; i < len; i++) {
      if (i >= vcol) {
        off = i - vcol;
      }
      switch (state) {
        case STATE_START:
          if (work[i] == single_quote && check_single_quote) {
            state = STATE_SINGLE_QUOTE;
            first_quote = i;
            if (i >= vcol && off < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[off] = string_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_STRING;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
            break;
          }
          if (work[i] == double_quote && check_double_quote) {
            state = STATE_DOUBLE_QUOTE;
            first_quote = i;
            if (i >= vcol && off < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[off] = string_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_STRING;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
            break;
          }
          /*
           * Convert all tab characters to spaces for later
           */
          if (work[i] == tab_character) {
            number_blanks++;
            work[i] = THE_CHAR_SPACE;
            break;
          }
          break;
        case STATE_SINGLE_QUOTE:
          if (work[i] == backslash) {
            if (backslash_single_quote) {
              if (i + 1 != len) {
                work[i + 1] = THE_CHAR_SPACE;
                number_blanks++;
              }
            }
          } else if (work[i] == single_quote) {
            state = STATE_START;
            first_quote = len + 1;
          }
          if (i >= vcol && off < THE_MAX_SCREEN_WIDTH) {
            scurr->highlighting[off] = string_colour;
          }
          scurr->highlight_type[i] = THE_SYNTAX_STRING;
          if (work[i] != THE_CHAR_SPACE) {
            number_blanks++;
            work[i] = THE_CHAR_SPACE;
          }
          break;
        case STATE_DOUBLE_QUOTE:
          if (work[i] == backslash) {
            if (backslash_double_quote) {
              if (i + 1 != len) {
                work[i + 1] = THE_CHAR_SPACE;
                number_blanks++;
              }
            }
          } else if (work[i] == double_quote) {
            state = STATE_START;
            first_quote = len + 1;
          }
          if (i >= vcol && off < THE_MAX_SCREEN_WIDTH) {
            scurr->highlighting[off] = string_colour;
          }
          scurr->highlight_type[i] = THE_SYNTAX_STRING;
          if (work[i] != THE_CHAR_SPACE) {
            number_blanks++;
            work[i] = THE_CHAR_SPACE;
          }
          break;
        default:
          break;
      }
    }
    if (first_quote < len) {
      for (i = first_quote; i < len; i++) {
        if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
          scurr->highlighting[i - vcol] = incomplete_string_colour;
        }
        scurr->highlight_type[i] = THE_SYNTAX_INCOMPLETESTRING;
      }
    }
  }
  return RC_OK;
}

static short parse_delimiters(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr, uchar *start, uchar *end, chtype colour, uchar highlight_type) {
  int i, j, state;
  long len = scurr->length, pos = 0, len_plus_one;
  long vcol;
  uchar *ptr_start = start, *ptr_end = end;

  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  state = STATE_START;
  len_plus_one = len + 1;
  for (i = 0; i < len_plus_one; i++) {
    switch (state) {
      case STATE_START:
        if (work[i] == *ptr_start) {
          state = STATE_START_TAG;
          pos = i;
          ptr_start++;
          break;
        }
        break;
      case STATE_START_TAG:
        if (*ptr_start == '\0') {
          state = STATE_MATCH;
          ptr_start = start;
          ptr_end = end;
          break;
        }
        if (work[i] != *ptr_start) {
          state = STATE_START;
          ptr_start = start;
          break;
        }
        ptr_start++;
        break;
      case STATE_MATCH:        /* now look for end delim */
        if (work[i] == *ptr_end) {
          state = STATE_END_TAG;
          ptr_end++;
          break;
        }
        break;
      case STATE_END_TAG:
        if (*ptr_end == '\0') {
          /*
           * We have a match
           */
          for (j = pos; j < i; j++) {
            if (work[j] != THE_CHAR_SPACE) {
              if (j >= vcol && j - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[j - vcol] = colour;
              }
              scurr->highlight_type[j] = highlight_type;
              number_blanks++;
              work[j] = THE_CHAR_SPACE;
            }
          }
          if (work[i] == *ptr_start) {
            pos = i;
            ptr_start++;
            state = STATE_START_TAG;
          } else {
            state = STATE_START;
          }
          break;
        }
        if (work[i] != *ptr_end) {
          state = STATE_START;
          ptr_start = start;
          break;
        }
        ptr_end++;
        break;
    }
  }
  return RC_OK;
}

static short parse_markup_tag(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  chtype colour;
  short rc;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_HTML_TAG);
  } else if (scurr->is_current_line) {
    colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_HTML_TAG);
  } else {
    colour = set_colour(fd->ecolour + ECOLOUR_HTML_TAG);
  }
  rc = parse_delimiters(scrno, fd, scurr, fd->parser->markup_tag_start_delim, fd->parser->markup_tag_end_delim, colour, THE_SYNTAX_MARKUP);
  return rc;
}

static short parse_markup_reference(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  chtype colour;
  short rc;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_HTML_CHAR);
  } else if (scurr->is_current_line) {
    colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_HTML_CHAR);
  } else {
    colour = set_colour(fd->ecolour + ECOLOUR_HTML_CHAR);
  }
  rc = parse_delimiters(scrno, fd, scurr, fd->parser->markup_reference_start_delim, fd->parser->markup_reference_end_delim, colour, THE_SYNTAX_MARKUP);
  return rc;
}

static short parse_headers(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  PARSE_HEADERS *curr = fd->parser->first_header;
  long i, j, k, vcol, pos = 0, len = scurr->length;
  uchar *ptr;
  int state;
  bool found = FALSE;
  chtype header_colour, original_header_colour;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_header_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_HEADER);
  } else if (scurr->is_current_line) {
    original_header_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_HEADER);
  } else {
    original_header_colour = set_colour(fd->ecolour + ECOLOUR_HEADER);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  for (; curr != NULL; curr = curr->next) {
    if (curr->alternate != 255) {       /* was an alternate colour specified for this header */
      if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
        header_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + curr->alternate);
      } else if (scurr->is_current_line) {
        header_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + curr->alternate);
      } else {
        header_colour = set_colour(fd->ecolour + curr->alternate);
      }
    } else {
      header_colour = original_header_colour;
    }
    ptr = curr->header_delim;
    state = STATE_START;
    switch (curr->header_column) {
      case 0:                  /* 'any' column */
        for (i = 0; i < len; i++) {
          /*
           * Ignore spaces, they are either "real" spaces or
           * already-highlighted characters
           */
          if (work[i] == THE_CHAR_SPACE) {
            i++;
            continue;
          }
          switch (state) {
            case STATE_START:
              if (work[i] == *ptr) {
                state = STATE_HEADER;
                pos = i;
                ptr++;
                break;
              }
              break;
            case STATE_HEADER:
              if (*ptr == '\0') {
                state = STATE_MATCH;
                break;
              }
              if (work[i] != *ptr) {
                state = STATE_START;
                ptr = curr->header_delim;
                break;
              }
              ptr++;
              break;
          }
          if (state == STATE_MATCH) {
            break;
          }
        }
        if (state == STATE_MATCH || (state == STATE_HEADER && *ptr == '\0')) {
          for (i = pos; i < len; i++) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = header_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_HEADER;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
          }
          found = TRUE;
        }
        break;
      case MAX_INT:            /* 'firstnonblank' */
        for (i = 0; i < len; i++) {
          switch (state) {
            case STATE_START:
              if (work[i] != THE_CHAR_SPACE) {
                if (work[i] == *ptr) {
                  state = STATE_HEADER;
                  pos = i;
                  ptr++;
                } else {
                  state = STATE_IGNORE;
                }
              }
              break;
            case STATE_HEADER:
              if (work[i] == *ptr) {
                ptr++;
                break;
              }
              if (*ptr == '\0') {
                state = STATE_MATCH;
                break;
              }
              state = STATE_IGNORE;
              break;
          }
          if (state == STATE_MATCH || state == STATE_IGNORE) {
            break;
          }
        }
        if (state == STATE_MATCH || (state == STATE_HEADER && *ptr == '\0')) {
          for (i = pos; i < len; i++) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = header_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_HEADER;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
          }
          found = TRUE;
        }
        break;
      default:                 /* specific column */
        for (i = curr->header_column - 1, j = 0; i < len; i++, j++, ptr++) {
          if (*ptr == '\0') {
            for (k = curr->header_column - 1; k < len; k++) {
              if (k >= vcol && k - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[k - vcol] = header_colour;
              }
              scurr->highlight_type[k] = THE_SYNTAX_HEADER;
              if (work[k] != THE_CHAR_SPACE) {
                number_blanks++;
                work[k] = THE_CHAR_SPACE;
              }
            }
            found = TRUE;
            break;
          }
          if (*ptr != work[i]) {
            break;
          }
        }
        break;
    }
    if (found) {
      break;
    }
  }
  return RC_OK;
}

static short parse_labels(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  long vcol, pos = 0, end = 0, len = scurr->length, len_label;
  long i, j, k;
  uchar *ptr;
  int state;
  chtype label_colour;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    label_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_LABEL);
  } else if (scurr->is_current_line) {
    label_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_LABEL);
  } else {
    label_colour = set_colour(fd->ecolour + ECOLOUR_LABEL);
  }
  len_label = fd->parser->len_label_delim;
  ptr = fd->parser->label_delim;
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  if (len_label == 0) {
    /*
     * column n
     */
    pos = fd->parser->label_column - 1;
    if (pos >= 0 && len > pos && work[pos] != THE_CHAR_SPACE) {
      for (i = pos; i < len; i++) {
        if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
          scurr->highlighting[i - vcol] = label_colour;
        }
        scurr->highlight_type[i] = THE_SYNTAX_LABEL;
        if (work[i] != THE_CHAR_SPACE) {
          number_blanks++;
          work[i] = THE_CHAR_SPACE;
        }
      }
    }
  } else {
    /*
     * delimiter ...
     */
    state = STATE_START;
    switch (fd->parser->label_column) {
      case 0:                  /* 'any' column */
        for (i = 0; i < len; i++) {
          /*
           * Ignore spaces, they are either "real" spaces or
           * already-highlighted characters
           */
          if (work[i] == THE_CHAR_SPACE) {
            continue;
          }
          switch (state) {
            case STATE_START:
              if (work[i] == *ptr) {
                state = STATE_LABEL;
                pos = i;
                ptr++;
                break;
              }
              break;
            case STATE_LABEL:
              if (*ptr == '\0') {
                state = STATE_MATCH;
                break;
              }
              if (work[i] != *ptr) {
                state = STATE_START;
                ptr = fd->parser->label_delim;
                break;
              }
              ptr++;
              break;
          }
          if (state == STATE_MATCH) {
            break;
          }
        }
        if (state == STATE_MATCH || (state == STATE_LABEL && *ptr == '\0')) {
          for (i = pos; i > -1; i--) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = label_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_LABEL;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
          }
        }
        break;
      /*
       * firstnonblank
       * the first word on the line must end with the specified delimiter.
       * e.g.
       *     label::
       *     -------  is the valid label if '::' is the delimiter
       *     junk label::
       *              is not
       *     junk label::
       *              is not
       */
      case MAX_INT:            /* 'firstnonblank' */
        end = len;
        for (i = 0; i < len; i++) {
          switch (state) {
            case STATE_START:
              if (work[i] != THE_CHAR_SPACE) {
                state = STATE_LABEL;
                pos = i;
              }
              break;
            case STATE_LABEL:
              if (work[i] == THE_CHAR_SPACE) {
                state = STATE_IGNORE;
              } else if (work[i] == *ptr) {
                state = STATE_DELIMITER;
                ptr++;
              }
              break;
            case STATE_DELIMITER:
              if (*ptr == '\0') {
                end = i;
                state = STATE_MATCH;
                break;
              }
              if (work[i] == *ptr) {
                ptr++;
                break;
              }
              state = STATE_IGNORE;
              break;
          }
          if (state == STATE_MATCH || state == STATE_IGNORE) {
            break;
          }
        }
        if (state == STATE_MATCH || (state == STATE_DELIMITER && *ptr == '\0')) {
          for (i = pos; i < end; i++) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = label_colour;
              scurr->highlight_type[i - vcol] = THE_SYNTAX_LABEL;
            }
            scurr->highlight_type[i] = THE_SYNTAX_LABEL;
            if (work[i] != THE_CHAR_SPACE) {
              number_blanks++;
              work[i] = THE_CHAR_SPACE;
            }
          }
        }
        break;
      default:                 /* specific column */
        for (i = fd->parser->label_column - 1, j = 0; i < len; i++, j++, ptr++) {
          if (*ptr == '\0') {
            for (k = fd->parser->label_column - 1; k > -1; k--) {
              if (k >= vcol && k - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[k - vcol] = label_colour;
              }
              scurr->highlight_type[k] = THE_SYNTAX_LABEL;
              if (work[k] != THE_CHAR_SPACE) {
                number_blanks++;
                work[k] = THE_CHAR_SPACE;
              }
            }
            break;
          }
          if (*ptr != work[i]) {
            break;
          }
        }
        break;
    }
  }
  return RC_OK;
}

static short parse_match(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  int i;
  long len = scurr->length;
  long vcol, off = 0;
  chtype paren_colour;

  /*
   * This is incomplete.
   * It needs to determine the level of bracket matching and to check multiple lines.
   */
  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    paren_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_LEVEL_1_PAREN);
  } else if (scurr->is_current_line) {
    paren_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_LEVEL_1_PAREN);
  } else {
    paren_colour = set_colour(fd->ecolour + ECOLOUR_LEVEL_1_PAREN);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  for (i = 0; i < len; i++) {
    if (i >= vcol) {
      off = i - vcol;
    }
    if (work[i] == '(' || work[i] == ')' || work[i] == '[' || work[i] == ']' || work[i] == '{' || work[i] == '}') {
      if (i >= vcol && off < THE_MAX_SCREEN_WIDTH) {
        scurr->highlighting[off] = paren_colour;
      }
      scurr->highlight_type[i] = THE_SYNTAX_MATCH;
      work[off] = THE_CHAR_SPACE;
      number_blanks++;
    }
  }
  return RC_OK;
}

static short parse_directory(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  PARSE_EXTENSION *curr;
  long i, vcol, len = scurr->length, start;
  bool found = FALSE;
  chtype directory_colour;
  uchar alternate, type;

  vcol = SCREEN_VIEW(scrno)->verify_col - 1;

  /*
   * Check directory, link, extensions, then executable
   */
  if (fd->parser->have_directory_directory) {
    /*
     * A directory under unix if first character is 'd'
     * A directory under other platforms if line starts with "....(dir)"
     */
    if (len >= 1) {
      if (work[0] == 'd') {
        alternate = fd->parser->directory_option_alternate;
        type = THE_SYNTAX_DIRECTORY;
        found = TRUE;
      } else if (len >= 9) {
        if (memcmp(work + 4, "(dir)", 5) == 0) {
          alternate = fd->parser->directory_option_alternate;
          type = THE_SYNTAX_DIRECTORY;
          found = TRUE;
        }
      }
    }
  }
  if (!found && fd->parser->have_directory_link) {
    /*
     * A link under unix if first character is 'l'
     */
    if (len >= 1) {
      if (work[0] == 'l') {
        alternate = fd->parser->link_option_alternate;
        type = THE_SYNTAX_LINK;
        found = TRUE;
      }
    }
  }
  if (!found && fd->parser->have_extensions) {
    /*
     * Loop through the linked list and check if the string matches the
     * end of the directory line
     */
    for (curr = fd->parser->first_extension; curr != NULL; curr = curr->next) {
      if (len >= FILE_START + curr->extension_length) {
        start = len - curr->extension_length;
        if (fd->parser->case_sensitive) {
          if (memcmp(work + start, curr->extension, curr->extension_length) == 0) {
            alternate = curr->alternate;
            type = THE_SYNTAX_EXTENSION;
            found = TRUE;
            break;
          }
        } else {
          if (memcmpi(work + start, curr->extension, curr->extension_length) == 0) {
            alternate = curr->alternate;
            type = THE_SYNTAX_EXTENSION;
            found = TRUE;
            break;
          }
        }
      }
    }
  }
  if (!found && fd->parser->have_executable) {
    /*
     * An executable under unix if 'x' in either owner, group or world
     */
    if (len >= 4 && work[3] == 'x') {
      alternate = fd->parser->executable_option_alternate;
      type = THE_SYNTAX_EXECUTABLE;
      found = TRUE;
    } else if (len >= 7 && work[6] == 'x') {
      alternate = fd->parser->executable_option_alternate;
      type = THE_SYNTAX_EXECUTABLE;
      found = TRUE;
    } else if (len >= 10 && work[9] == 'x') {
      alternate = fd->parser->executable_option_alternate;
      type = THE_SYNTAX_EXECUTABLE;
      found = TRUE;
    }
  }
  /*
   * Set the ECOLOUR
   */
  if (found) {
    if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
      directory_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alternate);
    } else if (scurr->is_current_line) {
      directory_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alternate);
    } else {
      directory_colour = set_colour(fd->ecolour + alternate);
    }
    /*
     * Change the line colours
     */
    for (i = 0; i < len; i++) {
      if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
        scurr->highlighting[i - vcol] = directory_colour;
        scurr->highlight_type[i - vcol] = type;
      }
      scurr->highlight_type[i] = type;
      if (work[i] != THE_CHAR_SPACE) {
        number_blanks++;
        work[i] = THE_CHAR_SPACE;
      }
    }
  }
  return RC_OK;
}

static int match(regex_t *preg, char *string, int size) {
  regmatch_t m;

  if (0 == regexec(preg, string, 1, &m, 0) && m.rm_eo <= size) {
    return m.rm_eo - m.rm_so;
  }
  return -1;
}

static short parse_postcompare(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  PARSE_POSTCOMPARE *curr;
  long i, j, vcol, len = scurr->length;
  bool found_for_column, individual_found;
  chtype postcompare_colour, original_postcompare_colour;
  long re_len;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_postcompare_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_KEYWORDS);
  } else if (scurr->is_current_line) {
    original_postcompare_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_KEYWORDS);
  } else {
    original_postcompare_colour = set_colour(fd->ecolour + ECOLOUR_KEYWORDS);
  }

  vcol = SCREEN_VIEW(scrno)->verify_col - 1;

  for (i = 0; i < len;) {
    /*
     * For the most common non-relevant character; ' '
     * check it manually, because its quicker than re_match().
     */
    if (work[i] == THE_CHAR_SPACE) {
      i++;
      continue;
    }
    found_for_column = FALSE;
    for (curr = fd->parser->first_postcompare; curr != NULL; curr = curr->next) {
      individual_found = FALSE;
      if (curr->is_class_type == TRUE) {
        /* CLASS TYPE */
        re_len = match(&curr->pattern_buffer, (char *) work + i, len - i);
        if (re_len > 0) {
          individual_found = TRUE;
        }
      } else {
        /* STRING type */
        re_len = strlen((char *) curr->string);
        if (re_len <= len - i && memcmp((char *) curr->string, (char *) work + i, re_len) == 0) {
          individual_found = TRUE;
        }
      }
      if (individual_found) {
        if (curr->alternate != 255) {   /* was an alternate colour specified for this postcompare */
          if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
            postcompare_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + curr->alternate);
          } else if (scurr->is_current_line) {
            postcompare_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + curr->alternate);
          } else {
            postcompare_colour = set_colour(fd->ecolour + curr->alternate);
          }
        } else {
          postcompare_colour = original_postcompare_colour;
        }
        for (j = 0; j < re_len; j++, i++) {
          if (work[i] != THE_CHAR_SPACE) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = postcompare_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_POSTCOMPARE;
            work[i] = THE_CHAR_SPACE;
            number_blanks++;
          }
        }
        found_for_column = TRUE;
      }
    }
    if (!found_for_column) {
      i++;
    }
  }
  return RC_OK;
}

static short parse_columns(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  long i, j, vcol, len = scurr->length;
  chtype column_colour, original_column_colour;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_column_colour = set_colour(fd->attr + ATTR_CURSORLINE);
  } else if (scurr->is_current_line) {
    original_column_colour = set_colour(fd->attr + ATTR_CURLINE);
  } else {
    original_column_colour = set_colour(fd->attr + ATTR_FILEAREA);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  for (i = 0; i < fd->parser->number_columns; i++) {
    if (fd->parser->column_alternate[i] != 255) {       /* was an alternate colour specified for this column */
      if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
        column_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + fd->parser->column_alternate[i]);
      } else if (scurr->is_current_line) {
        column_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + fd->parser->column_alternate[i]);
      } else {
        column_colour = set_colour(fd->ecolour + fd->parser->column_alternate[i]);
      }
    } else {
      column_colour = original_column_colour;
    }
    for (j = fd->parser->first_column[i] - 1; j < len && j < fd->parser->last_column[i]; j++) {
      if (j >= vcol && j - vcol < THE_MAX_SCREEN_WIDTH) {
        scurr->highlighting[j - vcol] = column_colour;
      }
      scurr->highlight_type[j] = '?';
      work[j] = THE_CHAR_SPACE;
      number_blanks++;
    }
  }
  return RC_OK;
}

short find_preprocessor(FILE_DETAILS *fd, uchar *word, int len, int *alternate_colour) {
  PARSE_KEYWORDS *curr = fd->parser->first_keyword;
  uchar preprocessor_char = fd->parser->preprocessor_char;
  bool found;

  for (; curr != NULL; curr = curr->next) {
    if (len == curr->keyword_length - 1) {
      found = FALSE;
      if (*(curr->keyword) == preprocessor_char) {
        if (fd->parser->case_sensitive) {
          if (memcmp(word, curr->keyword + 1, len) == 0) {
            found = TRUE;
          }
        } else {
          if (memcmpi(word, curr->keyword + 1, len) == 0) {
            found = TRUE;
          }
        }
      }
      if (found) {
        *alternate_colour = curr->alternate;
        return len;
      }
    }
  }
  return 0;
}

bool find_keyword(FILE_DETAILS *fd, uchar *word, int len, int *alternate_colour) {
  PARSE_KEYWORDS *curr = fd->parser->first_keyword;
  bool found = FALSE;

  for (; curr != NULL; curr = curr->next) {
    if (len == curr->keyword_length) {
      if (fd->parser->case_sensitive) {
        if (memcmp(word, curr->keyword, len) == 0) {
          found = TRUE;
        }
      } else {
        if (memcmpi(word, curr->keyword, len) == 0) {
          found = TRUE;
        }
      }
      if (found) {
        *alternate_colour = curr->alternate;
        break;
      }
    }
  }
  return found;
}

static short parse_keywords(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  int i, j, alt = 0;
  long len = scurr->length;
  long vcol;
  long re_len, save_re_len;
  chtype keyword_colour, original_keyword_colour;
  chtype number_colour;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_KEYWORDS);
    number_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_NUMBERS);
  } else if (scurr->is_current_line) {
    original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_KEYWORDS);
    number_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_NUMBERS);
  } else {
    original_keyword_colour = set_colour(fd->ecolour + ECOLOUR_KEYWORDS);
    number_colour = set_colour(fd->ecolour + ECOLOUR_NUMBERS);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  for (i = 0; i < len;) {
    /*
     * For the most common non-relevant character; ' '
     * check it manually, because its quicker than re_match().
     */
    if (work[i] == THE_CHAR_SPACE) {
      i++;
    } else {
      /*
       * Find an identfier
       */
      re_len = match(&fd->parser->body_pattern_buffer, (char *) work + i, len - i);
      if (re_len < 0) {
        /*
         * No identifier, is it a number ??
         */
        if (fd->parser->have_number_pattern_buffer && CURRENT_VIEW->syntax_headers & HEADER_NUMBER) {
          re_len = match(&fd->parser->number_pattern_buffer, (char *) work + i, len - i);
          if (re_len > 0) {
            for (j = 0; j < re_len; j++, i++) {
              if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[i - vcol] = number_colour;
              }
              scurr->highlight_type[i] = THE_SYNTAX_NUMBER;
              if (work[i] != THE_CHAR_SPACE) {
                work[i] = THE_CHAR_SPACE;
                number_blanks++;
              }
            }
          } else {
            i++;
          }
        } else {
          i++;
        }
      } else {
        /*
         * We now have a match of an identifier; see if its a keyword
         */
        if (work[i + re_len - 1] == THE_CHAR_SPACE) {   /* bug in regex()! */
          re_len--;
        }
        save_re_len = re_len;
        if (find_keyword(fd, work + i, re_len, &alt)) {
          /*
           * Have found a matching keyword...
           */
          if (alt != 255) {     /* was an alternate colour specified for this keyword */
            if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
              keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alt);
            } else if (scurr->is_current_line) {
              keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alt);
            } else {
              keyword_colour = set_colour(fd->ecolour + alt);
            }
          } else {
            keyword_colour = original_keyword_colour;
          }
          for (j = 0; j < re_len; j++, i++) {
            if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[i - vcol] = keyword_colour;
            }
            scurr->highlight_type[i] = THE_SYNTAX_KEYWORD;
            work[i] = THE_CHAR_SPACE;
            number_blanks++;
          }
        } else {
          /*
           * We have NOT found a keyword, so check if its a number.
           * If not a number, simply remove the characters from the line
           * so it can no longer match any further token types.
           */
          if (fd->parser->have_number_pattern_buffer && CURRENT_VIEW->syntax_headers & HEADER_NUMBER) {
            re_len = match(&fd->parser->number_pattern_buffer, (char *) work + i, len - i);
            if (re_len > 0) {
              for (j = 0; j < re_len; j++, i++) {
                if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
                  scurr->highlighting[i - vcol] = number_colour;
                }
                scurr->highlight_type[i] = THE_SYNTAX_NUMBER;
                if (work[i] != THE_CHAR_SPACE) {
                  work[i] = THE_CHAR_SPACE;
                  number_blanks++;
                }
              }
            } else {
              for (j = 0; j < save_re_len; j++, i++) {
                if (work[i] != THE_CHAR_SPACE) {
                  work[i] = THE_CHAR_SPACE;
                  number_blanks++;
                }
              }
            }
          } else {
            for (j = 0; j < save_re_len; j++, i++) {
              if (work[i] != THE_CHAR_SPACE) {
                work[i] = THE_CHAR_SPACE;
                number_blanks++;
              }
            }
          }
        }
      }
    }
  }
  return RC_OK;
}

bool find_function(FILE_DETAILS *fd, uchar *word, int len, int *alternate_colour) {
  PARSE_FUNCTIONS *curr = fd->parser->first_function;
  bool found = FALSE;

  for (; curr != NULL; curr = curr->next) {
    if (len == curr->function_length) {
      if (fd->parser->case_sensitive) {
        if (memcmp(word, curr->function, len) == 0) {
          found = TRUE;
        }
      } else {
        if (memcmpi(word, curr->function, len) == 0) {
          found = TRUE;
        }
      }
      if (found) {
        *alternate_colour = curr->alternate;
        break;
      }
    }
  }
  return found;
}

static short parse_functions(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  int i, j, alt = 0;
  long len = scurr->length;
  long vcol, function_len;
  long re_len;
  chtype function_colour, original_function_colour;
  uchar *word = NULL;
  int k, word_start, word_end, state;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_function_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_FUNCTIONS);
  } else if (scurr->is_current_line) {
    original_function_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_FUNCTIONS);
  } else {
    original_function_colour = set_colour(fd->ecolour + ECOLOUR_FUNCTIONS);
  }
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  for (i = 0; i < len;) {
    /*
     * For the most common non-relevant character; ' ' check it manually,
     * because it should be quicker than re_match().
     */
    if (work[i] == THE_CHAR_SPACE) {
      i++;
    } else {
      /*
       * We have an indentfier
       */
      re_len = match(&fd->parser->function_pattern_buffer, (char *) work + i, len - i);
      if (re_len < 0) {
        /*
         * If we have the option REXX setting,
         * then the identifier we have could be a function
         * (without the brackets) eg
         *   Call Left "string", 1
         * Use the pattern buffer for identifiers to get to locate the possible function.
         */
        if (fd->parser->rexx_option) {
          re_len = match(&fd->parser->body_pattern_buffer, (char *) work + i, len - i);
          if (re_len < 0) {
            /*
             * No identifier found, try the next char...
             */
            i++;
            continue;
          } else {
            /*
             * Need to check that the previous word is "call".
             * Then we can say we have a potential Rexx function
             */
            for (word_start = 0, word_end = 0, state = STATE_START, k = i - 1; k > -1; k--) {
              switch (state) {
                case STATE_START:
                  if (work[k] != THE_CHAR_SPACE) {
                    state = STATE_START_TAG;
                    word_end = k;
                  }
                  break;
                case STATE_START_TAG:
                  if (work[k] == THE_CHAR_SPACE) {
                    state = STATE_END_TAG;
                    word_start = k + 1;
                  }
                  break;
              }
              if (state == STATE_END_TAG) {
                break;
              }
            }
            if (word_end - word_start == 3 && memcmpi(work + word_start, (uchar *) "call", 4) == 0 && word_end + 1 != i) {
              /*
               * We have found the CALL keyword.
               * Process it here so that subsequent words on this line do not get set incorrectly.
               */
              chtype keyword_colour, original_keyword_colour;
              if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
                original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_KEYWORDS);
              } else if (scurr->is_current_line) {
                original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_KEYWORDS);
              } else {
                original_keyword_colour = set_colour(fd->ecolour + ECOLOUR_KEYWORDS);
              }
              if (find_keyword(fd, (uchar *) "call", 4, &alt)) {
                if (alt != 255) {       /* was an alternate colour specified for this keyword */
                  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
                    keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alt);
                  } else if (scurr->is_current_line) {
                    keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alt);
                  } else {
                    keyword_colour = set_colour(fd->ecolour + alt);
                  }
                } else {
                  keyword_colour = original_keyword_colour;
                }
                for (j = 0; j < 4; j++) {
                  if (word_start + j >= vcol && (word_start + j) - vcol < THE_MAX_SCREEN_WIDTH) {
                    scurr->highlighting[(word_start + j) - vcol] = keyword_colour;
                  }
                  scurr->highlight_type[(word_start + j)] = THE_SYNTAX_KEYWORD;
                  if (work[word_start + j] != THE_CHAR_SPACE) {
                    work[word_start + j] = THE_CHAR_SPACE;
                    number_blanks++;
                  }
                }
              }
            } else {
              i++;
              continue;
            }
          }
        } else {
          /*
           * Not REXX option, so no function found, try the next char...
           */
          i++;
          continue;
        }
      }
      /*
       * We now have a match of a function.
       * Find the matchable string by locating either the function_char or the first space.
       */
      word = work + i;
      for (function_len = 0; function_len < re_len; function_len++, word++) {
        if (*word == THE_CHAR_SPACE || *word == fd->parser->function_char) {
          /*
           * We MUST always get here!
           */
          break;
        }
      }
      if (find_function(fd, work + i, function_len, &alt)) {
        if (alt != 255) {       /* was an alternate colour specified for this keyword */
          if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
            function_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alt);
          } else if (scurr->is_current_line) {
            function_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alt);
          } else {
            function_colour = set_colour(fd->ecolour + alt);
          }
        } else {
          function_colour = original_function_colour;
        }
        for (j = 0; j < function_len; j++, i++) {
          if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
            scurr->highlighting[i - vcol] = function_colour;
          }
          scurr->highlight_type[i] = THE_SYNTAX_FUNCTION;
          work[i] = THE_CHAR_SPACE;
          number_blanks++;
        }
      } else {
        /*
         * We haven't found an explicit function,
         * so check if we have the wildcard option set with an alternate colour
         */
        if (fd->parser->have_function_option_alternate) {
          /*
           * OK, we have the option, but we must exclude explicit keywords
           * as a construct like in C:
           *   if ( expr )
           * would be treated as a function call.
           */
          if (!find_keyword(fd, work + i, function_len, &alt)) {
            alt = fd->parser->function_option_alternate;
            if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
              function_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alt);
            } else if (scurr->is_current_line) {
              function_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alt);
            } else {
              function_colour = set_colour(fd->ecolour + alt);
            }
            for (j = 0; j < function_len; j++, i++) {
              if (i >= vcol && i - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[i - vcol] = function_colour;
              }
              scurr->highlight_type[i] = THE_SYNTAX_FUNCTION;
              if (work[i] != THE_CHAR_SPACE) {
                work[i] = THE_CHAR_SPACE;
                number_blanks++;
              }
            }
          } else {
            i += re_len;
          }
        } else {
          i += re_len;
        }
      }
    }
  }
  return RC_OK;
}

static short parse_preprocessor(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr) {
  int i, j, k, len_plus_one, state, alt = 0;
  long len = scurr->length, preprocessor_char_start_pos = 0;
  long vcol, keyword_length, keyword_start_pos = 0, re_len;
  chtype keyword_colour, original_keyword_colour;
  uchar *word = NULL;

  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + ECOLOUR_PREDIR);
  } else if (scurr->is_current_line) {
    original_keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + ECOLOUR_PREDIR);
  } else {
    original_keyword_colour = set_colour(fd->ecolour + ECOLOUR_PREDIR);
  }
  state = STATE_START;
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  len_plus_one = len + 1;
  for (i = 0; i < len_plus_one; i++) {
    switch (state) {
      case STATE_START:
        if (work[i] == THE_CHAR_SPACE) {
          break;
        }
        if (work[i] == fd->parser->preprocessor_char) {
          preprocessor_char_start_pos = i;
          state = STATE_OPTION;
          break;
        }
        state = STATE_IGNORE;
        break;
      case STATE_OPTION:
        if (work[i] == THE_CHAR_SPACE) {
          break;
        }
        word = work + i;
        keyword_start_pos = i;
        state = STATE_STRING;
        break;
      case STATE_STRING:
        if (work[i] == THE_CHAR_SPACE) {
          keyword_length = i - keyword_start_pos;
          if (keyword_length < fd->parser->min_keyword_length) {
            state = STATE_START;
            break;
          }
          work[i] = '\0';
          if ((re_len = find_preprocessor(fd, word, keyword_length, &alt))) {
            if (alt != 255) {   /* was an alternate colour specified for this keyword */
              if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
                keyword_colour = merge_curline_colour(fd->attr + ATTR_CURSORLINE, fd->ecolour + alt);
              } else if (scurr->is_current_line) {
                keyword_colour = merge_curline_colour(fd->attr + ATTR_CURLINE, fd->ecolour + alt);
              } else {
                keyword_colour = set_colour(fd->ecolour + alt);
              }
            } else {
              keyword_colour = original_keyword_colour;
            }
            for (j = 0, k = keyword_start_pos; j < re_len; j++, k++) {
              if (k >= vcol && k - vcol < THE_MAX_SCREEN_WIDTH) {
                scurr->highlighting[k - vcol] = keyword_colour;
              }
              scurr->highlight_type[k] = THE_SYNTAX_KEYWORD;
              work[k] = THE_CHAR_SPACE;
              number_blanks++;
            }
            if (preprocessor_char_start_pos >= vcol && preprocessor_char_start_pos - vcol < THE_MAX_SCREEN_WIDTH) {
              scurr->highlighting[preprocessor_char_start_pos - vcol] = keyword_colour;
            }
            scurr->highlight_type[preprocessor_char_start_pos] = THE_SYNTAX_KEYWORD;
            work[preprocessor_char_start_pos] = THE_CHAR_SPACE;
            number_blanks++;
          }
          /*
           * Don't increment number_blanks; it
           * was a blank before we set it to nul
           */
          work[i] = THE_CHAR_SPACE;
          state = STATE_IGNORE;
        }
        // fall-through
      case STATE_IGNORE:
        break;
    }
    if (state == STATE_IGNORE) {
      break;
    }
  }
  return RC_OK;
}

short parse_line(uchar scrno, FILE_DETAILS *fd, SHOW_LINE *scurr, short start_row) {
  int i;
  long len = scurr->length;
  chtype normal_colour;

  /*
   * Set all columns to default colour.
   */
  if (scurr->is_cursor_line && scurr->is_cursor_line_filearea_different) {
    normal_colour = set_colour(fd->attr + ATTR_CURSORLINE);
  } else if (scurr->is_current_line) {
    normal_colour = set_colour(fd->attr + ATTR_CURLINE);
  } else {
    normal_colour = set_colour(fd->attr + ATTR_FILEAREA);
  }
  for (i = 0; i < THE_MAX_SCREEN_WIDTH; i++) {
    scurr->highlighting[i] = normal_colour;
  }
  /*
   * The length of this line is 0, so return now; we have already set its attributes to blank
   */
  if (len == 0) {
    return RC_OK;
  }
  /*
   * Error allocating memory for highlighing; bail out
   */
  if (scurr->highlight_type == NULL) {
    return RC_OK;
  }
  memcpy((char *) brec, scurr->contents, len);
  brec[len] = THE_CHAR_SPACE;
  work = brec;
  number_blanks = 0;
  /*
   * Parse columns - must be done first
   */
  if (fd->parser->have_columns && CURRENT_VIEW->syntax_headers & HEADER_COLUMN) {
    parse_columns(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse single and or double quoted strings
   * No multi-line strings.
   */
  if (fd->parser->have_string && CURRENT_VIEW->syntax_headers & HEADER_STRING) {
    parse_strings(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse headers
   */
  if (fd->parser->have_headers && CURRENT_VIEW->syntax_headers & HEADER_HEADER) {
    parse_headers(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse single line comments
   */
  if (fd->parser->have_line_comments && CURRENT_VIEW->syntax_headers & HEADER_COMMENT) {
    parse_line_comments(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse DELIMITER labels first (COLUMN labels done later)
   */
  if (fd->parser->len_label_delim && CURRENT_VIEW->syntax_headers & HEADER_LABEL) {
    parse_labels(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Find functions; if applicable
   */
  if (fd->parser->function_option && CURRENT_VIEW->syntax_headers & HEADER_FUNCTION) {
    parse_functions(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse markup references if applicable
   */
  if (fd->parser->have_markup_reference && CURRENT_VIEW->syntax_headers & HEADER_MARKUP) {
    parse_markup_reference(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse markup tags if applicable
   */
  if (fd->parser->have_markup_tag && CURRENT_VIEW->syntax_headers & HEADER_MARKUP) {
    parse_markup_tag(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse parens/braces/brackets
   */
  if (fd->parser->have_match && CURRENT_VIEW->syntax_headers & HEADER_MATCH) {
    parse_match(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Find preprocesor keywords; if applicable
   */
  if (fd->parser->preprocessor_option) {
    parse_preprocessor(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Find keywords, numbers, and remove identifiers
   */
  if (fd->parser->have_body_pattern_buffer && CURRENT_VIEW->syntax_headers & HEADER_KEYWORD) {
    parse_keywords(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse COLUMN labels (DELIMITER labels done earlier)
   */
  if (fd->parser->len_label_delim == 0 && CURRENT_VIEW->syntax_headers & HEADER_LABEL) {
    parse_labels(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Do parsing for pseudo file DIR.DIR
   */
  if (CURRENT_FILE->pseudo_file == PSEUDO_DIR && CURRENT_VIEW->syntax_headers & HEADER_DIRECTORY) {
    parse_directory(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  /*
   * Parse postcompare - always last
   */
  if (fd->parser->have_postcompare && CURRENT_VIEW->syntax_headers & HEADER_POSTCOMPARE) {
    parse_postcompare(scrno, fd, scurr);
    if (number_blanks == len) {
      return RC_OK;
    }
  }
  return RC_OK;
}

static short construct_case(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;

  if (memcmpi(line, (uchar *) "ignore", line_length) == 0) {
    parser->case_sensitive = FALSE;
  } else if (memcmpi(line, (uchar *) "respect", line_length) == 0) {
    parser->case_sensitive = TRUE;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", line, lineno);
    display_error(216, (uchar *) tmp, FALSE);
    rc = RC_INVALID_OPERAND;
  }
  return rc;
}

#define CONMAR_PARAMS  3

static short construct_markup(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONMAR_PARAMS + 1];
  uchar strip[CONMAR_PARAMS];
  unsigned short num_params = 0;
  short option = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(line, word, CONMAR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params != 3) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (equal((uchar *) "tag", word[0], 3)) {
    option = 1;
  } else if (equal((uchar *) "reference", word[0], 9)) {
    option = 2;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (strlen((char *) word[1]) > MAX_DELIMITER_LENGTH) {
    sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[1], lineno, MAX_DELIMITER_LENGTH);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (strlen((char *) word[2]) > MAX_DELIMITER_LENGTH) {
    sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[2], lineno, MAX_DELIMITER_LENGTH);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (option == 1) {
    parser->have_markup_tag = TRUE;
    strcpy((char *) parser->markup_tag_start_delim, (char *) word[1]);
    parser->len_markup_tag_start_delim = strlen((char *) word[1]);
    strcpy((char *) parser->markup_tag_end_delim, (char *) word[2]);
    parser->len_markup_tag_end_delim = strlen((char *) word[2]);
  } else {
    parser->have_markup_reference = TRUE;
    strcpy((char *) parser->markup_reference_start_delim, (char *) word[1]);
    parser->len_markup_reference_start_delim = strlen((char *) word[1]);
    strcpy((char *) parser->markup_reference_end_delim, (char *) word[2]);
    parser->len_markup_reference_end_delim = strlen((char *) word[2]);
  }
  return rc;
}

#define CONSTR_PARAMS  3

static short construct_string(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONSTR_PARAMS + 1];
  uchar strip[CONSTR_PARAMS];
  unsigned short num_params = 0;
  short option = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(line, word, CONSTR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (equal((uchar *) "single", word[0], 6)) {
    option = STATE_SINGLE_QUOTE;
  } else if (equal((uchar *) "double", word[0], 6)) {
    option = STATE_DOUBLE_QUOTE;
  } else if (equal((uchar *) "delimiter", word[0], 9)) {
    option = STATE_DELIMITER;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (num_params == 1) {
    if (option == STATE_SINGLE_QUOTE) {
      parser->check_single_quote = TRUE;
    } else if (option == STATE_DOUBLE_QUOTE) {
      parser->check_double_quote = TRUE;
    } else {
      sprintf((char *) tmp, "Too few tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
  }
  if (num_params == 2) {
    if (option == STATE_SINGLE_QUOTE && !equal((uchar *) "backslash", word[1], 9)) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    } else {
      parser->check_single_quote = TRUE;
      parser->backslash_single_quote = TRUE;
    }
    if (option == STATE_DOUBLE_QUOTE && !equal((uchar *) "backslash", word[1], 9)) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    } else {
      parser->check_double_quote = TRUE;
      parser->backslash_double_quote = TRUE;
    }
    if (option == STATE_DELIMITER && strlen((char *) word[1]) != 1) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    } else {
      parser->string_delimiter = word[1][0];
    }
  }
  if (num_params == 3) {
    if (option != STATE_DELIMITER) {
      sprintf((char *) tmp, "Too many tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (equal((uchar *) "backslash", word[2], 9)) {
      parser->string_delimiter = word[1][0];
      parser->backslash_delimiter = TRUE;
    } else {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[2], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
  }
  parser->have_string = TRUE;
  return rc;
}

#define CONCOM_PARAMS  5

static short construct_comment(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONCOM_PARAMS + 1];
  uchar strip[CONCOM_PARAMS];
  unsigned short num_params = 0;
  short option = 0, where = 0;
  bool nested = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  num_params = param_split(line, word, CONCOM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (equal((uchar *) "line", word[0], 4)) {
    option = 1;
  } else if (equal((uchar *) "paired", word[0], 6)) {
    option = 2;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (option == 1) {
    if (num_params < 3) {
      sprintf((char *) tmp, "Too few tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (strlen((char *) word[1]) > MAX_DELIMITER_LENGTH) {
      sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[1], lineno, MAX_DELIMITER_LENGTH);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (equal((uchar *) "any", word[2], 3)) {
      if (num_params > 4) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = 0;
    } else if (equal((uchar *) "column", word[2], 3)) {
      if (!valid_positive_integer(word[3])) {
        sprintf((char *) tmp, "Invalid operand '%s', line %d", word[3], lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = atoi((char *) word[3]);
    } else if (equal((uchar *) "firstnonblank", word[2], 13)) {
      if (num_params > 4) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = MAX_INT;
    } else {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", (char *) word[2], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    parser->current_comments = parse_commentsll_add(parser->first_comments, parser->current_comments, sizeof(PARSE_COMMENTS));
    if (parser->first_comments == NULL) {
      parser->first_comments = parser->current_comments;
    }
    parser->current_comments->len_start_delim = strlen((char *) word[1]);
    strcpy((char *) parser->current_comments->start_delim, (char *) word[1]);
    parser->current_comments->column = where;
    parser->current_comments->line_comment = TRUE;
    parser->have_line_comments = TRUE;
  } else {
    if (num_params < 3) {
      sprintf((char *) tmp, "Too few tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    switch (num_params) {
      case 4:
        if (equal((uchar *) "nest", word[3], 4)) {
          nested = TRUE;
        } else if (equal((uchar *) "nonest", word[3], 6)) {
          nested = FALSE;
        } else {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[2], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
      /*
       * Fall through and setup the delimiters
       */
      case 3:
        if (strlen((char *) word[1]) > MAX_DELIMITER_LENGTH) {
          sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[1], lineno, MAX_DELIMITER_LENGTH);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (strlen((char *) word[2]) > MAX_DELIMITER_LENGTH) {
          sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[2], lineno, MAX_DELIMITER_LENGTH);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        /*
         * Everything is fine now, add the new entry to the linked list
         */
        parser->current_comments = parse_commentsll_add(parser->first_comments, parser->current_comments, sizeof(PARSE_COMMENTS));
        if (parser->first_comments == NULL) {
          parser->first_comments = parser->current_comments;
        }
        parser->current_comments->len_start_delim = strlen((char *) word[1]);
        strcpy((char *) parser->current_comments->start_delim, (char *) word[1]);
        parser->current_comments->len_end_delim = strlen((char *) word[2]);
        strcpy((char *) parser->current_comments->end_delim, (char *) word[2]);
        parser->current_comments->nested = nested;
        parser->current_comments->line_comment = FALSE;
        parser->have_paired_comments = TRUE;
        break;
      default:
        break;
    }
  }
  return rc;
}

#define CONHEA_PARAMS  6

static short construct_header(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONHEA_PARAMS + 1];
  uchar strip[CONHEA_PARAMS];
  unsigned short num_params = 0;
  short where = 0;
  short alt_start, value_idx;
  uchar alternate = 255;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(line, word, CONHEA_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (!equal((uchar *) "line", word[0], 4)) {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (num_params < 3) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (strlen((char *) word[1]) > MAX_DELIMITER_LENGTH) {
    sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[1], lineno, MAX_DELIMITER_LENGTH);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (equal((uchar *) "any", word[2], 3)) {
    if (num_params > 3 && num_params != 5) {
      sprintf((char *) tmp, "Too many tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    alt_start = 3;
    where = 0;
  } else if (equal((uchar *) "column", word[2], 3)) {
    if (num_params > 4 && num_params != 6) {
      sprintf((char *) tmp, "Too many tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (!valid_positive_integer(word[3])) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[3], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    alt_start = 4;
    where = atoi((char *) word[3]);
  } else if (equal((uchar *) "firstnonblank", word[2], 13)) {
    if (num_params > 3 && num_params != 5) {
      sprintf((char *) tmp, "Too many tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    alt_start = 3;
    where = MAX_INT;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", (char *) word[2], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Check if ALT is specified
   */
  if (num_params == alt_start + 2) {
    value_idx = alt_start + 1;
    if (equal((uchar *) "alternate", word[alt_start], 3)) {
      if (strlen((char *) word[value_idx]) > 1) {
        sprintf((char *) tmp, "ALTernate must be followed by a single character, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      if (get_alternate(word[value_idx], &alternate)) {
        sprintf((char *) tmp, "Invalid operand '%s', line %d", word[value_idx], lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return (RC_INVALID_OPERAND);
      }
    } else {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[alt_start], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return (RC_INVALID_OPERAND);
    }
  }
  parser->current_header = parse_headerll_add(parser->first_header, parser->current_header, sizeof(PARSE_HEADERS));
  if (parser->first_header == NULL) {
    parser->first_header = parser->current_header;
  }
  parser->current_header->len_header_delim = strlen((char *) word[1]);
  strcpy((char *) parser->current_header->header_delim, (char *) word[1]);
  parser->current_header->header_column = where;
  parser->current_header->alternate = alternate;
  parser->have_headers = TRUE;
  return rc;
}

#define CONLAB_PARAMS  4

static short construct_label(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONLAB_PARAMS + 1];
  uchar strip[CONLAB_PARAMS];
  unsigned short num_params = 0;
  short where = 0;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  num_params = param_split(line, word, CONLAB_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * column n
   */
  if (equal((uchar *) "column", word[0], 6)) {
    if (!valid_positive_integer(word[1])) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", (char *) word[1], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    where = atoi((char *) word[1]);
    parser->len_label_delim = 0;
    strcpy((char *) parser->label_delim, (char *) "");
  } else if (equal((uchar *) "delimiter", word[0], 9)) {
    if (num_params < 3) {
      sprintf((char *) tmp, "Too few tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (strlen((char *) word[1]) > MAX_DELIMITER_LENGTH) {
      sprintf((char *) tmp, "Token too long '%s', line %d. Length should be <= %d", (char *) word[1], lineno, MAX_DELIMITER_LENGTH);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    if (equal((uchar *) "any", word[2], 3)) {
      if (num_params > 4) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = 0;
    } else if (equal((uchar *) "column", word[2], 3)) {
      if (!valid_positive_integer(word[3])) {
        sprintf((char *) tmp, "Invalid operand '%s', line %d", (char *) word[3], lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = atoi((char *) word[3]);
    } else if (equal((uchar *) "firstnonblank", word[2], 13)) {
      if (num_params > 4) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      where = MAX_INT;
    } else {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", (char *) word[2], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
    parser->len_label_delim = strlen((char *) word[1]);
    strcpy((char *) parser->label_delim, (char *) word[1]);
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  parser->label_column = where;
  return rc;
}

static short construct_match(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;

  /*
   * Just set have_match to TRUE
   */
  parser->have_match = TRUE;
  return rc;
}

#define CONKEY_PARAMS  5

static short construct_keyword(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONKEY_PARAMS + 1];
  uchar strip[CONKEY_PARAMS];
  unsigned short num_params = 0;
  uchar alternate = 255;
  int i, num_pairs, keyword_idx, value_idx;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  num_params = param_split(line, word, CONKEY_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params > 1) {
    /*
     * Total parameters must be a multiple of 2
     */
    if ((num_params - 1) % 2 != 0) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", line, lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return (RC_INVALID_OPERAND);
    }
    num_pairs = (num_params - 1) / 2;
    for (i = 0; i < num_pairs; i++) {
      keyword_idx = 1 + (i * 2);
      value_idx = 2 + (i * 2);
      if (equal((uchar *) "alternate", word[keyword_idx], 3)) {
        if (strlen((char *) word[value_idx]) > 1) {
          sprintf((char *) tmp, "ALTernate must be followed by a single character, line %d", lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (get_alternate(word[value_idx], &alternate)) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[value_idx], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else if (equal((uchar *) "type", word[keyword_idx], 3)) {
        /* TODO */
      } else {
        sprintf((char *) tmp, "Invalid operand '%s', line %d", word[keyword_idx], lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return (RC_INVALID_OPERAND);
      }
    }
  }
  parser->current_keyword = parse_keywordll_add(parser->first_keyword, parser->current_keyword, sizeof(PARSE_KEYWORDS));
  if (parser->first_keyword == NULL) {
    parser->first_keyword = parser->current_keyword;
  }
  parser->current_keyword->keyword_length = strlen((char *) word[0]);
  parser->current_keyword->keyword = (uchar *) malloc((1 + parser->current_keyword->keyword_length) * sizeof(uchar));
  if (parser->current_keyword->keyword == NULL) {
    display_error(216, (uchar *) "out of memory", FALSE);
    return RC_OUT_OF_MEMORY;
  }
  strcpy((char *) parser->current_keyword->keyword, (char *) word[0]);
  parser->current_keyword->alternate = alternate;
  /*
   * Put keyword into hashtable - TBD
   */
  if (parser->current_keyword->keyword_length < parser->min_keyword_length) {
    parser->min_keyword_length = parser->current_keyword->keyword_length;
  }
  return rc;
}

#define CONFUN_PARAMS  5

static short construct_function(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONFUN_PARAMS + 1];
  uchar strip[CONFUN_PARAMS + 1];
  unsigned short num_params = 0;
  uchar alternate = 255;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  num_params = param_split(line, word, CONFUN_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params > 1) {
    if (equal((uchar *) "alternate", word[1], 3)) {
      if (num_params > 2) {
        if (strlen((char *) word[2]) > 1) {
          sprintf((char *) tmp, "Too few tokens, line %d", lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (get_alternate(word[2], &alternate)) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[2], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
    }
  }
  parser->current_function = parse_functionll_add(parser->first_function, parser->current_function, sizeof(PARSE_FUNCTIONS));
  if (parser->first_function == NULL) {
    parser->first_function = parser->current_function;
  }
  parser->current_function->function_length = strlen((char *) word[0]);
  parser->current_function->function = (uchar *) malloc((1 + parser->current_function->function_length) * sizeof(uchar));
  if (parser->current_function->function == NULL) {
    display_error(216, (uchar *) "out of memory", FALSE);
    return RC_OUT_OF_MEMORY;
  }
  strcpy((char *) parser->current_function->function, (char *) word[0]);
  parser->current_function->alternate = alternate;
  /*
   * Put function into hashtable - TBD
   */
  if (parser->current_function->function_length < parser->min_function_length) {
    parser->min_function_length = parser->current_function->function_length;
  }
  return rc;
}

#define CONOPT_PARAMS  6

static short construct_option(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONOPT_PARAMS + 1];
  uchar strip[CONOPT_PARAMS];
  uchar alternate;
  unsigned short num_params = 0;
  bool rexx_option = FALSE, preprocessor_option = FALSE, function_option = FALSE;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(line, word, CONOPT_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (equal((uchar *) "rexx", word[0], 4)) {
    rexx_option = TRUE;
  } else if (equal((uchar *) "preprocessor", word[0], 12)) {
    preprocessor_option = TRUE;
  } else if (equal((uchar *) "function", word[0], 8)) {
    function_option = TRUE;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (num_params == 1) {
    if (preprocessor_option || function_option) {
      sprintf((char *) tmp, "Too few tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    }
  } else {
    if (rexx_option) {
      sprintf((char *) tmp, "Too many tokens, line %d", lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return RC_INVALID_OPERAND;
    } else if (preprocessor_option) {
      if (num_params > 2) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      } else {
        if (strlen((char *) word[1]) != 1) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        parser->preprocessor_char = word[1][0];
      }
    } else if (function_option) {
      if (num_params < 3) {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      } else {
        if (strlen((char *) word[1]) != 1) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (equal((uchar *) "blank", word[2], 5)) {
          parser->function_blank = TRUE;
        } else if (equal((uchar *) "noblank", word[2], 7)) {
          parser->function_blank = FALSE;
        } else {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        parser->function_char = word[1][0];
        /*
         * The remainder of the checking is done for the optional "* alternate x"
         */
        if (num_params > 3) {
          if (num_params != 6) {
            sprintf((char *) tmp, "Too few tokens, line %d", lineno);
            display_error(216, (uchar *) tmp, FALSE);
            return RC_INVALID_OPERAND;
          }
          /*
           * Must have "DEFAULT ALTernate x"
           */
          if (!equal((uchar *) "default", word[3], 7)) {
            sprintf((char *) tmp, "Invalid operand '%s', line %d", word[3], lineno);
            display_error(216, (uchar *) tmp, FALSE);
            return RC_INVALID_OPERAND;
          }
          if (equal((uchar *) "alternate", word[4], 3)) {
            if (strlen((char *) word[5]) > 1) {
              sprintf((char *) tmp, "Invalid operand '%s', line %d", word[5], lineno);
              display_error(216, (uchar *) tmp, FALSE);
              return RC_INVALID_OPERAND;
            }
            if (get_alternate(word[5], &alternate)) {
              sprintf((char *) tmp, "Invalid operand '%s', line %d", word[5], lineno);
              display_error(216, (uchar *) tmp, FALSE);
              return (RC_INVALID_OPERAND);
            }
            parser->function_option_alternate = alternate;
            parser->have_function_option_alternate = TRUE;
          } else {
            sprintf((char *) tmp, "Invalid operand '%s', line %d", word[4], lineno);
            display_error(216, (uchar *) tmp, FALSE);
            return RC_INVALID_OPERAND;
          }
        }
      }
    }
  }
  if (preprocessor_option) {
    parser->preprocessor_option = TRUE;
  } else if (rexx_option) {
    parser->rexx_option = TRUE;
  } else if (function_option) {
    parser->function_option = TRUE;
  }
  return rc;
}

#define CONID_PARAMS  3

static short construct_identifier(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  short rc = RC_OK;
  uchar *word[CONID_PARAMS + 1];
  uchar strip[CONID_PARAMS];
  unsigned short num_params = 0;
  uchar *pattern = NULL; int n;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(line, word, CONID_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  pattern = (uchar *) malloc((20 + line_length) * sizeof(uchar));
  if (pattern == NULL) {
    display_error(216, (uchar *) "out of memory", FALSE);
    return RC_OUT_OF_MEMORY;
  }
  /*
   * Create the pattern buffer for keywords...
   */
  strcpy((char *) pattern, (char *) word[0]);
  strcat((char *) pattern, (char *) word[1]);
  strcat((char *) pattern, "*");
  if (num_params == 3) {
    /*
     * Append the optional last character RE to the end of the body RE
     */
    strcat((char *) pattern, (char *) word[2]);
  }
  /* The following memset() is not meant to use wide character */
  memset(&parser->body_pattern_buffer, 0, sizeof(regex_t));
  if (0 != (n = regcomp(&parser->body_pattern_buffer, (char *) pattern, 0))) {
    /*
     * If ptr returns something, it is an error string
     */
    n = regerror(n, &parser->body_pattern_buffer, (char *) tmp, sizeof(tmp));
    sprintf((char *) tmp + n, " in %s", pattern);
    display_error(216, (uchar *) tmp, FALSE);
    free(pattern);
    return RC_INVALID_OPERAND;
  }
  parser->have_body_pattern_buffer = TRUE;
  /*
   * Create the pattern buffer for functions...
   * If we said to in :option section
   */
  if (parser->function_option) {
    strcpy((char *) pattern, (char *) word[0]);
    strcat((char *) pattern, (char *) word[1]);
    strcat((char *) pattern, "*");
    if (parser->function_blank) {
      strcat((char *) pattern, " *");
    }
    if (num_params == 3) {
      strcat((char *) pattern, (char *) word[2]);
      strcat((char *) pattern, "?");
    }
    tmp[0] = parser->function_char;
    tmp[1] = '\0';
    strcat((char *) pattern, (char *) tmp);
    /* The following memset() is not meant to use wide character */
    memset(&parser->function_pattern_buffer, 0, sizeof(regex_t));
    if (0 != (n = regcomp(&parser->function_pattern_buffer, (char *) pattern, 0))) {
      /*
       * If ptr returns something, it is an error string
       */
      n = regerror(n, &parser->function_pattern_buffer, (char *) tmp, sizeof(tmp));
      sprintf((char *) tmp + n, " in %s", pattern);
      display_error(216, (uchar *) tmp, FALSE);
      free(pattern);
      return RC_INVALID_OPERAND;
    }
    free(pattern);
    parser->have_function_pattern_buffer = TRUE;
  }
  return rc;
}

#define CONDIR_MAX_EXT 10
#define CONDIR_PARAMS  CONDIR_MAX_EXT+3

static short construct_directory(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  PARSE_EXTENSION *curr;
  short rc = RC_OK;
  uchar *word[CONDIR_PARAMS + 1];
  uchar strip[CONDIR_PARAMS];
  unsigned short num_params, num_ext = 0;
  short option = 0, alt_word;
  int i;
  uchar alternate = 255;

  for (i = 0; i < CONDIR_PARAMS; i++) {
    strip[i] = STRIP_BOTH;
  }
  num_params = param_split(line, word, CONDIR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 1) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (equal((uchar *) "directory", word[0], 3)) {
    option = 1;
  } else if (equal((uchar *) "link", word[0], 4)) {
    option = 2;
  } else if (equal((uchar *) "executable", word[0], 3)) {
    option = 3;
  } else if (equal((uchar *) "extensions", word[0], 3)) {
    option = 4;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Now we know the keyword, work out its arguments.
   * "ALT x" is optional at the end
   */
  switch (option) {
    case 1:                    /* directory */
      if (num_params == 1) {
        alternate = ECOLOUR_DIRECTORY;
      } else if (num_params < 3) {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      } else if (num_params > 3) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      /* we have 3; last 2 must be alternate */
      alt_word = 1;
      break;
    case 2:                    /* link */
      if (num_params == 1) {
        alternate = ECOLOUR_LINK;
      } else if (num_params < 3) {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      } else if (num_params > 3) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      /* we have 3; last 2 must be alternate */
      alt_word = 1;
      break;
    case 3:                    /* executable */
      if (num_params == 1) {
        alternate = ECOLOUR_EXECUTABLE;
      } else if (num_params < 3) {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      } else if (num_params > 3) {
        sprintf((char *) tmp, "Too many tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      /* we have 3; last 2 must be alternate */
      alt_word = 1;
      break;
    case 4:                    /* extensions */
      if (num_params < 4) {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
      /*
       * second last word is where ALTernate starts
       */
      alt_word = num_params - 2;
      num_ext = num_params - 3;
      break;
  }
  /*
   * Check the optional alternate colour parameters if we haven't defaulted
   */
  if (alternate == 255) {
    if (equal((uchar *) "alternate", word[alt_word], 3)) {
      if (num_params > 2) {
        if (strlen((char *) word[alt_word + 1]) > 1) {
          sprintf((char *) tmp, "Too few tokens, line %d", lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (get_alternate(word[alt_word + 1], &alternate)) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[alt_word + 1], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
    }
  }
  /*
   * Once everything has been validated, set the flags
   */
  switch (option) {
    case 1:
      parser->have_directory_directory = TRUE;
      parser->directory_option_alternate = alternate;
      break;
    case 2:
      parser->have_directory_link = TRUE;
      parser->link_option_alternate = alternate;
      break;
    case 3:
      parser->have_executable = TRUE;
      parser->executable_option_alternate = alternate;
      break;
    case 4:
      for (i = 1; i < num_ext + 1; i++) {
        curr = parse_extensionll_add(parser->first_extension, parser->current_extension, sizeof(PARSE_EXTENSION));
        if (curr == NULL) {
          display_error(216, (uchar *) "out of memory", FALSE);
          return RC_OUT_OF_MEMORY;
        }
        parser->current_extension = curr;
        if (parser->first_extension == NULL) {
          parser->first_extension = parser->current_extension;
        }
        parser->current_extension->extension_length = strlen((char *) word[i]);
        parser->current_extension->extension = (uchar *) malloc((1 + parser->current_extension->extension_length) * sizeof(uchar));
        if (parser->current_extension->extension == NULL) {
          display_error(216, (uchar *) "out of memory", FALSE);
          return RC_OUT_OF_MEMORY;
        }
        strcpy((char *) parser->current_extension->extension, (char *) word[i]);
        parser->current_extension->alternate = alternate;
      }
      parser->have_extensions = TRUE;
      break;
  }
  return rc;
}

#define CONPOST_PARAMS  4

static short construct_postcompare(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  uchar *word[CONPOST_PARAMS + 1];
  uchar strip[CONPOST_PARAMS + 1];
  unsigned short num_params = 0;
  uchar *pattern = NULL; int n;
  PARSE_POSTCOMPARE *curr;
  uchar alternate = 255;
  regex_t pattern_buffer;
  bool is_class_type;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  num_params = param_split(line, word, CONPOST_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 2) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Check the optional alternate colour parameters first.
   */
  if (num_params > 2) {
    if (equal((uchar *) "alternate", word[2], 3)) {
      if (num_params > 3) {
        if (strlen((char *) word[3]) > 1) {
          sprintf((char *) tmp, "Too few tokens, line %d", lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (get_alternate(word[3], &alternate)) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[3], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
    }
  }
  /*
   * Now determine if the type is CLASS or STRING...
   */
  if (equal((uchar *) "class", word[0], 5)) {
    /*
     * CLASS type - word[1] is RE
     */
    pattern = (uchar *) malloc((20 + line_length) * sizeof(uchar));
    if (pattern == NULL) {
      display_error(216, (uchar *) "out of memory", FALSE);
      return RC_OUT_OF_MEMORY;
    }
    /*
     * Create the pattern buffer for the RE...
     */
    strcpy((char *) pattern, (char *) word[1]);
    /* The following memset() is not meant to use wide character */
    memset(&pattern_buffer, 0, sizeof(regex_t));
    if (0 != (n = regcomp(&pattern_buffer, (char *) pattern, 0))) {
      /*
       * If ptr returns something, it is an error string
       */
      n = regerror(n, &pattern_buffer, (char *) tmp, sizeof(tmp));
      sprintf((char *) tmp + n, " in %s", pattern);
      display_error(216, (uchar *) tmp, FALSE);
      free(pattern);
      return RC_INVALID_OPERAND;
    }
    free(pattern);
    is_class_type = TRUE;
  } else if (equal((uchar *) "text", word[0], 4)) {
    /*
     * TEXT type - word[1] is string
     */
    is_class_type = FALSE;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  curr = parse_postcomparell_add(parser->first_postcompare, parser->current_postcompare, sizeof(PARSE_POSTCOMPARE));
  if (curr == NULL) {
    display_error(216, (uchar *) "out of memory", FALSE);
    return RC_OUT_OF_MEMORY;
  }
  parser->current_postcompare = curr;
  if (parser->first_postcompare == NULL) {
    parser->first_postcompare = parser->current_postcompare;
  }
  parser->current_postcompare->string_length = 0;
  parser->current_postcompare->string = NULL;
  if (is_class_type) {
    parser->current_postcompare->pattern_buffer = pattern_buffer;
  } else {
    parser->current_postcompare->string_length = strlen((char *) word[1]);
    parser->current_postcompare->string = (uchar *) malloc((1 + parser->current_postcompare->string_length) * sizeof(uchar));
    if (parser->current_postcompare->string == NULL) {
      display_error(216, (uchar *) "out of memory", FALSE);
      return RC_OUT_OF_MEMORY;
    }
    strcpy((char *) parser->current_postcompare->string, (char *) word[1]);
  }
  parser->current_postcompare->alternate = alternate;
  parser->current_postcompare->is_class_type = is_class_type;
  parser->have_postcompare = TRUE;
  return (RC_OK);
}

#define CONNUM_PARAMS  1

static short construct_number(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  uchar *word[CONNUM_PARAMS + 1];
  uchar strip[CONNUM_PARAMS + 1];
  unsigned short num_params = 0;
  char *pattern;
  int n;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  num_params = param_split(line, word, CONNUM_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params == 0) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  if (num_params > 1) {
    sprintf((char *) tmp, "Too many tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Get number type
   */
  if (equal((uchar *) "rexx", word[0], 4)) {
    pattern = REXX_NUMBER_RE;
  } else if (equal((uchar *) "cobol", word[0], 5)) {
    pattern = COBOL_NUMBER_RE;
  } else if (equal((uchar *) "c", word[0], 1)) {
    pattern = C_NUMBER_RE;
  } else {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Create the pattern buffer for the RE...
   */
  memset(&parser->number_pattern_buffer, 0, sizeof(regex_t));
  if (0 != (n = regcomp(&parser->number_pattern_buffer, (char *) pattern, 0))) {
    /*
     * If ptr returns something, it is an error string
     */
    n = regerror(n, &parser->number_pattern_buffer, (char *) tmp, sizeof(tmp));
    sprintf((char *) tmp + n, " in %s", pattern);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  parser->have_number_pattern_buffer = TRUE;
  return (RC_OK);
}

#define CONCOL_PARAMS  5

static short construct_column(uchar *line, int line_length, PARSER_DETAILS *parser, int lineno) {
  uchar *word[CONCOL_PARAMS + 1];
  uchar strip[CONCOL_PARAMS + 1];
  unsigned short num_params = 0;
  uchar alternate = 255;
  long col1 = 0L, col2 = 0L;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  strip[3] = STRIP_BOTH;
  strip[4] = STRIP_BOTH;
  strip[5] = STRIP_BOTH;
  num_params = param_split(line, word, CONCOL_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  if (num_params < 3) {
    sprintf((char *) tmp, "Too few tokens, line %d", lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return RC_INVALID_OPERAND;
  }
  /*
   * Check the optional alternate colour parameters first.
   */
  if (num_params > 3) {
    if (equal((uchar *) "alternate", word[3], 3)) {
      if (num_params > 3) {
        if (strlen((char *) word[4]) > 1) {
          sprintf((char *) tmp, "Too few tokens, line %d", lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return RC_INVALID_OPERAND;
        }
        if (get_alternate(word[4], &alternate)) {
          sprintf((char *) tmp, "Invalid operand '%s', line %d", word[4], lineno);
          display_error(216, (uchar *) tmp, FALSE);
          return (RC_INVALID_OPERAND);
        }
      } else {
        sprintf((char *) tmp, "Too few tokens, line %d", lineno);
        display_error(216, (uchar *) tmp, FALSE);
        return RC_INVALID_OPERAND;
      }
    }
  }
  /*
   * 'EXCLUDE' is only valid option at this stage
   */
  if (!equal((uchar *) "exclude", word[0], 7)) {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[0], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  /*
   * Now obtain the start and end columns.
   */
  if (!valid_positive_integer(word[1])) {
    sprintf((char *) tmp, "Invalid operand '%s', line %d", word[1], lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  col1 = atol((char *) word[1]);
  /*
   * Second parameter must be positive integer or *
   */
  if (strcmp((char *) word[2], "*") == 0) {
    col2 = max_line_length;
  } else {
    if (!valid_positive_integer(word[2])) {
      sprintf((char *) tmp, "Invalid operand '%s', line %d", word[2], lineno);
      display_error(216, (uchar *) tmp, FALSE);
      return (RC_INVALID_OPERAND);
    }
    col2 = atol((char *) word[2]);
  }
  if (col2 > max_line_length) {
    col2 = max_line_length;
  }
  if (col1 > col2) {
    sprintf((char *) tmp, "First column(%ld) must be < last column(%ld), line %d", col1, col2, lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  if (parser->number_columns == MAX_PARSER_COLUMNS) {
    sprintf((char *) tmp, "Maximum number of :column options (%d) exceeded, line %d", MAX_PARSER_COLUMNS, lineno);
    display_error(216, (uchar *) tmp, FALSE);
    return (RC_INVALID_OPERAND);
  }
  parser->first_column[parser->number_columns] = col1;
  parser->last_column[parser->number_columns] = col2;
  parser->column_alternate[parser->number_columns] = alternate;
  parser->number_columns++;
  parser->have_columns = TRUE;
  return (RC_OK);
}

short construct_parser(uchar *contents, int contents_length, PARSER_DETAILS **parser, uchar *parser_name, uchar *filename) {
  int i, j, line_len;
  int state = 0;
  uchar line[250];
  short rc = RC_OK;
  int lineno = 0;

  last_parser = parserll_add(first_parser, last_parser, sizeof(PARSER_DETAILS));
  if (first_parser == NULL) {
    first_parser = last_parser;
  }
  strcpy((char *) last_parser->parser_name, (char *) parser_name);
  strcpy((char *) last_parser->filename, (char *) filename);
  last_parser->min_keyword_length = MAX_INT;
  last_parser->min_function_length = MAX_INT;
  *parser = last_parser;
  for (i = 0, j = 0; i < contents_length; i++) {
    if (contents[i] == '\n') {
      /*
       * We have a line, parse it
       */
      lineno++;
      line[j] = '\0';
      j = 0;
      strtrunc(line);
      line_len = strlen((char *) line);
      /*
       * Ignore blank lines and comments
       */
      if (line_len == 0 || line[0] == THE_CHAR_STAR) {
        continue;
      }
      if (line[0] == THE_CHAR_COLON) {
        state = STATE_FILE_HEADER;
      }
      switch (state) {
        case STATE_FILE_HEADER:
          if (memcmpi(line, (uchar *) ":case", line_len) == 0) {
            state = STATE_CASE;
            break;
          }
          if (memcmpi(line, (uchar *) ":option", line_len) == 0) {
            state = STATE_OPTION;
            break;
          }
          if (memcmpi(line, (uchar *) ":number", line_len) == 0) {
            state = STATE_NUMBER;
            break;
          }
          if (memcmpi(line, (uchar *) ":identifier", line_len) == 0) {
            state = STATE_IDENTIFIER;
            break;
          }
          if (memcmpi(line, (uchar *) ":string", line_len) == 0) {
            state = STATE_STRING;
            break;
          }
          if (memcmpi(line, (uchar *) ":comment", line_len) == 0) {
            state = STATE_COMMENT;
            break;
          }
          if (memcmpi(line, (uchar *) ":header", line_len) == 0) {
            state = STATE_HEADER;
            break;
          }
          if (memcmpi(line, (uchar *) ":label", line_len) == 0) {
            state = STATE_LABEL;
            break;
          }
          if (memcmpi(line, (uchar *) ":match", line_len) == 0) {
            state = STATE_MATCH;
            break;
          }
          if (memcmpi(line, (uchar *) ":keyword", line_len) == 0) {
            state = STATE_KEYWORD;
            break;
          }
          if (memcmpi(line, (uchar *) ":function", line_len) == 0) {
            state = STATE_FUNCTION;
            break;
          }
          if (memcmpi(line, (uchar *) ":postcompare", line_len) == 0) {
            state = STATE_POSTCOMPARE;
            break;
          }
          if (memcmpi(line, (uchar *) ":column", line_len) == 0) {
            state = STATE_COLUMN;
            break;
          }
          if (memcmpi(line, (uchar *) ":markup", line_len) == 0) {
            state = STATE_MARKUP;
            break;
          }
          if (memcmpi(line, (uchar *) ":directory", line_len) == 0) {
            state = STATE_DIRECTORY;
            break;
          }
          sprintf((char *) tmp, "Invalid header '%s', line %d", line, lineno);
          display_error(216, (uchar *) tmp, FALSE);
          rc = RC_INVALID_OPERAND;
          break;
        case STATE_CASE:
          rc = construct_case(line, line_len, last_parser, lineno);
          break;
        case STATE_OPTION:
          rc = construct_option(line, line_len, last_parser, lineno);
          break;
        case STATE_NUMBER:
          rc = construct_number(line, line_len, last_parser, lineno);
          break;
        case STATE_IDENTIFIER:
          rc = construct_identifier(line, line_len, last_parser, lineno);
          break;
        case STATE_STRING:
          rc = construct_string(line, line_len, last_parser, lineno);
          break;
        case STATE_COMMENT:
          rc = construct_comment(line, line_len, last_parser, lineno);
          break;
        case STATE_HEADER:
          rc = construct_header(line, line_len, last_parser, lineno);
          break;
        case STATE_LABEL:
          rc = construct_label(line, line_len, last_parser, lineno);
          break;
        case STATE_MATCH:
          rc = construct_match(line, line_len, last_parser, lineno);
          break;
        case STATE_KEYWORD:
          rc = construct_keyword(line, line_len, last_parser, lineno);
          break;
        case STATE_FUNCTION:
          rc = construct_function(line, line_len, last_parser, lineno);
          break;
        case STATE_POSTCOMPARE:
          rc = construct_postcompare(line, line_len, last_parser, lineno);
          break;
        case STATE_COLUMN:
          rc = construct_column(line, line_len, last_parser, lineno);
          break;
        case STATE_MARKUP:
          rc = construct_markup(line, line_len, last_parser, lineno);
          break;
        case STATE_DIRECTORY:
          rc = construct_directory(line, line_len, last_parser, lineno);
          break;
        default:
          break;
      }
      if (rc != RC_OK) {
        break;
      }
    } else {
      line[j++] = contents[i];
    }
  }
  return rc;
}

short destroy_parser(PARSER_DETAILS *parser) {
  short rc = RC_OK;

  if (parser->first_comments) {
    parser->first_comments = parse_commentsll_free(parser->first_comments);
  }
  if (parser->first_keyword) {
    parser->first_keyword = parse_keywordll_free(parser->first_keyword);
  }
  if (parser->first_function) {
    parser->first_function = parse_functionll_free(parser->first_function);
  }
  if (parser->first_header) {
    parser->first_header = parse_headerll_free(parser->first_header);
  }
  if (parser->have_body_pattern_buffer) {
    regfree(&parser->body_pattern_buffer);
  }
  if (parser->have_function_pattern_buffer) {
    regfree(&parser->function_pattern_buffer);
  }
  if (parser->have_number_pattern_buffer) {
    regfree(&parser->number_pattern_buffer);
  }
  if (parser->have_postcompare) {
    parser->first_postcompare = parse_postcomparell_free(parser->first_postcompare);
  }
  if (parser->have_extensions) {
    parser->first_extension = parse_extensionll_free(parser->first_extension);
  }
  return rc;
}

bool find_parser_mapping(FILE_DETAILS *fd, PARSER_MAPPING *curr_mapping) {
  if (curr_mapping->filemask) {
    if (fnmatch((char *) curr_mapping->filemask, (char *) fd->fname, 0) == 0) {
      return TRUE;
    }
  } else {
    /*
     * We now want to try "magic numbers" of shell scripts
     * If the file has at least one line, and that line starts with #!, get the last string from the first word.
     * This should be the interpreter to use. eg #!/usr/bin/csh
     */
    if (fd && fd->first_line && fd->first_line->next) {
      LINE *curr = fd->first_line->next;
      uchar *ptr = curr->line;
      int i, state = STATE_START, start = 0, len;

      if (curr->length > 3 && *(curr->line) == THE_CHAR_HASH && *(curr->line + 1) == THE_CHAR_BANG) {
        for (i = 2; i < curr->length + 1; i++) {
          switch (state) {
            case STATE_START:
              if (*(ptr + i) == THE_CHAR_SLASH) {
                state = STATE_SLASH;
                start = i;
                break;
              }
              break;
            case STATE_SLASH:
              if (*(ptr + i) == THE_CHAR_SLASH) {
                start = i;
                break;
              }
              if (*(ptr + i) == THE_CHAR_SPACE) {
                len = i - start - 1;
                /*
                 * first check if the result is "env".
                 * In this case the magic number is likely #!/usr/bin/env mapping
                 * so we want to keep reading after the space to get the mapping
                 */
                if (memcmp(ptr + start + 1, "env", len) == 0) {
                  state = STATE_ENV;
                  start = i;
                }
              } else if (*(ptr + i) == '\0') {
                len = i - start - 1;
                if (len == curr_mapping->magic_number_length && memcmp(ptr + start + 1, curr_mapping->magic_number, len) == 0) {
                  return TRUE;
                  break;
                }
              }
              break;
            case STATE_ENV:
              if (*(ptr + i) == THE_CHAR_SPACE) {
                start = i;
                break;
              }
              if (*(ptr + i) == '\0') {
                len = i - start - 1;
                if (len == curr_mapping->magic_number_length && memcmp(ptr + start + 1, curr_mapping->magic_number, len) == 0) {
                  return TRUE;
                  break;
                }
              }
              break;
          }
        }
      }
    }
  }
  return FALSE;
}

PARSER_DETAILS *find_auto_parser(FILE_DETAILS *fd) {
  PARSER_MAPPING *curr_mapping = first_parser_mapping;

  /*
   * Check the filename against the available parsers' filemasks.
   * If the filemask uses filenames then check the filename.
   * If the filemask uses "magic" check the magic number of the first line of the file.
   */
  for (; curr_mapping != NULL; curr_mapping = curr_mapping->next) {
    if (find_parser_mapping(fd, curr_mapping)) {
      fd->parser = curr_mapping->parser;
      break;
    }
  }
  return (fd->parser);
}

short parse_reserved_line(RESERVED *rsrvd) {
  int i = 0, j = 0, k = 0;
  chtype current_colour, default_colour;
  int state = STATE_START;
  uchar *line = rsrvd->line;
  uchar *disp = rsrvd->disp;

  current_colour = default_colour = set_colour(rsrvd->attr);
  if (CTLCHARx) {
    for (;;) {
      switch (state) {
        case STATE_START:
          if (line[i] == ctlchar_escape) {
            /*
             * The character in the reserved line is the escape character
             */
            state = STATE_DELIMITER;
            i++;
            break;
          }
          rsrvd->highlighting[k] = current_colour;
          disp[k] = line[i];
          i++;
          k++;
          break;
        case STATE_DELIMITER:
          /*
           * Find what attribute character follows the escape character...
           */
          for (j = 0; j < MAX_CTLCHARS; j++) {
            if (ctlchar_char[j] == line[i]) {
              /*
               * ...and it is the attribute character for OFF, set the colour to the default colour for reserved lines
               */
              if (ctlchar_attr[j].pair == -1) {
                current_colour = default_colour;
              } else {
                /*
                 * ... and set the colour associated with the attribute character
                 */
                current_colour = set_colour(&ctlchar_attr[j]);
              }
              state = STATE_START;
              i++;
              break;
            }
          }
          rsrvd->highlighting[k] = current_colour;
          disp[k] = line[i];
          i++;
          k++;
          break;
      }
      if (i > rsrvd->length) {
        break;
      }
    }
    rsrvd->disp_length = k - 1;
  } else {
    memcpy(rsrvd->disp, rsrvd->line, rsrvd->length);
    for (i = 0; i < rsrvd->length; i++) {
      rsrvd->highlighting[i] = current_colour;
    }
    rsrvd->disp_length = rsrvd->length;
  }
  return (RC_OK);
}

uchar get_syntax_element(uchar scrno, int row, int col) {
  SHOW_LINE *scurr;
  uchar syntax_element;
  long vcol = SCREEN_VIEW(scrno)->verify_col - 1;

  scurr = screen[scrno].sl;
  if (row > screen[scrno].rows[WINDOW_FILEAREA] || col > screen[scrno].cols[WINDOW_FILEAREA]) {
    syntax_element = THE_SYNTAX_UNKNOWN;
  } else {
    scurr += row;
    if (col + vcol >= scurr->length) {
      syntax_element = THE_SYNTAX_UNKNOWN;
    } else {
      syntax_element = scurr->highlight_type[col + vcol];
    }
  }
  return (syntax_element);
}

