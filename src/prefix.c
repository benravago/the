// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

/*-------------------------- declarations -----------------------------*/

static short parse_prefix_command (THE_PPC *);
static void split_prefix_command (uchar *, THE_PPC *);

static short invalidate_prefix (THE_PPC *, char *);
static short post_prefix_add (THE_PPC *, short, long);

static short prefix_makecurr (THE_PPC *, short, long);
static short prefix_add (THE_PPC *, short, long);
static short prefix_duplicate (THE_PPC *, short, long);
static short prefix_copy (THE_PPC *, short, long);
static short prefix_move (THE_PPC *, short, long);
static short prefix_lowercase (THE_PPC *, short, long);
static short prefix_uppercase (THE_PPC *, short, long);
static short prefix_delete (THE_PPC *, short, long);
static short prefix_point (THE_PPC *, short, long);
static short prefix_shift_left (THE_PPC *, short, long);
static short prefix_shift_right (THE_PPC *, short, long);
static short prefix_bounds_shift_left (THE_PPC *, short, long);
static short prefix_bounds_shift_right (THE_PPC *, short, long);
static short prefix_tabline (THE_PPC *, short, long);
static short prefix_scale (THE_PPC *, short, long);
static short prefix_show (THE_PPC *, short, long);
static short prefix_exclude (THE_PPC *, short, long);
static short prefix_block_duplicate (THE_PPC *, short, long);
static short prefix_block_copy (THE_PPC *, short, long);
static short prefix_block_move (THE_PPC *, short, long);
static short prefix_block_lowercase (THE_PPC *, short, long);
static short prefix_block_uppercase (THE_PPC *, short, long);
static short prefix_block_delete (THE_PPC *, short, long);
static short prefix_block_shift_left (THE_PPC *, short, long);
static short prefix_block_shift_right (THE_PPC *, short, long);
static short prefix_block_bounds_shift_left (THE_PPC *, short, long);
static short prefix_block_bounds_shift_right (THE_PPC *, short, long);
static short prefix_block_exclude (THE_PPC *, short, long);

static THE_PPC *find_top_ppc (THE_PPC *, short);
static THE_PPC *find_target_ppc (void);

static THE_PPC *calculate_target_line (void);

static short try_rexx_prefix_macro (THE_PPC *);

/*
 * The following two are to specify the first and last items in the
 * linked list for prefix synonyms.
 */
LINE *first_prefix_synonym = NULL;
LINE *last_prefix_synonym = NULL;

/*
 * The view in which the pending list is being executed.
 */
static VIEW_DETAILS *vd_pending;
static uchar pending_screen;

#define PENDING_VIEW           (vd_pending)
#define PENDING_FILE           (vd_pending->file_for_view)
#define PENDING_SCREEN         screen[pending_screen]
#define PENDING_WINDOW         (PENDING_SCREEN.win[vd_pending->current_window])

#define THE_PPC_NO_TARGET      (-1)
#define THE_PPC_NO_COMMAND     (-2)

#define THE_PPC_TARGET_PREVIOUS  3
#define THE_PPC_TARGET_FOLLOWING 4

/* the above two defines correspond to the position in the pc[] array  */
/* and should be changed if the position in pc[] array changes.        */

#define NUMBER_PREFIX_COMMANDS 33

static PREFIX_COMMAND pc[2][NUMBER_PREFIX_COMMANDS] = {
  {
    /* THE, XEDIT, KEDIT, KEDITW compatibility mode prefix commands */
    /* environment commands... */
    { (uchar *) "tabl", 4, PC_IS_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_tabline, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, FALSE},
    { (uchar *) "scale", 5, PC_IS_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_scale, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, FALSE},
    { (uchar *) "/", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_makecurr, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 50, NULL, FALSE, TRUE},
    /* targets... */
    /* 3 */ { (uchar *) "p", 1, PC_NOT_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_VALID_BOF, PC_INVALID_RO, NULL, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, TRUE},
    /* 4 */ { (uchar *) "f", 1, PC_NOT_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, NULL, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, TRUE},
    /* block commands... */
    { (uchar *) "lcc", 3, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_lowercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "ucc", 3, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_uppercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "\"\"", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_duplicate, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, FALSE, FALSE},
    { (uchar *) "cc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_copy, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "mm", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_move, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "dd", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_delete, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 10, NULL, FALSE, FALSE},
    { (uchar *) "xx", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_block_exclude, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "<<", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ">>", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "((", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_bounds_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "))", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_bounds_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    /* line commands - no targets... */
    { (uchar *) "lc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_lowercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "uc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_uppercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    /* line commands with targets... */
    { (uchar *) "c", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_copy, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "m", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_move, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "d", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_delete, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 10, NULL, FALSE, FALSE},
    /* line commands - no targets... */
    { (uchar *) ".", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_point, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, TRUE, FALSE},
    { (uchar *) "\"", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_duplicate, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, FALSE, FALSE},
    { (uchar *) "s", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_show, MAX_LONG, PC_IGNORE_SCOPE, PC_USE_LAST_IN_SCOPE, 30, NULL, FALSE, TRUE},
    { (uchar *) "x", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_exclude, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "<", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ">", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "(", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_bounds_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ")", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_bounds_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "i", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_add, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 60, post_prefix_add, FALSE, FALSE},
    { (uchar *) "a", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_add, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 60, post_prefix_add, FALSE, FALSE},
  },
  {
    /* ISPF compatibility mode prefix commands */
    /* environment commands... */
    { (uchar *) "tabs", 4, PC_IS_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_tabline, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, FALSE},
    { (uchar *) "cols", 4, PC_IS_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_scale, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, FALSE},
    { (uchar *) "bounds", 6, PC_IS_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_scale, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, FALSE},
    /* targets... */
    /* 3 */ { (uchar *) "b", 1, PC_NOT_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_VALID_BOF, PC_INVALID_RO, NULL, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, TRUE},
    /* 4 */ { (uchar *) "a", 1, PC_NOT_ACTION, PC_NO_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, NULL, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 0, NULL, FALSE, TRUE},
    /* block commands... */
    { (uchar *) "lcc", 3, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_lowercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "ucc", 3, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_uppercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "rr", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_duplicate, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, FALSE, FALSE},
    { (uchar *) "cc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_copy, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "mm", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_move, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "dd", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_delete, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 10, NULL, FALSE, FALSE},
    { (uchar *) "xx", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_block_exclude, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "<<", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ">>", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "((", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_bounds_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "))", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_IS_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_block_bounds_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    /* line commands - no targets... */
    { (uchar *) "lc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_lowercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "uc", 2, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_uppercase, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    /* line commands with targets... */
    { (uchar *) "c", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_copy, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "m", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_move, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "d", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_delete, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 10, NULL, FALSE, FALSE},
    /* line commands - no targets... */
    { (uchar *) ".", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_VALID_BOF, PC_VALID_RO, prefix_point, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, TRUE, FALSE},
    { (uchar *) "r", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_duplicate, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 40, NULL, FALSE, FALSE},
    { (uchar *) "s", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_show, MAX_LONG, PC_IGNORE_SCOPE, PC_USE_LAST_IN_SCOPE, 30, NULL, FALSE, TRUE},
    { (uchar *) "x", 1, PC_IS_ACTION, PC_MULTIPLES, PC_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_VALID_RO, prefix_exclude, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "<", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ">", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "(", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_bounds_shift_left, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) ")", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_INVALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_bounds_shift_right, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 30, NULL, FALSE, FALSE},
    { (uchar *) "i", 1, PC_IS_ACTION, PC_MULTIPLES, PC_NO_FULL_TARGET, PC_NOT_BLOCK, PC_TARGET_NOT_REQD, PC_VALID_TOF, PC_INVALID_BOF, PC_INVALID_RO, prefix_add, 1L, PC_RESPECT_SCOPE, PC_NO_USE_LAST_IN_SCOPE, 60, post_prefix_add, FALSE, FALSE},
  }
};

long prefix_current_line;
bool in_prefix_macro = FALSE;   /* indicate if processing prefix macro */

short execute_prefix_commands(void) {
  short ispf_idx, cmd_idx = 0, rc = RC_OK;
  uchar *mult;
  long long_mult = 0L;
  LINE *curr;
  THE_PPC *curr_ppc = NULL, *top_ppc;
  TARGET target;
  long target_type = TARGET_NORMAL;
  unsigned short y = 0, x = 0;
  int top_priority_idx = 0;
  int save_number_of_files;

  /*
   * Setup pending values for the view, file and screen to be used by all pending prefix commands.
   */
  pending_screen = current_screen;
  PENDING_VIEW = CURRENT_VIEW;

  post_process_line(PENDING_VIEW, PENDING_VIEW->focus_line, (LINE *) NULL, TRUE);
  ispf_idx = (compatible_feel == COMPAT_ISPF) ? 1 : 0;
  /*
   * Do a pass, determining the cmd_idx so that target commands can be matched
   */
  curr_ppc = PENDING_FILE->first_ppc;
  while (curr_ppc) {
    /*
     * If we have already processed the command ignore it
     */
    if (curr_ppc->ppc_processed) {
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * If an invalid prefix command from last time, ignore it.
     */
    if (*(curr_ppc->ppc_orig_command) == '?') {
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * Parse the prefix command and ignore any errors at the moment
     */
    parse_prefix_command(curr_ppc);
    curr_ppc = curr_ppc->next;
  }
  /*
   * Validate and execute all pending prefix commands from top of file down
   */
  top_priority_idx = (-1);
  curr_ppc = PENDING_FILE->first_ppc;
  while (curr_ppc) {
    /*
     * To imitate XEDIT behaviour, ignore a prefix command
     * if the line on which the prefix command has been entered is not in scope.
     */
    curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, curr_ppc->ppc_line_number, PENDING_FILE->number_lines);
    if (!(IN_SCOPE(PENDING_VIEW, curr) || PENDING_VIEW->scope_all || pc[ispf_idx][curr_ppc->ppc_cmd_idx].allowed_on_shadow_line == TRUE)) {
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * If an invalid prefix command from last time, clear it.
     */
    if (*(curr_ppc->ppc_orig_command) == '?') {
      clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * If we have already processed the command ignore it
     */
    if (curr_ppc->ppc_processed) {
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * Set the pending prefix to processed before we execute
     * as the executing prefix command is not pending at the time it is executed.
     * For the top part of a block command, we have to "unprocess" it later
     */
    curr_ppc->ppc_current_command = TRUE;
    /*
     * To get here we have 'normal' prefix command.
     */
    if (rexx_support) {         /* if REXX support is in effect ... */
      save_number_of_files = number_of_files;
      /*
       * first determine if the command is a prefix macro BEFORE looking for standard prefix commands.
       */
      rc = try_rexx_prefix_macro(curr_ppc);
      /*
       * If at this point there are no more files in the ring;
       * we assume that this was caused by exitting the last file in the ring from a prefix macro,
       * exit and ignore any more prefix commands. This is messy !!!
       */
      if (number_of_files == 0) {
        return (RC_COMMAND_NO_FILES);
      }
      if (number_of_files != save_number_of_files && PENDING_VIEW != CURRENT_VIEW) {
        return (RC_OK);
      }
      /*
       * If it was a Rexx prefix macro, then go get the next pending command
       */
      if (rc != RC_NOT_COMMAND) {
        curr_ppc->ppc_current_command = FALSE;
        curr_ppc = curr_ppc->next;
        continue;
      }
    }
    curr_ppc->ppc_current_command = FALSE;
    /*
     * if no prefix macro found for the prefix command, check to see if it is a standard prefix command.
     */
    if ((cmd_idx = parse_prefix_command(curr_ppc)) == THE_PPC_NO_TARGET) {
      invalidate_prefix(curr_ppc, NULL);
      curr_ppc = curr_ppc->next;
      continue;
    }
    if (cmd_idx == THE_PPC_NO_COMMAND) {
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * If running in read-only mode and the function selected is not valid display an error.
     */
    if (ISREADONLY(PENDING_FILE) && !pc[ispf_idx][cmd_idx].valid_in_readonly) {
      display_error(56, (uchar *) "", FALSE);
      invalidate_prefix(curr_ppc, NULL);
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * Set the block_command flag for the current prefix command to the appropriate value for the prefix command.
     */
    curr_ppc->ppc_block_command = pc[ispf_idx][cmd_idx].block_prefix_command;
    /*
     * Ignore the first instance of a block command
     */
    if (curr_ppc->ppc_block_command) {
      top_ppc = find_top_ppc(curr_ppc, curr_ppc->ppc_cmd_idx);
      if (top_ppc == NULL) {
        /*
         * For the top part of a block command, we have to set it back to "unprocessed"
         */
        // curr_ppc->ppc_processed = FALSE;
        curr_ppc = curr_ppc->next;
        continue;
      }
      /*
       * If we have the second instance of a block command work out any arguments.
       * This is done like:
       *   If the top instance has an arg, use that,
       *   otherwise use the argument from the second (current) instance
       */
      if (*(top_ppc->ppc_op[0]) && strlen((char *) top_ppc->ppc_op[0]) != 0) {
        mult = top_ppc->ppc_op[0];
      } else {
        mult = curr_ppc->ppc_op[0];
      }
    } else {
      mult = curr_ppc->ppc_op[0];
    }
    /*
     * We now have a recognised command. We have to validate its parameters
     * and find any associated pending commands.
     */
    /*
     * If the command does not allow parameters and there are parameters, error.
     */
    if (!pc[ispf_idx][cmd_idx].multiples_allowed && strcmp((char *) mult, "") != 0) {
      invalidate_prefix(curr_ppc, NULL);
      curr_ppc = curr_ppc->next;
      continue;
    }
    /*
     * If the command does allow parameters and there are no parameters, set to default...
     */
    rc = RC_OK;
    if (strcmp((char *) mult, "") == 0) {
      long_mult = pc[ispf_idx][cmd_idx].default_target;
    } else {
      /*
       * ...otherwise validate the target supplied...
       */
      if (pc[ispf_idx][cmd_idx].multiples_allowed) {
        if (!pc[ispf_idx][cmd_idx].text_arg) {  /* . command */
          if (pc[ispf_idx][cmd_idx].full_target_allowed) {
            initialise_target(&target);
            target.ignore_scope = pc[ispf_idx][cmd_idx].ignore_scope;
            if (pc[ispf_idx][cmd_idx].use_last_not_in_scope && mult[0] == '-') { /* S command */
              rc = validate_target(mult, &target, target_type, find_last_not_in_scope(PENDING_VIEW, NULL, curr_ppc->ppc_line_number, DIRECTION_FORWARD), TRUE, TRUE);
            } else {
              rc = validate_target(mult, &target, target_type, curr_ppc->ppc_line_number, TRUE, TRUE);
            }
            if (target.num_lines == 0L) {
              rc = RC_INVALID_OPERAND;
            }
            long_mult = target.num_lines;
            free_target(&target);
          } else {
            if (!valid_positive_integer(mult)) {
              rc = RC_INVALID_OPERAND;
            } else {
              long_mult = atol((char *) mult);
            }
          }
        }
      }
    }
    /*
     * If the target is invalid or not found, invalidate the command.
     */
    if (rc != RC_OK) {
      invalidate_prefix(curr_ppc, NULL);
      curr_ppc = curr_ppc->next;
      continue;
    }
    curr_ppc->ppc_cmd_param = long_mult;
    /*
     * Execute the function associated with the prefix command...
     */
    if (cmd_idx != (-1) && cmd_idx != THE_PPC_NO_COMMAND && pc[ispf_idx][cmd_idx].function != NULL) {
      rc = (*pc[ispf_idx][cmd_idx].function) (curr_ppc, cmd_idx, long_mult);
      if (rc == RC_OK) {
        curr_ppc->ppc_processed = TRUE;
      }
    }
    /*
     * Determine the prefix command with the highest priority and save it.
     */
    if (cmd_idx >= 0) {
      if (top_priority_idx == (-1)) {
        top_priority_idx = cmd_idx;
      } else {
        if (pc[ispf_idx][cmd_idx].priority > pc[ispf_idx][top_priority_idx].priority) {
          top_priority_idx = cmd_idx;
        }
      }
    }
    curr_ppc = curr_ppc->next;
  }
  /*
   * The "cleared" pending prefix commands now need to be deleted from the linked list...
   *
   * Only clear the pending prefix command if it is NOT a builtin prefix command
   * and it was NOT created by SET PENDING (ie it was typed into the prefix area
   */
  curr_ppc = PENDING_FILE->first_ppc;
  while (curr_ppc) {
    if (curr_ppc->ppc_cmd_idx == (-1) &&!curr_ppc->ppc_set_by_pending) { /* NOT a builtin */
      curr_ppc = delete_pending_prefix_command(curr_ppc, PENDING_FILE, NULL);
    } else {
      curr_ppc = curr_ppc->next;
    }
  }
  /*
   * Now that we are here, display the new version of the screen.
   */
  if (top_priority_idx != (-1)) {
    if (pc[ispf_idx][top_priority_idx].post_function != NULL) {
      rc = (*pc[ispf_idx][top_priority_idx].post_function) (curr_ppc, cmd_idx, long_mult);
    }
  }
  if (PENDING_VIEW == CURRENT_VIEW) {
    pre_process_line(PENDING_VIEW, PENDING_VIEW->focus_line, (LINE *) NULL);
    build_screen(pending_screen);
    display_screen(pending_screen);
    if (PENDING_VIEW->current_window != WINDOW_COMMAND) {
      if (curses_started) {
        getyx(PENDING_WINDOW, y, x);
      }
      PENDING_VIEW->focus_line = get_focus_line_in_view(pending_screen, PENDING_VIEW->focus_line, y);
      y = get_row_for_focus_line(pending_screen, PENDING_VIEW->focus_line, PENDING_VIEW->current_row);
      if (curses_started) {
        wmove(PENDING_WINDOW, y, x);
      }
      pre_process_line(PENDING_VIEW, PENDING_VIEW->focus_line, (LINE *) NULL);
    }
  } else {
    pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    build_screen(current_screen);
    display_screen(current_screen);
    if (CURRENT_VIEW->current_window != WINDOW_COMMAND) {
      if (curses_started) {
        getyx(CURRENT_WINDOW, y, x);
      }
      CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen, CURRENT_VIEW->focus_line, y);
      y = get_row_for_focus_line(current_screen, CURRENT_VIEW->focus_line, CURRENT_VIEW->current_row);
      if (curses_started) {
        wmove(CURRENT_WINDOW, y, x);
      }
      pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
    }
  }
  return (RC_OK);
}

static short parse_prefix_command(THE_PPC *curr_ppc) {
  short i = 0;
  short rc = RC_OK;
  LINE *curr = NULL;
  int ispf_idx = (compatible_feel == COMPAT_ISPF) ? 1 : 0;

  /*
   * For each pending prefix command for the current view, execute the appropriate command.
   */
  rc = THE_PPC_NO_TARGET;
  if (blank_field(curr_ppc->ppc_command)) {     /* if prefix command is blank, return */
    return (THE_PPC_NO_COMMAND);
  }
  /*
   * For each prefix synonym, determine if the supplied prefix command matches
   * Synonyms are case sensitive
   */
  curr = first_prefix_synonym;
  while (curr) {
    if (strcmp((char *) curr_ppc->ppc_command, (char *) curr->name) != 0) {
      curr = curr->next;
      continue;
    }
    /*
     * Now that a match on synonym is made, determine the original prefix command associated with that synonym...
     */
    for (i = 0; i < NUMBER_PREFIX_COMMANDS; i++) {
      if (pc[ispf_idx][i].cmd == NULL) {
        break;
      }
      if (strcmp((char *) pc[ispf_idx][i].cmd, (char *) curr->line) == 0) {
        curr_ppc->ppc_cmd_idx = i;
        return (i);
      }
    }
    /*
     * To get here we found a prefix synonym, but no matching original command, so return an error.
     */
    curr_ppc->ppc_cmd_idx = (-1);
    return (rc);
  }
  /*
   * We don't have a prefix synonym for the supplied command; look for builtin prefix commands.
   * Builtin prefix commands are case insensitive
   */
  for (i = 0; i < NUMBER_PREFIX_COMMANDS; i++) {
    if (pc[ispf_idx][i].cmd == NULL) {
      break;
    }
    if (strcasecmp((char *) curr_ppc->ppc_command, (char *) pc[ispf_idx][i].cmd) == 0) {
      /*
       * Set a flag in ppc[] array to indicate which command is present.
       */
      curr_ppc->ppc_cmd_idx = i;
      rc = i;
      break;
    }
  }
  /*
   * If command not found, set a flag in ppc[] array to indicate command is invalid.
   */
  if (rc == THE_PPC_NO_TARGET) {
    curr_ppc->ppc_cmd_idx = (-1);
  }
  return (rc);
}

static short prefix_makecurr(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long top_line = curr_ppc->ppc_line_number;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  execute_makecurr(pending_screen, PENDING_VIEW, top_line);
  if (number_lines != 0L) {
    PENDING_VIEW->current_column = (long) number_lines;
  }
  return (0);
}

static short prefix_tabline(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long top_line = curr_ppc->ppc_line_number;
  short tab_row = 0;

  if ((tab_row = get_row_for_focus_line(pending_screen, top_line, (-1))) != (-1)) {
    PENDING_VIEW->tab_base = POSITION_TOP;
    PENDING_VIEW->tab_off = tab_row + 1;
    PENDING_VIEW->tab_on = TRUE;
  }
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  return (0);
}

static short prefix_scale(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long top_line = curr_ppc->ppc_line_number;
  short scale_row = 0;

  if ((scale_row = get_row_for_focus_line(pending_screen, top_line, (-1))) != (-1)) {
    PENDING_VIEW->scale_base = POSITION_TOP;
    PENDING_VIEW->scale_off = scale_row + 1;
    PENDING_VIEW->scale_on = TRUE;
  }
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  return (0);
}

static short prefix_show(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long i = 0;
  short rc = RC_OK;
  long top_line = curr_ppc->ppc_line_number;
  long target_line = 0L;
  LINE *curr = NULL;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (TOF(top_line) || BOF(top_line)) {
    return (-1);
  }
  /*
   * Find the current line from where we start showing...
   */
  curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, top_line, PENDING_FILE->number_lines);
  /*
   * If the line that the show command was entered is in scope, then
   * return with an error.
   */
  if (IN_SCOPE(PENDING_VIEW, curr)) {
    invalidate_prefix(curr_ppc, NULL);
    return (-1);
  }
  /*
   * For a negative target, show the lines from the end of the excluded block.
   */
  if (number_lines < 0) {
    target_line = find_last_not_in_scope(PENDING_VIEW, curr, top_line, DIRECTION_FORWARD);
    curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, target_line, PENDING_FILE->number_lines);
    for (i = 0; i > number_lines; i--) {
      curr->select = PENDING_VIEW->display_high;
      curr = curr->prev;
      if (curr->prev == NULL || IN_SCOPE(PENDING_VIEW, curr)) {
        break;
      }
    }
  } else {
    /*
     * For a positive target, show the lines from the start of the excluded block.
     */
    for (i = 0; i < number_lines; i++) {
      curr->select = PENDING_VIEW->display_high;
      curr = curr->next;
      if (curr->next == NULL || IN_SCOPE(PENDING_VIEW, curr)) {
        break;
      }
    }
  }
  return (rc);
}

static short prefix_exclude(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = RC_OK;
  long top_line = curr_ppc->ppc_line_number;
  LINE *curr = NULL;
  short direction = DIRECTION_FORWARD;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (TOF(top_line) || BOF(top_line)) {
    return (-1);
  }
  /*
   * If the high value of SET DISPLAY is 255, we can't exclude any lines so exit.
   */
  if (PENDING_VIEW->display_high == 255) {
    return (rc);
  }
  if (number_lines < 0) {
    direction = DIRECTION_BACKWARD;
  } else {
    direction = DIRECTION_FORWARD;
  }
  /*
   * Find the current line from where we start excluding...
   */
  curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, top_line, PENDING_FILE->number_lines);
  /*
   * For the number of lines affected, change the select level if the line is in scope.
   */
  while (number_lines != 0) {
    if (IN_SCOPE(PENDING_VIEW, curr)) {
      curr->select = (short) PENDING_VIEW->display_high + 1;
    }
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
    number_lines -= direction;
  }
  /*
   * Determine if current line is now not in scope...
   */
  curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, PENDING_VIEW->current_line, PENDING_FILE->number_lines);
  if (!IN_SCOPE(PENDING_VIEW, curr)) {
    PENDING_VIEW->current_line = find_next_in_scope(PENDING_VIEW, curr, PENDING_VIEW->current_line, DIRECTION_FORWARD);
  }
  return (rc);
}

static short prefix_add(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = (-1);
  long top_line = curr_ppc->ppc_line_number;

  if (top_line == PENDING_FILE->number_lines + 1) {
    top_line--;
  }
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  rc = insert_new_line(pending_screen, PENDING_VIEW, (uchar *) "", 0, number_lines, top_line, FALSE, FALSE, TRUE, PENDING_VIEW->display_low, FALSE, FALSE);
  return (rc);
}

static short prefix_duplicate(THE_PPC *curr_ppc, short cmd_idx, long number_occ) {
  short rc = (-1);
  long top_line = curr_ppc->ppc_line_number;
  long lines_affected = 0L;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (top_line != 0L && top_line != PENDING_FILE->number_lines + 1) {
    rc = rearrange_line_blocks(COMMAND_DUPLICATE, SOURCE_PREFIX, top_line, top_line, top_line, number_occ, PENDING_VIEW, PENDING_VIEW, TRUE, &lines_affected);
  }
  return (rc);
}

static short prefix_copy(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long bottom_line = 0L, target_line = 0L, lines_affected = 0L;
  long top_line = curr_ppc->ppc_line_number;
  THE_PPC *target_ppc = NULL;
  short rc = (-1);

  if ((target_ppc = calculate_target_line()) == NULL) {
    return (rc);
  }
  target_line = target_ppc->ppc_line_number + target_ppc->ppc_cmd_param;
  bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (top_line != 0L && top_line != PENDING_FILE->number_lines + 1) {
    rc = rearrange_line_blocks(COMMAND_COPY, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, TRUE, &lines_affected);
  }
  clear_pending_prefix_command(target_ppc, PENDING_FILE, (LINE *) NULL);
  return (rc);
}

static short prefix_move(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long bottom_line = 0L, target_line = 0L, lines_affected = 0L;
  long top_line = curr_ppc->ppc_line_number;
  THE_PPC *target_ppc = NULL;
  short rc = (-1);

  if ((target_ppc = calculate_target_line()) == NULL) {
    return (rc);
  }
  target_line = target_ppc->ppc_line_number + target_ppc->ppc_cmd_param;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);
  if (top_line != 0L && top_line != PENDING_FILE->number_lines + 1) {
    if ((rc = rearrange_line_blocks(COMMAND_MOVE_COPY_SAME, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, TRUE, &lines_affected)) != RC_OK) {
      return (rc);
    }
    if (target_line < top_line) {
      top_line += number_lines;
      target_line += number_lines;
      bottom_line += number_lines;
    }
    rc = rearrange_line_blocks(COMMAND_MOVE_DELETE_SAME, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, TRUE, &lines_affected);
  }
  clear_pending_prefix_command(target_ppc, PENDING_FILE, (LINE *) NULL);
  return (rc);
}

static short prefix_point(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  int rc = RC_OK;
  uchar buf[MAX_PREFIX_WIDTH + 2];

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  strcpy((char *) buf, ".");
  strcat((char *) buf, (char *) curr_ppc->ppc_op[0]);
  if (execute_set_point(pending_screen, PENDING_VIEW, buf, curr_ppc->ppc_line_number, TRUE) != RC_OK) {
    invalidate_prefix(curr_ppc, NULL);
    rc = (-1);
  }
  return (rc);
}

static short prefix_delete(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = (-1);
  long top_line = curr_ppc->ppc_line_number;
  long bottom_line = 0L, target_line = 0L, lines_affected = 0L;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (!VIEW_TOF(PENDING_VIEW, top_line) && !VIEW_BOF(PENDING_VIEW, top_line)) {
    bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);
    target_line = (number_lines < 0L) ? (bottom_line) : (top_line);
    rc = rearrange_line_blocks(COMMAND_DELETE, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, TRUE, &lines_affected);
  }
  return (rc);
}

static short prefix_shift_left(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = curr_ppc->ppc_line_number;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (top_line != 0L && top_line != PENDING_FILE->number_lines + 1) {
    execute_shift_command(pending_screen, PENDING_VIEW, TRUE, number_cols, top_line, 1L, TRUE, TARGET_UNFOUND, FALSE, FALSE);
  }
  return (0);
}

static short prefix_shift_right(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long start_line = curr_ppc->ppc_line_number;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (start_line != 0L && start_line != PENDING_FILE->number_lines + 1) {
    execute_shift_command(pending_screen, PENDING_VIEW, FALSE, number_cols, start_line, 1L, TRUE, TARGET_UNFOUND, FALSE, FALSE);
  }
  return (0);
}

static short prefix_bounds_shift_left(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = curr_ppc->ppc_line_number;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (top_line != 0L && top_line != PENDING_FILE->number_lines + 1) {
    execute_shift_command(pending_screen, PENDING_VIEW, TRUE, number_cols, top_line, 1L, TRUE, TARGET_UNFOUND, FALSE, TRUE);
  }
  return (0);
}

static short prefix_bounds_shift_right(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long start_line = curr_ppc->ppc_line_number;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (start_line != 0L && start_line != PENDING_FILE->number_lines + 1) {
    execute_shift_command(pending_screen, PENDING_VIEW, FALSE, number_cols, start_line, 1L, TRUE, TARGET_UNFOUND, FALSE, TRUE);
  }
  return (0);
}

static short prefix_lowercase(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long start_line = curr_ppc->ppc_line_number;
  long start_col = PENDING_VIEW->zone_start - 1;
  long end_col = PENDING_VIEW->zone_end - 1;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (start_line != 0L && start_line != PENDING_FILE->number_lines + 1) {
    do_actual_change_case(start_line, number_lines, CASE_LOWER, FALSE, DIRECTION_FORWARD, start_col, end_col);
  }
  return (0);
}

static short prefix_uppercase(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long start_line = curr_ppc->ppc_line_number;
  long start_col = PENDING_VIEW->zone_start - 1;
  long end_col = PENDING_VIEW->zone_end - 1;

  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  if (start_line != 0L && start_line != PENDING_FILE->number_lines + 1) {
    do_actual_change_case(start_line, number_lines, CASE_UPPER, FALSE, DIRECTION_FORWARD, start_col, end_col);
  }
  return (0);
}

static short prefix_block_duplicate(THE_PPC *curr_ppc, short cmd_idx, long number_occ) {
  short rc = (-1);
  long top_line = 0L, bottom_line = 0L, lines_affected = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (rc);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  rc = rearrange_line_blocks(COMMAND_DUPLICATE, SOURCE_PREFIX, top_line, bottom_line, bottom_line, number_occ, PENDING_VIEW, PENDING_VIEW, FALSE, &lines_affected);
  return (rc);
}

static short prefix_block_copy(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = (-1);
  long top_line = 0L, bottom_line = 0L, target_line = 0L, lines_affected = 0L;
  THE_PPC *top_ppc = NULL;
  THE_PPC *target_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (rc);
  }
  if ((target_ppc = calculate_target_line()) == NULL) {
    return (rc);
  }
  target_line = target_ppc->ppc_line_number + target_ppc->ppc_cmd_param;
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  rc = rearrange_line_blocks(COMMAND_COPY, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, FALSE, &lines_affected);
  clear_pending_prefix_command(target_ppc, PENDING_FILE, (LINE *) NULL);
  return (rc);
}

static short prefix_block_move(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = (-1);
  long top_line = 0L, bottom_line = 0L, target_line = 0L, num_lines = 0L, lines_affected = 0L;
  THE_PPC *top_ppc = NULL;
  THE_PPC *target_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (rc);
  }
  if ((target_ppc = calculate_target_line()) == NULL) {
    return (rc);
  }
  target_line = target_ppc->ppc_line_number + target_ppc->ppc_cmd_param;
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  /*
   * Quick validation of line numbers. Cannot move inside the block
   */
  if (target_line > top_line && target_line < bottom_line) {
    invalidate_prefix(target_ppc, "Prefix \"%s\" is invalid for the line on which is was entered");
    return (-1);
  }
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  if ((rc = rearrange_line_blocks(COMMAND_MOVE_COPY_SAME, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, FALSE, &lines_affected)) != RC_OK) {
    return (rc);
  }
  if (target_line < top_line) {
    num_lines = bottom_line - top_line + 1L;
    top_line += num_lines;
    bottom_line += num_lines;
    target_line += num_lines;
  }
  rc = rearrange_line_blocks(COMMAND_MOVE_DELETE_SAME, SOURCE_PREFIX, top_line, bottom_line, target_line, 1L, PENDING_VIEW, PENDING_VIEW, FALSE, &lines_affected);
  clear_pending_prefix_command(target_ppc, PENDING_FILE, (LINE *) NULL);
  return (rc);
}

static short prefix_block_delete(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = (-1);
  long top_line = 0L, bottom_line = 0L, lines_affected = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (rc);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  rc = rearrange_line_blocks(COMMAND_DELETE, SOURCE_PREFIX, top_line, bottom_line, bottom_line, 1L, PENDING_VIEW, PENDING_VIEW, FALSE, &lines_affected);
  return (rc);
}

static short prefix_block_shift_left(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  execute_shift_command(pending_screen, PENDING_VIEW, TRUE, number_cols, top_line, bottom_line - top_line + 1L, FALSE, TARGET_UNFOUND, FALSE, FALSE);
  return (0);
}

static short prefix_block_shift_right(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  execute_shift_command(pending_screen, PENDING_VIEW, FALSE, number_cols, top_line, bottom_line - top_line + 1L, FALSE, TARGET_UNFOUND, FALSE, FALSE);
  return (0);
}

static short prefix_block_exclude(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = RC_OK;
  long top_line = 0L, bottom_line = 0L, num_lines = 0L, i = 0L;
  LINE *curr = NULL;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  num_lines = bottom_line - top_line + 1L;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  /*
   * If the high value of SET DISPLAY is 255, we can't exclude any lines so exit.
   */
  if (PENDING_VIEW->display_high == 255) {
    return (rc);
  }
  /*
   * Find the current line from where we start excluding...
   */
  curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, top_line, PENDING_FILE->number_lines);
  /*
   * For the number of lines affected, change the select level if the line is in scope.
   */
  for (i = 0; i < num_lines; i++) {
    if (IN_SCOPE(PENDING_VIEW, curr)) {
      curr->select = (short) PENDING_VIEW->display_high + 1;
    }
    curr = curr->next;
  }
  /*
   * Determine if current line is now not in scope...
   */
  curr = lll_find(PENDING_FILE->first_line, PENDING_FILE->last_line, PENDING_VIEW->current_line, PENDING_FILE->number_lines);
  if (!IN_SCOPE(PENDING_VIEW, curr)) {
    PENDING_VIEW->current_line = find_next_in_scope(PENDING_VIEW, curr, PENDING_VIEW->current_line, DIRECTION_FORWARD);
  }
  return (rc);
}

static short prefix_block_bounds_shift_left(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  execute_shift_command(pending_screen, PENDING_VIEW, TRUE, number_cols, top_line, bottom_line - top_line + 1L, FALSE, TARGET_UNFOUND, FALSE, TRUE);
  return (0);
}

static short prefix_block_bounds_shift_right(THE_PPC *curr_ppc, short cmd_idx, long number_cols) {
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  execute_shift_command(pending_screen, PENDING_VIEW, FALSE, number_cols, top_line, bottom_line - top_line + 1L, FALSE, TARGET_UNFOUND, FALSE, TRUE);
  return (0);
}

static short prefix_block_lowercase(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long start_col = PENDING_VIEW->zone_start - 1;
  long end_col = PENDING_VIEW->zone_end - 1;
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  do_actual_change_case(top_line, bottom_line - top_line + 1L, CASE_LOWER, FALSE, DIRECTION_FORWARD, start_col, end_col);
  return (0);
}

static short prefix_block_uppercase(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  long start_col = PENDING_VIEW->zone_start - 1;
  long end_col = PENDING_VIEW->zone_end - 1;
  long top_line = 0L, bottom_line = 0L;
  THE_PPC *top_ppc = NULL;

  if ((top_ppc = find_top_ppc(curr_ppc, cmd_idx)) == NULL) {
    return (-1);
  }
  top_line = top_ppc->ppc_line_number;
  bottom_line = curr_ppc->ppc_line_number;
  top_line = (top_line == 0L) ? 1L : top_line;
  bottom_line = (bottom_line == PENDING_FILE->number_lines + 1L) ? bottom_line - 1L : bottom_line;
  clear_pending_prefix_command(curr_ppc, PENDING_FILE, (LINE *) NULL);
  clear_pending_prefix_command(top_ppc, PENDING_FILE, (LINE *) NULL);
  do_actual_change_case(top_line, bottom_line - top_line + 1L, CASE_UPPER, FALSE, DIRECTION_FORWARD, start_col, end_col);
  return (0);
}

static short invalidate_prefix(THE_PPC *curr_ppc, char *template) {
  short len = 0;
  uchar buf[MAX_PREFIX_WIDTH + 150];

  if (*(curr_ppc->ppc_orig_command) != '?') {
    len = strlen((char *) curr_ppc->ppc_orig_command);
    meminschr(curr_ppc->ppc_orig_command, '?', 0, PENDING_VIEW->prefix_width, len);
    *(curr_ppc->ppc_orig_command + len + 1) = '\0';
  }
  /*
   * ensure that there is no chance that a 'real' prefix command can be executed.
   */
  curr_ppc->ppc_cmd_idx = THE_PPC_NO_COMMAND;
  /*
   * Display an error. If we supply a template use that.
   */
  if (template == NULL) {
    sprintf((char *) buf, "Invalid prefix subcommand: %s", curr_ppc->ppc_orig_command + 1);
  } else {
    sprintf((char *) buf, template, curr_ppc->ppc_orig_command + 1);
  }
  display_error(0, (uchar *) buf, FALSE);
  return (RC_OK);
}

void clear_pending_prefix_command(THE_PPC *curr_ppc, FILE_DETAILS *curr_file, LINE *curr_line) {
  LINE *curr = curr_line;

  /*
   * If curr_ppc == NULL, then do nothing.
   */
  if (curr_ppc == NULL) {
    return;
  }
  if (curr == (LINE *) NULL) {
    curr = lll_find(curr_file->first_line, curr_file->last_line, curr_ppc->ppc_line_number, curr_file->number_lines);
  }
  curr->pre = NULL;
  curr_ppc->ppc_cmd_idx = (-1);
  curr_ppc->ppc_block_command = FALSE;
  curr_ppc->ppc_shadow_line = FALSE;
  curr_ppc->ppc_set_by_pending = FALSE;
  curr_ppc->ppc_orig_command[0] = '\0';
  curr_ppc->ppc_command[0] = '\0';
  curr_ppc->ppc_processed = TRUE;
  return;
}

THE_PPC *delete_pending_prefix_command(THE_PPC *curr_ppc, FILE_DETAILS *curr_file, LINE *curr_line) {
  LINE *curr = curr_line;
  THE_PPC *return_ppc = NULL;

  /*
   * If curr_ppc == NULL, then do nothing.
   */
  if (curr_ppc == NULL) {
    return (NULL);
  }
  if (curr == (LINE *) NULL) {
    curr = lll_find(curr_file->first_line, curr_file->last_line, curr_ppc->ppc_line_number, curr_file->number_lines);
  }
  curr->pre = NULL;
  return_ppc = pll_del(&(curr_file->first_ppc), &(curr_file->last_ppc), curr_ppc, DIRECTION_FORWARD);
  return (return_ppc);
}

static THE_PPC *find_top_ppc(THE_PPC *curr_ppc, short top_cmd_idx) {
  THE_PPC *top_ppc = PENDING_FILE->first_ppc;

  while (top_ppc != curr_ppc) {
    if (top_cmd_idx == top_ppc->ppc_cmd_idx && top_ppc->ppc_processed == FALSE) {
      return (top_ppc);
    }
    top_ppc = top_ppc->next;
  }
  return (NULL);
}

static THE_PPC *find_target_ppc(void) {
  THE_PPC *target_ppc = NULL;

  target_ppc = PENDING_FILE->first_ppc;
  while (target_ppc != NULL) {
    if (target_ppc->ppc_processed == FALSE) {
      if (target_ppc->ppc_cmd_idx == THE_PPC_TARGET_PREVIOUS || target_ppc->ppc_cmd_idx == THE_PPC_TARGET_FOLLOWING) {
        return (target_ppc);
      }
    }
    target_ppc = target_ppc->next;
  }
  return (NULL);
}

static THE_PPC *calculate_target_line(void) {
  THE_PPC *target_ppc = NULL;

  if ((target_ppc = find_target_ppc()) == NULL) {
    return (NULL);
  }
  target_ppc->ppc_cmd_param = 0L;
  switch (target_ppc->ppc_cmd_idx) {
    case THE_PPC_TARGET_PREVIOUS:
      /*
       * If the target line is NOT top of file line and the target type is PREVIOUS,
       * subtract 1 from the target line.
       */
      if (!TOF(target_ppc->ppc_line_number)) {
        target_ppc->ppc_cmd_param = (-1);
      }
      break;
    case THE_PPC_TARGET_FOLLOWING:
      /*
       * If the target line is the bottom of file and the target type is FOLLOWING,
       * subtract 1 from the target line.
       */
      if (BOF(target_ppc->ppc_line_number)) {
        target_ppc->ppc_cmd_param = (-1);
      }
      break;
    default:
      break;
  }
  return (target_ppc);
}

static short try_rexx_prefix_macro(THE_PPC *curr_ppc) {
  short pmacro_rc = 0, errnum = 0;
  long line_number = 0L;
  uchar pm_parms[(MAX_PREFIX_WIDTH * 4) + 1];
  short macrorc = 0;
  int i;

  /*
   * If the parsed command is blank, ignore it.
   */
  if (blank_field(curr_ppc->ppc_command)) {
    return (RC_NOT_COMMAND);
  }
  get_valid_macro_file_name(find_prefix_synonym(curr_ppc->ppc_command), temp_cmd, macro_suffix, &errnum);
  if (errnum == 0) {
    line_number = curr_ppc->ppc_line_number;
    /*
     * If the prefix command was entered on a shadow line, pass this to the prefix macro...
     */
    if (curr_ppc->ppc_shadow_line) {
      sprintf((char *) pm_parms, " PREFIX %s SHADOW %ld", curr_ppc->ppc_command, line_number);
    } else {
      sprintf((char *) pm_parms, " PREFIX %s SET %ld", curr_ppc->ppc_command, line_number);
    }
    for (i = 0; i < PPC_OPERANDS; i++) {
      if (strlen((char *) curr_ppc->ppc_op[i]) != 0) {
        strcat((char *) pm_parms, " ");
        strcat((char *) pm_parms, (char *) curr_ppc->ppc_op[i]);
      }
    }
    strcat((char *) temp_cmd, (char *) pm_parms);       /* add on the parameter list */
    prefix_current_line = line_number;
    in_prefix_macro = TRUE;
    /*
     * Go and execute the prefix macro.
     */
    pmacro_rc = execute_macro(temp_cmd, TRUE, &macrorc);
    in_prefix_macro = FALSE;
  } else {
    pmacro_rc = RC_NOT_COMMAND;
  }
  return (pmacro_rc);
}

#define STATE_BEFORE_CMD 0
#define STATE_ALPHA_CMD 1
#define STATE_OTHER_CMD 2
#define STATE_DIGIT_OPERAND 3
#define STATE_REM_OPERAND 4

static void split_prefix_command(uchar *str, THE_PPC *curr_ppc) {
  int opt_idx[PPC_OPERANDS] = { 0, };
  char cmd[MAX_PREFIX_WIDTH];
  int cmd_idx = 0;
  int i, len = strlen((char *) str);
  char ch = ' ', lastch;
  int state = STATE_BEFORE_CMD;
  int option = -1;

  for (i = 0; i < len; i++) {
    lastch = ch;
    ch = str[i];
    switch (state) {
      case STATE_BEFORE_CMD:
        /*
         * determine what the cmd starts with:
         * ALPHA - alpha cmd
         * DIGIT - operand
         * other - other cmd
         */
        if (isalpha(ch)) {
          cmd[cmd_idx] = ch;
          cmd_idx++;
          state = STATE_ALPHA_CMD;
          break;
        } else if (isdigit(ch)) {
          if (isspace(lastch)) {
            /*
             * When we get a space, increment the option number.
             * Don't do this if we are processing the last option;
             * include the spaces as part of the last option
             */
            if (option < PPC_OPERANDS - 1) {
              option++;
            }
          }
          curr_ppc->ppc_op[option][opt_idx[option]] = ch;
          opt_idx[option]++;
          state = STATE_DIGIT_OPERAND;
        } else if (isspace(ch)) {
          // no-op
        } else {
          cmd[cmd_idx] = ch;
          cmd_idx++;
          state = STATE_OTHER_CMD;
          break;
        }
        break;
      case STATE_ALPHA_CMD:
        if (isalpha(ch)) {
          cmd[cmd_idx] = ch;
          cmd_idx++;
          state = STATE_ALPHA_CMD;
          break;
        } else {
          /*
           * End of ALPHA_CMD
           */
          if (!isspace(ch)) {
            if (option < PPC_OPERANDS - 1) {
              option++;
            }
            curr_ppc->ppc_op[option][opt_idx[option]] = ch;
            opt_idx[option]++;
          }
          state = STATE_REM_OPERAND;
        }
        break;
      case STATE_OTHER_CMD:
        if (isspace(ch) || isalpha(ch) || isdigit(ch)) {
          /*
           * End of OTHER_CMD
           */
          if (!isspace(ch)) {
            if (option < PPC_OPERANDS - 1) {
              option++;
            }
            curr_ppc->ppc_op[option][opt_idx[option]] = ch;
            opt_idx[option]++;
            state = STATE_REM_OPERAND;
          }
        } else {
          /* still the cmd... */
          cmd[cmd_idx] = ch;
          cmd_idx++;
        }
        break;
      case STATE_DIGIT_OPERAND:        /* only valid BEFORE CMD */
        if (isspace(ch)) {
          if (option == PPC_OPERANDS - 1) {
            curr_ppc->ppc_op[option][opt_idx[option]] = ch;
            opt_idx[option]++;
          }
        } else if (isdigit(ch)) {
          if (isspace(lastch)) {
            if (option < PPC_OPERANDS - 1) {
              option++;
            }
          }
          curr_ppc->ppc_op[option][opt_idx[option]] = ch;
          opt_idx[option]++;
        } else if (isalpha(ch)) {
          cmd[cmd_idx] = ch;
          cmd_idx++;
          state = STATE_ALPHA_CMD;
          break;
        } else {
          cmd[cmd_idx] = ch;
          cmd_idx++;
          state = STATE_OTHER_CMD;
        }
        break;
      /*
       * Operands after the command delimited by spaces:
       */
      case STATE_REM_OPERAND:  /* only valid AFTER CMD */
        if (option < PPC_OPERANDS - 1) {
          if (isspace(ch)) {
            break;
          }
          if (isspace(lastch)) {
            if (option < PPC_OPERANDS) {
              option++;
            }
          }
        }
        curr_ppc->ppc_op[option][opt_idx[option]] = ch;
        opt_idx[option]++;
        break;
      default:
        break;
    }
  }
  /*
   * Nul Terminate the command
   */
  cmd[cmd_idx] = '\0';
  strcpy((char *) curr_ppc->ppc_command, (char *) cmd);
  /*
   * Nul Terminate each option
   */
  for (i = 0; i < PPC_OPERANDS; i++) {
    cmd_idx = opt_idx[i];
    curr_ppc->ppc_op[i][cmd_idx] = '\0';
  }
}

void add_prefix_command(uchar curr_screen, VIEW_DETAILS *curr_view, LINE *curr, long line_number, bool block_command, bool set_by_pending) {
  short i = 0;
  uchar temp_prefix_array[MAX_PREFIX_WIDTH + 1];
  THE_PPC *curr_ppc = NULL;
  bool redisplay_screen = FALSE;

  prefix_changed = FALSE;
  /*
   * Copy the contexts of the prefix record area into a temporary area.
   */
  for (i = 0; i < pre_rec_len; i++) {
    temp_prefix_array[i] = pre_rec[i];
  }
  temp_prefix_array[pre_rec_len] = '\0';
  strtrunc(temp_prefix_array);
  /*
   * If the prefix record area is blank, clear the pending prefix area.
   */
  if (blank_field(temp_prefix_array)) {
    (void) delete_pending_prefix_command(curr->pre, curr_view->file_for_view, curr);
    redisplay_screen = TRUE;
  } else {
    /*
     * If the input line already points to an entry in the array,
     * use the existing entry in the array, otherwise add to the next entry.
     */
    curr_ppc = pll_find(curr_view->file_for_view->first_ppc, line_number);
    if (curr_ppc == NULL) {     /* not found */
      curr_ppc = pll_add(&curr_view->file_for_view->first_ppc, sizeof(THE_PPC), line_number);
      if (curr_ppc == NULL) {
        display_error(30, (uchar *) "", FALSE);
        return;
      }
    }
    curr->pre = curr_ppc;
    /*
     * Parse the prefix command line into command and operands.
     */
    strcpy((char *) curr_ppc->ppc_orig_command, (char *) temp_prefix_array);
    split_prefix_command(temp_prefix_array, curr_ppc);
    curr_ppc->ppc_line_number = line_number;
    curr_ppc->ppc_block_command = block_command;
    curr_ppc->ppc_cmd_idx = (-1);
    if (IN_SCOPE(curr_view, curr)) {
      curr_ppc->ppc_shadow_line = FALSE;
    } else {
      curr_ppc->ppc_shadow_line = TRUE;
    }
    /*
     * Set whether the prefix command was entered in the prefix are or by SET PENDING
     */
    curr_ppc->ppc_set_by_pending = set_by_pending;
    curr_ppc->ppc_processed = FALSE;
  }
  if (redisplay_screen) {
    build_screen(curr_screen);
    display_screen(curr_screen);
  }
  return;
}

short add_prefix_synonym(uchar *synonym, uchar *macroname) {
  LINE *curr = NULL;

  /*
   * First thing is to delete any definitions that may exist for the supplied synonym.
   */
  curr = first_prefix_synonym;
  while (curr != NULL) {
    if (strcmp((char *) curr->name, (char *) synonym) == 0) {
      if (curr->name != NULL) {
        free(curr->name);
      }
      if (curr->line != NULL) {
        free(curr->line);
      }
      curr = lll_del(&first_prefix_synonym, &last_prefix_synonym, curr, DIRECTION_FORWARD);
    } else {
      curr = curr->next;
    }
  }
  /*
   * If synonym and macroname are the same we are deleting the synonym
   * (already done above); just don't add it.
   */
  if (strcmp((char *) synonym, (char *) macroname) != 0) {
    /*
     * Lastly append the synonym at the end of the linked list.
     */
    curr = lll_add(first_prefix_synonym, last_prefix_synonym, sizeof(LINE));
    if (curr == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    curr->line = (uchar *) malloc((strlen((char *) macroname) + 1) * sizeof(uchar));
    if (curr->line == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) curr->line, (char *) macroname);
    curr->name = (uchar *) malloc((strlen((char *) synonym) + 1) * sizeof(uchar));
    if (curr->name == NULL) {
      display_error(30, (uchar *) "", FALSE);
      return (RC_OUT_OF_MEMORY);
    }
    strcpy((char *) curr->name, (char *) synonym);
    last_prefix_synonym = curr;
    if (first_prefix_synonym == NULL) {
      first_prefix_synonym = last_prefix_synonym;
    }
  }
  return (RC_OK);
}

uchar *find_prefix_synonym(uchar *synonym) {
  LINE *curr = NULL;

  curr = first_prefix_synonym;
  while (curr != NULL) {
    if (strcmp((char *) synonym, (char *) curr->name) == 0) {
      return ((uchar *) curr->line);
    }
    curr = curr->next;
  }
  return (synonym);
}

uchar *find_prefix_oldname(uchar *oldname) {
  LINE *curr = NULL;

  curr = first_prefix_synonym;
  while (curr != NULL) {
    if (strcmp((char *) oldname, (char *) curr->line) == 0) {
      return (curr->name);
    }
    curr = curr->next;
  }
  return (oldname);
}

uchar *get_prefix_command(long prefix_index) {
  int ispf_idx = (compatible_feel == COMPAT_ISPF) ? 1 : 0;

  /*
   * Just return a pointer to the prefix command associated with the supplied index.
   */
  return (pc[ispf_idx][prefix_index].cmd);
}

static short post_prefix_add(THE_PPC *curr_ppc, short cmd_idx, long number_lines) {
  short rc = RC_OK;
  unsigned short y = 0;

  y = getcury(PENDING_WINDOW);
  if (PENDING_VIEW->current_window == WINDOW_PREFIX) {
    PENDING_VIEW->current_window = WINDOW_FILEAREA;
  }
  wmove(PENDING_WINDOW, y, 0);
  if (PENDING_VIEW->current_window == WINDOW_FILEAREA) {
    if (!VIEW_BOF(PENDING_VIEW, (PENDING_VIEW->focus_line) + 1L)) {
      rc = THEcursor_down(pending_screen, PENDING_VIEW, TRUE);
    }
  }
  return (rc);
}

