// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* This file contains funtions related to reserved lines.              */

#include "the.h"
#include "proto.h"

RESERVED *add_reserved_line(CHARTYPE * spec, CHARTYPE * line, short base, short off, COLOUR_ATTR * attr, bool autoscroll) {
  RESERVED *curr = NULL;
  CHARTYPE *templine = line;

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
    templine = (CHARTYPE *) "";
  }
  if ((curr->line = (CHARTYPE *) malloc((strlen((DEFCHAR *) templine) + 1) * sizeof(CHARTYPE))) == NULL) {
    display_error(30, (CHARTYPE *) "", FALSE);
    return (NULL);
  }
  if ((curr->disp = (CHARTYPE *) malloc((strlen((DEFCHAR *) templine) + 1) * sizeof(CHARTYPE))) == NULL) {
    display_error(30, (CHARTYPE *) "", FALSE);
    return (NULL);
  }
  if ((curr->highlighting = (chtype *) malloc((strlen((DEFCHAR *) templine) + 1) * sizeof(chtype))) == NULL) {
    display_error(30, (CHARTYPE *) "", FALSE);
    return (NULL);
  }
  if ((curr->spec = (CHARTYPE *) malloc((strlen((DEFCHAR *) spec) + 1) * sizeof(CHARTYPE))) == NULL) {
    display_error(30, (CHARTYPE *) "", FALSE);
    return (NULL);
  }
  if ((curr->attr = (COLOUR_ATTR *) malloc(sizeof(COLOUR_ATTR))) == NULL) {
    display_error(30, (CHARTYPE *) "", FALSE);
    return (NULL);
  }
  strcpy((DEFCHAR *) curr->line, (DEFCHAR *) templine);
  strcpy((DEFCHAR *) curr->spec, (DEFCHAR *) spec);
  curr->length = strlen((DEFCHAR *) templine);
  curr->base = base;
  curr->off = off;
  curr->autoscroll = autoscroll;
  memcpy(curr->attr, attr, sizeof(COLOUR_ATTR));
  parse_reserved_line(curr);
  return (curr);
}

RESERVED *find_reserved_line(CHARTYPE scrno, bool find_by_row, ROWTYPE row, short base, short off) {
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
    display_error(64, (CHARTYPE *) "", FALSE);
    return (RC_NO_LINES_CHANGED);
  }
  if (curr->line != NULL) {
    free(curr->line);
  }
  if (curr->disp != NULL) {
    free(curr->disp);
  }
  if (curr->highlighting != NULL) {
    free(curr->highlighting);
  }
  if (curr->spec != NULL) {
    free(curr->spec);
  }
  if (curr->attr != NULL) {
    free(curr->attr);
  }
  rll_del(&CURRENT_FILE->first_reserved, NULL, curr, DIRECTION_FORWARD);
  return (RC_OK);
}

