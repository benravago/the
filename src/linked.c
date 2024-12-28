// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

THELIST *ll_add(THELIST *first, THELIST *curr, unsigned short size) {
  THELIST *next = NULL;

  if ((next = (THELIST *) malloc(size)) != (THELIST *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      if (first == NULL) {
        /*
         * Only entry in list
         */
        next->next = NULL;
        next->prev = NULL;
      } else {
        /*
         * Insert this entry before first. Calling routine
         * must reset first to returned pointer.
         */
        next->next = first;
        next->prev = NULL;
        first->prev = next;
      }
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return next;
}

THELIST *ll_del(THELIST **first, THELIST **last, THELIST *curr, short direction, THELIST_DEL delfunc) {
  THELIST *new_curr = NULL;

  if (delfunc) {
    (*delfunc) (curr->data);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return NULL;
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return curr;
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return curr;
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return curr;
}

THELIST *ll_free(THELIST *first, THELIST_DEL delfunc) {
  THELIST *curr = NULL;
  THELIST *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (delfunc) {
      (*delfunc) (curr->data);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return (THELIST *) NULL;
}

LINE *lll_add(LINE *first, LINE *curr, unsigned short size) {
  LINE *next = NULL;

  if ((next = (LINE *) malloc(size)) != (LINE *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      if (first == NULL) {
        /*
         * First entry in LL
         */
        /*          first = next; */
        next->next = NULL;
        next->prev = NULL;
      } else {
        /*
         * Insert this entry before first. Calling routine
         * must reset first to returned pointer.
         */
        next->next = first;
        next->prev = NULL;
        first->prev = next;
      }
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
      next->prev = curr;
    }
  }
  return (next);
}

LINE *lll_del(LINE **first, LINE **last, LINE *curr, short direction) {
  LINE *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

LINE *lll_free(LINE *first) {
  LINE *curr = NULL;
  LINE *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->line) {
      free(curr->line);
    }
    if (curr->name) {
      free(curr->name);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((LINE *) NULL);
}

LINE *lll_find(LINE *first, LINE *last, long line_number, long max_lines) {
  LINE *curr = NULL;
  long i = 0L;

  if (line_number < (max_lines / 2)) {
    curr = first;
    if (curr != NULL) {
      for (i = 0L; i < line_number; i++, curr = curr->next);    /* FGC - removed check for NULL */
    }
  } else {
    curr = last;
    if (curr != NULL) {
      for (i = max_lines + 1L; i > line_number; i--, curr = curr->prev);        /* FGC - removed check for NULL */
    }
  }
  return (curr);
}

LINE *lll_locate(LINE *first, uchar *value) {
  LINE *curr = NULL;

  curr = first;
  while (curr) {
    if (curr->name && strcmp((char *) curr->name, (char *) value) == 0) {
      break;
    }
    curr = curr->next;
  }
  return (curr);
}

VIEW_DETAILS *vll_add(VIEW_DETAILS *first, VIEW_DETAILS *curr, unsigned short size) {
  VIEW_DETAILS *next = (VIEW_DETAILS *) NULL;

  if ((next = (VIEW_DETAILS *) malloc(size)) != (VIEW_DETAILS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == (VIEW_DETAILS *) NULL) {
      first = next;
      next->next = (VIEW_DETAILS *) NULL;
    } else {
      if (curr->next != (VIEW_DETAILS *) NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

/*
 * This ll_del() function is different to others!
 */
VIEW_DETAILS *vll_del(VIEW_DETAILS **first, VIEW_DETAILS **last, VIEW_DETAILS *curr, short direction) {
  VIEW_DETAILS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   * If DIRECTION_FORWARD, curr becomes first, otherwise curr becomes prev
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    if (direction == DIRECTION_FORWARD) {
      new_curr = *first;
    } else {
      new_curr = curr->prev;
    }
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

DEFINE *dll_add(DEFINE *first, DEFINE *curr, unsigned short size) {
  DEFINE *next = NULL;

  if ((next = (DEFINE *) malloc(size)) != (DEFINE *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

DEFINE *dll_del(DEFINE **first, DEFINE **last, DEFINE *curr, short direction) {
  DEFINE *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

DEFINE *dll_free(DEFINE *first) {
  DEFINE *curr = NULL;
  DEFINE *new_curr = NULL;

  curr = first;
  while (curr != (DEFINE *) NULL) {
    if (curr->def_params != NULL) {
      free(curr->def_params);
    }
    if (curr->pcode != NULL) {
      free(curr->pcode);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((DEFINE *) NULL);
}

THE_PPC *pll_add(THE_PPC **first, unsigned short size, long line_number) {
  THE_PPC *next = NULL, *curr, *prev = NULL;

  if ((next = (THE_PPC *) malloc(size)) != (THE_PPC *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    /*
     * No other PPC exist
     */
    if (*first == NULL) {
      next->next = next->prev = NULL;
      *first = next;
    } else {
      curr = *first;
      while (curr) {
        if (curr->ppc_line_number < line_number && prev == NULL) {
          prev = curr;
        } else if (curr->ppc_line_number < line_number && curr->ppc_line_number > prev->ppc_line_number) {
          prev = curr;
        }
        curr = curr->next;
      }
      if (prev == NULL) {
        /*
         * Insert before first member
         */
        next->prev = NULL;
        next->next = *first;
        curr = *first;
        curr->prev = next;
        *first = next;
      } else {
        /*
         * Insert after prev
         */
        if (prev->next) {
          prev->next->prev = next;
        }
        next->next = prev->next;
        next->prev = prev;
        prev->next = next;
      }
    }
  }
  return (next);
}

THE_PPC *pll_del(THE_PPC **first, THE_PPC **last, THE_PPC *curr, short direction) {
  THE_PPC *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

THE_PPC *pll_free(THE_PPC *first) {
  THE_PPC *curr = NULL;
  THE_PPC *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((THE_PPC *) NULL);
}

THE_PPC *pll_find(THE_PPC *first, long line_number) {
  THE_PPC *curr_ppc = NULL;

  curr_ppc = first;
  while (curr_ppc != NULL) {
    if (curr_ppc->ppc_line_number == line_number) {
      return (curr_ppc);
    }
    curr_ppc = curr_ppc->next;
  }
  return (NULL);
}

RESERVED *rll_add(RESERVED *first, RESERVED *curr, unsigned short size) {
  RESERVED *next = NULL;

  if ((next = (RESERVED *) malloc(size)) != (RESERVED *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

RESERVED *rll_del(RESERVED **first, RESERVED **last, RESERVED *curr, short direction) {
  RESERVED *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

RESERVED *rll_free(RESERVED *first) {
  RESERVED *curr = NULL;
  RESERVED *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->line != (uchar *) NULL) {
      free(curr->line);
    }
    if (curr->spec != (uchar *) NULL) {
      free(curr->spec);
    }
    if (curr->attr != (COLOUR_ATTR *) NULL) {
      free(curr->attr);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((RESERVED *) NULL);
}

RESERVED *rll_find(RESERVED *first, short row) {
  RESERVED *curr = NULL;
  short i = 0;

  curr = first;
  if (curr != NULL) {
    for (i = 0; i < row && curr->next != NULL; i++, curr = curr->next);
  }
  return (curr);
}

PARSER_DETAILS *parserll_add(PARSER_DETAILS *first, PARSER_DETAILS *curr, unsigned short size) {
  PARSER_DETAILS *next = NULL;

  if ((next = (PARSER_DETAILS *) malloc(size)) != (PARSER_DETAILS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSER_DETAILS *parserll_del(PARSER_DETAILS **first, PARSER_DETAILS **last, PARSER_DETAILS *curr, short direction) {
  PARSER_DETAILS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSER_DETAILS *parserll_free(PARSER_DETAILS *first) {
  PARSER_DETAILS *curr = NULL;
  PARSER_DETAILS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->first_comments != NULL) {
      free(curr->first_comments);
    }
    if (curr->first_keyword != NULL) {
      free(curr->first_keyword);
    }
    if (curr->first_function != NULL) {
      free(curr->first_function);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSER_DETAILS *) NULL);
}

PARSER_DETAILS *parserll_find(PARSER_DETAILS *first, uchar *name) {
  PARSER_DETAILS *curr = NULL;

  curr = first;
  if (curr != NULL) {
    for (; curr != NULL; curr = curr->next) {
      if (strcasecmp((char *) name, (char *) curr->parser_name) == 0) {
        return curr;
      }
    }
  }
  return (NULL);
}

PARSE_KEYWORDS *parse_keywordll_add(PARSE_KEYWORDS *first, PARSE_KEYWORDS *curr, unsigned short size) {
  PARSE_KEYWORDS *next = NULL;

  if ((next = (PARSE_KEYWORDS *) malloc(size)) != (PARSE_KEYWORDS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_KEYWORDS *parse_keywordll_del(PARSE_KEYWORDS **first, PARSE_KEYWORDS **last, PARSE_KEYWORDS *curr, short direction) {
  PARSE_KEYWORDS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSE_KEYWORDS *parse_keywordll_free(PARSE_KEYWORDS *first) {
  PARSE_KEYWORDS *curr = NULL;
  PARSE_KEYWORDS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->keyword != NULL) {
      free(curr->keyword);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_KEYWORDS *) NULL);
}

PARSE_FUNCTIONS *parse_functionll_add(PARSE_FUNCTIONS *first, PARSE_FUNCTIONS *curr, unsigned short size) {
  PARSE_FUNCTIONS *next = NULL;

  if ((next = (PARSE_FUNCTIONS *) malloc(size)) != (PARSE_FUNCTIONS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_FUNCTIONS *parse_functionll_del(PARSE_FUNCTIONS **first, PARSE_FUNCTIONS **last, PARSE_FUNCTIONS *curr, short direction) {
  PARSE_FUNCTIONS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSE_FUNCTIONS *parse_functionll_free(PARSE_FUNCTIONS *first) {
  PARSE_FUNCTIONS *curr = NULL;
  PARSE_FUNCTIONS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->function != NULL) {
      free(curr->function);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_FUNCTIONS *) NULL);
}

PARSE_HEADERS *parse_headerll_add(PARSE_HEADERS *first, PARSE_HEADERS *curr, unsigned short size) {
  PARSE_HEADERS *next = NULL;

  if ((next = (PARSE_HEADERS *) malloc(size)) != (PARSE_HEADERS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_HEADERS *parse_headerll_free(PARSE_HEADERS *first) {
  PARSE_HEADERS *curr = NULL;
  PARSE_HEADERS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_HEADERS *) NULL);
}

PARSER_MAPPING *mappingll_add(PARSER_MAPPING *first, PARSER_MAPPING *curr, unsigned short size) {
  PARSER_MAPPING *next = NULL;

  if ((next = (PARSER_MAPPING *) malloc(size)) != (PARSER_MAPPING *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSER_MAPPING *mappingll_del(PARSER_MAPPING **first, PARSER_MAPPING **last, PARSER_MAPPING *curr, short direction) {
  PARSER_MAPPING *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSER_MAPPING *mappingll_free(PARSER_MAPPING *first) {
  PARSER_MAPPING *curr = NULL;
  PARSER_MAPPING *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->parser_name != NULL) {
      free(curr->parser_name);
    }
    if (curr->filemask != NULL) {
      free(curr->filemask);
    }
    if (curr->magic_number != NULL) {
      free(curr->magic_number);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSER_MAPPING *) NULL);
}

PARSER_MAPPING *mappingll_find(PARSER_MAPPING *first, uchar *filemask, uchar *magic_number) {
  PARSER_MAPPING *curr = NULL;

  curr = first;
  if (curr != NULL) {
    for (; curr != NULL; curr = curr->next) {
      if (filemask != NULL) {
        if (curr->filemask && strcmp((char *) filemask, (char *) curr->filemask) == 0) {
          return curr;
        }
      } else {
        if (curr->magic_number && strcmp((char *) magic_number, (char *) curr->magic_number) == 0) {
          return curr;
        }
      }
    }
  }
  return (NULL);
}

PARSE_COMMENTS *parse_commentsll_add(PARSE_COMMENTS *first, PARSE_COMMENTS *curr, unsigned short size) {
  PARSE_COMMENTS *next = NULL;

  if ((next = (PARSE_COMMENTS *) malloc(size)) != (PARSE_COMMENTS *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_COMMENTS *parse_commentsll_del(PARSE_COMMENTS **first, PARSE_COMMENTS **last, PARSE_COMMENTS *curr, short direction) {
  PARSE_COMMENTS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSE_COMMENTS *parse_commentsll_free(PARSE_COMMENTS *first) {
  PARSE_COMMENTS *curr = NULL;
  PARSE_COMMENTS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_COMMENTS *) NULL);
}

PARSE_POSTCOMPARE *parse_postcomparell_add(PARSE_POSTCOMPARE *first, PARSE_POSTCOMPARE *curr, unsigned short size) {
  PARSE_POSTCOMPARE *next = NULL;

  if ((next = (PARSE_POSTCOMPARE *) malloc(size)) != (PARSE_POSTCOMPARE *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_POSTCOMPARE *parse_postcomparell_del(PARSE_POSTCOMPARE **first, PARSE_POSTCOMPARE **last, PARSE_POSTCOMPARE *curr, short direction) {
  PARSE_POSTCOMPARE *new_curr = NULL;

  if (curr->string) {
    free(curr->string);
  }
  if (curr->is_class_type) {
    regfree(&curr->pattern_buffer);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSE_POSTCOMPARE *parse_postcomparell_free(PARSE_POSTCOMPARE *first) {
  PARSE_POSTCOMPARE *curr = NULL;
  PARSE_POSTCOMPARE *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->string) {
      free(curr->string);
    }
    if (curr->is_class_type) {
      regfree(&curr->pattern_buffer);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_POSTCOMPARE *) NULL);
}

PARSE_EXTENSION *parse_extensionll_add(PARSE_EXTENSION *first, PARSE_EXTENSION *curr, unsigned short size) {
  PARSE_EXTENSION *next = NULL;

  if ((next = (PARSE_EXTENSION *) malloc(size)) != (PARSE_EXTENSION *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      first = next;
      next->next = NULL;
    } else {
      if (curr->next != NULL) {
        curr->next->prev = next;
      }
      next->next = curr->next;
      curr->next = next;
    }
    next->prev = curr;
  }
  return (next);
}

PARSE_EXTENSION *parse_extensionll_del(PARSE_EXTENSION **first, PARSE_EXTENSION **last, PARSE_EXTENSION *curr, short direction) {
  PARSE_EXTENSION *new_curr = NULL;

  if (curr->extension) {
    free(curr->extension);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free(curr);
    *first = NULL;
    if (last != NULL) {
      *last = NULL;
    }
    return (NULL);
  }
  /*
   * Delete the first record
   */
  if (curr->prev == NULL) {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last  record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free(curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * All others
   */
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  if (direction == DIRECTION_FORWARD) {
    new_curr = curr->next;
  } else {
    new_curr = curr->prev;
  }

  free(curr);
  curr = new_curr;
  return (curr);
}

PARSE_EXTENSION *parse_extensionll_free(PARSE_EXTENSION *first) {
  PARSE_EXTENSION *curr = NULL;
  PARSE_EXTENSION *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->extension) {
      free(curr->extension);
    }
    new_curr = curr->next;
    free(curr);
    curr = new_curr;
  }
  return ((PARSE_EXTENSION *) NULL);
}

