// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* Linked list routines.                                    */

#include "the.h"
#include "proto.h"

/*
 * Adds a THELIST to the current linked list after the current member.
 * PARAMETERS:
 * first      - pointer to first THELIST in linked list
 * curr       - pointer to current THELIST in linked list
 * size       - size of a THELIST item
 * RETURN:    - pointer to next THELIST item
 */
THELIST *ll_add(THELIST *first, THELIST *curr, unsigned short size) {
  THELIST *next = NULL;

  if ((next = (THELIST *) malloc (size)) != (THELIST *) NULL) {
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

/*
 * Deletes a THELIST from the current linked list.
 * PARAMETERS:
 * first      - pointer to pointer to first THELIST in linked list
 * last       - pointer to pointer to last THELIST in linked list
 * curr       - pointer to current THELIST in linked list
 * direction  - whether to return the next or previous pointer after the current
 *              item is deleted
 * delfunc    - pointer to a THELIST_DEL function to delete the item data
 * RETURN:    - pointer to next or previous THELIST item
 */
THELIST *ll_del(THELIST **first, THELIST **last, THELIST *curr, short direction, THELIST_DEL delfunc) {
  THELIST *new_curr = NULL;

  if (delfunc) {
    (*delfunc) (curr->data);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return curr;
}

/*
 * Frees all THELIST items from a linked list.
 * PARAMETERS:
 * first      - pointer to first THELIST in linked list
 * delfunc    - pointer to a THELIST_DEL function to delete the item data
 * RETURN:    - NULL
 */
THELIST *ll_free(THELIST *first, THELIST_DEL delfunc) {
  THELIST *curr = NULL;
  THELIST *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (delfunc) {
      (*delfunc) (curr->data);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return (THELIST *) NULL;
}

/* Adds a LINE to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first LINE in linked list                   */
/* curr       - pointer to current LINE in linked list                 */
/* size       - size of a LINE item                                    */
/* RETURN:    - pointer to next LINE item                              */

LINE *lll_add(LINE *first, LINE *curr, unsigned short size) {
  LINE *next = NULL;

  if ((next = (LINE *) malloc (size)) != (LINE *) NULL) {
    /*
     * Ensure all pointers in the structure are set to NULL
     */
    memset(next, 0, size);
    if (curr == NULL) {
      if (first == NULL) {
        /*
         * First entry in LL
         */
        //  first = next;
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

LINE *lll_free(LINE *first) {
  LINE *curr = NULL;
  LINE *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->line)
      free (curr->line);
    if (curr->name)
      free (curr->name);
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((LINE *) NULL);
}

LINE *lll_find(LINE *first, LINE *last, line_t line_number, line_t max_lines) {
  LINE *curr = NULL;
  line_t i = 0L;

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

LINE *lll_locate(LINE *first, char_t *value) {
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

/* Adds a VIEW_DETAILS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first VIEW_DETAILS in linked list                   */
/* curr       - pointer to current VIEW_DETAILS in linked list                 */
/* size       - size of a VIEW_DETAILS item                                    */
/* RETURN:    - pointer to next VIEW_DETAILS item                              */

VIEW_DETAILS *vll_add(VIEW_DETAILS *first, VIEW_DETAILS *curr, unsigned short size) {
  VIEW_DETAILS *next = (VIEW_DETAILS *) NULL;

  if ((next = (VIEW_DETAILS *) malloc (size)) != (VIEW_DETAILS *) NULL) {
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

/* This ll_del() function is different to others!!!!!!!!               */

VIEW_DETAILS *vll_del(VIEW_DETAILS **first, VIEW_DETAILS **last, VIEW_DETAILS *curr, short direction) {
  VIEW_DETAILS *new_curr = NULL;

  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Adds a DEFINE to the current linked list after the current member.  */
/* PARAMETERS:                                                         */
/* first      - pointer to first DEFINE in linked list                 */
/* curr       - pointer to current DEFINE in linked list               */
/* size       - size of a DEFINE item                                  */
/* RETURN:    - pointer to next DEFINE item                            */

DEFINE *dll_add(DEFINE *first, DEFINE *curr, unsigned short size) {
  DEFINE *next = NULL;

  if ((next = (DEFINE *) malloc (size)) != (DEFINE *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

DEFINE *dll_free(DEFINE *first) {
  DEFINE *curr = NULL;
  DEFINE *new_curr = NULL;

  curr = first;
  while (curr != (DEFINE *) NULL) {
    if (curr->def_params != NULL) {
      free (curr->def_params);
    }
    if (curr->pcode != NULL) {
      free (curr->pcode);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((DEFINE *) NULL);
}

/* Adds a THE_PPC to the current linked list in line_number order.     */
/* PARAMETERS:                                                         */
/* first      - pointer to first THE_PPC in linked list                */
/* size       - size of a THE_PPC item                                 */
/* line_number- line number for this THE_PPC                           */
/* RETURN:    - pointer to new THE_PPC item                            */

THE_PPC *pll_add(THE_PPC **first, unsigned short size, line_t line_number) {
  THE_PPC *next = NULL, *curr, *prev = NULL;

  if ((next = (THE_PPC *) malloc (size)) != (THE_PPC *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

THE_PPC *pll_free(THE_PPC *first) {
  THE_PPC *curr = NULL;
  THE_PPC *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((THE_PPC *) NULL);
}

THE_PPC *pll_find(THE_PPC *first, line_t line_number) {
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

/* Adds a RESERVED to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first RESERVED in linked list                   */
/* curr       - pointer to current RESERVED in linked list                 */
/* size       - size of a RESERVED item                                    */
/* RETURN:    - pointer to next RESERVED item                              */

RESERVED *rll_add(RESERVED *first, RESERVED *curr, unsigned short size) {
  RESERVED *next = NULL;

  if ((next = (RESERVED *) malloc (size)) != (RESERVED *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

RESERVED *rll_free(RESERVED *first) {
  RESERVED *curr = NULL;
  RESERVED *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->line != (char_t *) NULL) {
      free (curr->line);
    }
    if (curr->spec != (char_t *) NULL) {
      free (curr->spec);
    }
    if (curr->attr != (COLOUR_ATTR *) NULL) {
      free (curr->attr);
    }
    new_curr = curr->next;
    free (curr);
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


/* Adds a PARSER_DETAILS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSER_DETAILS in linked list         */
/* curr       - pointer to current PARSER_DETAILS in linked list       */
/* size       - size of a PARSER_DETAILS item                          */
/* RETURN:    - pointer to next PARSER_DETAILS item                    */

PARSER_DETAILS *parserll_add(PARSER_DETAILS *first, PARSER_DETAILS *curr, unsigned short size) {
  PARSER_DETAILS *next = NULL;

  if ((next = (PARSER_DETAILS *) malloc (size)) != (PARSER_DETAILS *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSER_DETAILS *parserll_free(PARSER_DETAILS *first) {
  PARSER_DETAILS *curr = NULL;
  PARSER_DETAILS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->first_comments != NULL) {
      free (curr->first_comments);
    }
    if (curr->first_keyword != NULL) {
      free (curr->first_keyword);
    }
    if (curr->first_function != NULL) {
      free (curr->first_function);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSER_DETAILS *) NULL);
}

PARSER_DETAILS *parserll_find(PARSER_DETAILS *first, char_t *name) {
  PARSER_DETAILS *curr = NULL;
  short i = 0;

  curr = first;
  if (curr != NULL) {
    for (i = 0; curr != NULL; i++, curr = curr->next) {
      if (strcasecmp((char *) name, (char *) curr->parser_name) == 0) {
        return curr;
      }
    }
  }
  return (NULL);
}

/* Adds a PARSE_KEYWORDS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_KEYWORDS in linked list         */
/* curr       - pointer to current PARSE_KEYWORDS in linked list       */
/* size       - size of a PARSE_KEYWORDS item                          */
/* RETURN:    - pointer to next PARSE_KEYWORDS item                    */

PARSE_KEYWORDS *parse_keywordll_add(PARSE_KEYWORDS *first, PARSE_KEYWORDS *curr, unsigned short size) {
  PARSE_KEYWORDS *next = NULL;

  if ((next = (PARSE_KEYWORDS *) malloc (size)) != (PARSE_KEYWORDS *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSE_KEYWORDS *parse_keywordll_free(PARSE_KEYWORDS *first) {
  PARSE_KEYWORDS *curr = NULL;
  PARSE_KEYWORDS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->keyword != NULL) {
      free (curr->keyword);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_KEYWORDS *) NULL);
}

/* Adds a PARSE_FUNCTIONS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_FUNCTIONS in linked list         */
/* curr       - pointer to current PARSE_FUNCTIONS in linked list       */
/* size       - size of a PARSE_FUNCTIONS item                          */
/* RETURN:    - pointer to next PARSE_FUNCTIONS item                    */

PARSE_FUNCTIONS *parse_functionll_add(PARSE_FUNCTIONS *first, PARSE_FUNCTIONS *curr, unsigned short size) {
  PARSE_FUNCTIONS *next = NULL;

  if ((next = (PARSE_FUNCTIONS *) malloc (size)) != (PARSE_FUNCTIONS *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSE_FUNCTIONS *parse_functionll_free(PARSE_FUNCTIONS *first) {
  PARSE_FUNCTIONS *curr = NULL;
  PARSE_FUNCTIONS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->function != NULL) {
      free (curr->function);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_FUNCTIONS *) NULL);
}

/* Adds a PARSE_HEADERS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_HEADERS in linked list         */
/* curr       - pointer to current PARSE_HEADERS in linked list       */
/* size       - size of a PARSE_HEADERS item                          */
/* RETURN:    - pointer to next PARSE_HEADERS item                    */

PARSE_HEADERS *parse_headerll_add(PARSE_HEADERS *first, PARSE_HEADERS *curr, unsigned short size) {
  PARSE_HEADERS *next = NULL;

  if ((next = (PARSE_HEADERS *) malloc (size)) != (PARSE_HEADERS *) NULL) {
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

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSE_HEADERS *parse_headerll_free(PARSE_HEADERS *first) {
  PARSE_HEADERS *curr = NULL;
  PARSE_HEADERS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_HEADERS *) NULL);
}

/* Adds a PARSER_MAPPING to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSER_MAPPING in linked list         */
/* curr       - pointer to current PARSER_MAPPING in linked list       */
/* size       - size of a PARSER_MAPPING item                          */
/* RETURN:    - pointer to next PARSER_MAPPING item                    */

PARSER_MAPPING *mappingll_add(PARSER_MAPPING *first, PARSER_MAPPING *curr, unsigned short size) {
  PARSER_MAPPING *next = NULL;

  if ((next = (PARSER_MAPPING *) malloc (size)) != (PARSER_MAPPING *) NULL) {
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
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSER_MAPPING *mappingll_free(PARSER_MAPPING *first) {
  PARSER_MAPPING *curr = NULL;
  PARSER_MAPPING *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->parser_name != NULL) {
      free (curr->parser_name);
    }
    if (curr->filemask != NULL) {
      free (curr->filemask);
    }
    if (curr->magic_number != NULL) {
      free (curr->magic_number);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSER_MAPPING *) NULL);
}

PARSER_MAPPING *mappingll_find(PARSER_MAPPING *first, char_t *filemask, char_t *magic_number) {
  PARSER_MAPPING *curr = NULL;
  short i = 0;

  curr = first;
  if (curr != NULL) {
    for (i = 0; curr != NULL; i++, curr = curr->next) {
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

/* Adds a PARSE_COMMENTS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_COMMENTS in linked list         */
/* curr       - pointer to current PARSE_COMMENTS in linked list       */
/* size       - size of a PARSE_COMMENTS item                          */
/* RETURN:    - pointer to next PARSE_COMMENTS item                    */

PARSE_COMMENTS *parse_commentsll_add(PARSE_COMMENTS *first, PARSE_COMMENTS *curr, unsigned short size) {
  PARSE_COMMENTS *next = NULL;

  if ((next = (PARSE_COMMENTS *) malloc (size)) != (PARSE_COMMENTS *) NULL) {
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
    free (curr);
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
    free (curr);
    curr = new_curr;
    return (curr);
  }
  /*
   * Delete the last record
   */
  if (curr->next == NULL) {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL) {
      *last = curr->prev;
    }
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */

PARSE_COMMENTS *parse_commentsll_free(PARSE_COMMENTS *first) {
  PARSE_COMMENTS *curr = NULL;
  PARSE_COMMENTS *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_COMMENTS *) NULL);
}

/* Adds a PARSE_POSTCOMPARE to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_POSTCOMPARE in linked list      */
/* curr       - pointer to current PARSE_POSTCOMPARE in linked list    */
/* size       - size of a PARSE_POSTCOMPARE item                       */
/* RETURN:    - pointer to next PARSE_POSTCOMPARE item                 */

PARSE_POSTCOMPARE *parse_postcomparell_add(PARSE_POSTCOMPARE *first, PARSE_POSTCOMPARE *curr, unsigned short size) {
  PARSE_POSTCOMPARE *next = NULL;

  if ((next = (PARSE_POSTCOMPARE *) malloc (size)) != (PARSE_POSTCOMPARE *) NULL) {
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
    free (curr->string);
  }
  if (curr->is_class_type) {
    regfree (&curr->pattern_buffer);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_POSTCOMPARE                     */
/* RETURN:    - NULL                                                   */

PARSE_POSTCOMPARE *parse_postcomparell_free(PARSE_POSTCOMPARE *first) {
  PARSE_POSTCOMPARE *curr = NULL;
  PARSE_POSTCOMPARE *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->string) {
      free (curr->string);
    }
    if (curr->is_class_type) {
      regfree (&curr->pattern_buffer);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_POSTCOMPARE *) NULL);
}

/* Adds a PARSE_EXTENSION to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_EXTENSION in linked list      */
/* curr       - pointer to current PARSE_EXTENSION in linked list    */
/* size       - size of a PARSE_EXTENSION item                       */
/* RETURN:    - pointer to next PARSE_EXTENSION item                 */

PARSE_EXTENSION *parse_extensionll_add(PARSE_EXTENSION *first, PARSE_EXTENSION *curr, unsigned short size) {
  PARSE_EXTENSION *next = NULL;

  if ((next = (PARSE_EXTENSION *) malloc (size)) != (PARSE_EXTENSION *) NULL) {
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
    free (curr->extension);
  }
  /*
   * Delete the only record
   */
  if (curr->prev == NULL && curr->next == NULL) {
    free (curr);
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
    free (curr);
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
    free (curr);
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
  free (curr);
  curr = new_curr;
  return (curr);
}

/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_EXTENSION                     */
/* RETURN:    - NULL                                                   */

PARSE_EXTENSION *parse_extensionll_free(PARSE_EXTENSION *first) {
  PARSE_EXTENSION *curr = NULL;
  PARSE_EXTENSION *new_curr = NULL;

  curr = first;
  while (curr != NULL) {
    if (curr->extension) {
      free (curr->extension);
    }
    new_curr = curr->next;
    free (curr);
    curr = new_curr;
  }
  return ((PARSE_EXTENSION *) NULL);
}

