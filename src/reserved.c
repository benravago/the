// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* This file contains funtions related to reserved lines.              */


#include "the.h"
#include "proto.h"

RESERVED *add_reserved_line(char_t *spec, char_t *line, short base, short off, COLOUR_ATTR *attr, bool autoscroll) {
  RESERVED *curr = NULL;
  char_t *templine = line;

  /*
   * First check if the row already has a reserved line on it...
   */
  if ((curr = find_reserved_line(current_screen, FALSE, 0, base, off)) != NULL) {
    delete_reserved_line(base, off);
  }
  curr = rll_add(CURRENT_FILE->first_reserved, CURRENT_FILE->first_reserved, sizeof(RESERVED));
  if (CURRENT_FILE->first_reserved == NULL) {
    CURRENT_FILE->first_reserved = curr;
  }
  if (templine == NULL) {
    templine = (char_t *) "";
  }
  if ((curr->line = (char_t *) malloc ((strlen((char *) templine) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (NULL);
  }
  if ((curr->disp = (char_t *) malloc ((strlen((char *) templine) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (NULL);
  }
  if ((curr->highlighting = (chtype *) malloc ((strlen((char *) templine) + 1) * sizeof(chtype))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (NULL);
  }
  if ((curr->spec = (char_t *) malloc ((strlen((char *) spec) + 1) * sizeof(char_t))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (NULL);
  }
  if ((curr->attr = (COLOUR_ATTR *) malloc (sizeof(COLOUR_ATTR))) == NULL) {
    display_error(30, (char_t *) "", FALSE);
    return (NULL);
  }
  strcpy((char *) curr->line, (char *) templine);
  strcpy((char *) curr->spec, (char *) spec);
  curr->length = strlen((char *) templine);
  curr->base = base;
  curr->off = off;
  curr->autoscroll = autoscroll;
  memcpy(curr->attr, attr, sizeof(COLOUR_ATTR));
  parse_reserved_line(curr);
  return (curr);
}

RESERVED *find_reserved_line(char_t scrno, bool find_by_row, row_t row, short base, short off) {
  RESERVED *curr = SCREEN_FILE(scrno)->first_reserved;

  while (curr != NULL) {
    if (find_by_row) {
      if (curr->base == POSITION_TOP && row == curr->off - 1) {
        break;
      }
      if (curr->base == POSITION_BOTTOM && row == (curr->off + screen[scrno].rows[WINDOW_FILEAREA])) {
        break;
      }
      if (curr->base == POSITION_MIDDLE && row == (curr->off + (screen[scrno].rows[WINDOW_FILEAREA] / 2)) - 1) {
        break;
      }
    } else {
      if (curr->base == base && curr->off == off) {
        break;
      }
    }
    curr = curr->next;
  }
  return (curr);
}

short delete_reserved_line(short base, short off) {
  RESERVED *curr = NULL;

  if ((curr = find_reserved_line(current_screen, FALSE, 0, base, off)) == NULL) {
    display_error(64, (char_t *) "", FALSE);
    return (RC_NO_LINES_CHANGED);
  }
  if (curr->line != NULL) {
    free (curr->line);
  }
  if (curr->disp != NULL) {
    free (curr->disp);
  }
  if (curr->highlighting != NULL) {
    free (curr->highlighting);
  }
  if (curr->spec != NULL) {
    free (curr->spec);
  }
  if (curr->attr != NULL) {
    free (curr->attr);
  }
  rll_del(&CURRENT_FILE->first_reserved, NULL, curr, DIRECTION_FORWARD);
  return (RC_OK);
}

