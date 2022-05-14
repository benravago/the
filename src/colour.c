/***********************************************************************/
/* COLOUR.C - Colour related functions                                 */
/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-1999 Mark Hessling
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
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 * PO Box 203, Bellara, QLD 4507, AUSTRALIA
 * Author of THE, a Free XEDIT/KEDIT editor and, Rexx/SQL
 * Maintainer of PDCurses: Public Domain Curses and, Regina Rexx interpreter
 * Use Rexx ? join the Rexx Language Association: http://www.rexxla.org
 */

static char RCSid[] = "$Id: colour.c,v 1.1 1999/06/25 06:11:56 mark Exp mark $";

#include <the.h>
#include <proto.h>

/*
 *                        attributes
 *             FILEAREA   CURLINE  BLOCK    CBLOCK
 *             CMDLINE    IDLINE   MSGLINE  ARROW
 *             PREFIX     PENDING  SCALE    TOFEOF
 *             CTOFEOF    TABLINE  SHADOW   STATAREA
 *             DIVIDER    RESERVED NONDISP  HIGHLIGHT
 *             CHIGHLIGHT SLK      GAP      ALERT
 *             DIALOG
 */

#ifdef A_COLOR
 static chtype the_fore[ATTR_MAX] =
              {COLOR_WHITE,COLOR_WHITE,COLOR_BLUE,COLOR_RED,
               COLOR_BLACK,COLOR_BLUE,COLOR_RED,COLOR_BLACK,
               COLOR_BLACK,COLOR_RED,COLOR_YELLOW,COLOR_WHITE,
               COLOR_WHITE,COLOR_YELLOW,COLOR_RED,COLOR_BLUE,
               COLOR_RED,COLOR_WHITE,COLOR_MAGENTA,COLOR_WHITE,
               COLOR_YELLOW,COLOR_BLACK,COLOR_BLACK,COLOR_WHITE,
               COLOR_BLACK};
 static chtype the_back[ATTR_MAX] =
              {COLOR_BLUE,COLOR_BLUE,COLOR_WHITE,COLOR_WHITE,
               COLOR_CYAN,COLOR_WHITE,COLOR_WHITE,COLOR_CYAN,
               COLOR_CYAN,COLOR_WHITE,COLOR_BLUE,COLOR_BLUE,
               COLOR_BLUE,COLOR_BLUE,COLOR_WHITE,COLOR_WHITE,
               COLOR_WHITE,COLOR_BLACK,COLOR_CYAN,COLOR_CYAN,
               COLOR_CYAN,COLOR_CYAN,COLOR_CYAN,COLOR_RED,
               COLOR_WHITE};
 static chtype the_mod[ATTR_MAX] =
              {A_NORMAL,A_BOLD,  A_NORMAL,A_NORMAL,
               A_NORMAL,A_NORMAL,A_NORMAL,A_NORMAL,
               A_NORMAL,A_NORMAL,A_BOLD,  A_BOLD,
               A_BOLD,  A_BOLD,  A_NORMAL,A_NORMAL,
               A_NORMAL,A_NORMAL,A_BLINK, A_BOLD,
               A_BOLD,  A_BOLD,  A_NORMAL,A_NORMAL,
               A_NORMAL};
 static chtype kedit_fore[ATTR_MAX] =
              {COLOR_CYAN,COLOR_YELLOW,COLOR_CYAN,COLOR_YELLOW,
               COLOR_YELLOW,COLOR_YELLOW,COLOR_YELLOW,COLOR_YELLOW,
               COLOR_YELLOW,COLOR_WHITE,COLOR_YELLOW,COLOR_CYAN,
               COLOR_YELLOW,COLOR_YELLOW,COLOR_YELLOW,COLOR_YELLOW,
               COLOR_CYAN,COLOR_YELLOW,COLOR_MAGENTA,COLOR_WHITE,
               COLOR_YELLOW,COLOR_BLACK,COLOR_CYAN,COLOR_WHITE,
               COLOR_BLACK};
 static chtype kedit_back[ATTR_MAX] =
              {COLOR_BLUE,COLOR_BLUE,COLOR_WHITE,COLOR_WHITE,
               COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,
               COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,
               COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,
               COLOR_BLUE,COLOR_BLUE,COLOR_CYAN,COLOR_CYAN,
               COLOR_CYAN,COLOR_CYAN,COLOR_BLUE,COLOR_RED,
               COLOR_WHITE};
 static chtype kedit_mod[ATTR_MAX]  =
              {A_BOLD,  A_BOLD,  A_BOLD,  A_BOLD,
               A_BOLD,  A_BOLD,  A_BOLD,  A_BOLD,
               A_NORMAL,A_BOLD,  A_BOLD,  A_BOLD,
               A_BOLD,  A_BOLD,  A_NORMAL,A_BOLD,
               A_BOLD,  A_BOLD,  A_BLINK, A_BOLD,
               A_BOLD,  A_BOLD,  A_BOLD,  A_NORMAL,
               A_NORMAL};
 static chtype keditw_fore[ATTR_MAX] =
              {COLOR_BLACK,COLOR_BLUE,COLOR_WHITE,COLOR_YELLOW,
               COLOR_BLACK,COLOR_BLUE,COLOR_RED,COLOR_BLUE,
               COLOR_BLUE,COLOR_RED,COLOR_BLUE,COLOR_BLUE,
               COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_WHITE,
               COLOR_BLUE,COLOR_BLUE,COLOR_RED,COLOR_BLACK,
               COLOR_GREEN,COLOR_BLACK,COLOR_BLACK,COLOR_WHITE,
               COLOR_BLACK};
 static chtype keditw_back[ATTR_MAX] =
              {COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,COLOR_BLACK,
               COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,
               COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,
               COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_CYAN,
               COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_YELLOW,
               COLOR_YELLOW,COLOR_CYAN,COLOR_WHITE,COLOR_RED,
               COLOR_WHITE};
 static chtype keditw_mod[ATTR_MAX]  =
              {A_NORMAL,A_NORMAL,A_BOLD,  A_NORMAL,
               A_NORMAL,A_NORMAL,A_BOLD,  A_NORMAL,
               A_NORMAL,A_NORMAL,A_NORMAL,A_NORMAL,
               A_NORMAL,A_NORMAL,A_NORMAL,A_BOLD,
               A_NORMAL,A_NORMAL,A_BLINK, A_BOLD,
               A_NORMAL,A_NORMAL,A_NORMAL,A_BOLD,
               A_NORMAL};
 static chtype xedit_fore[ATTR_MAX] =
              {COLOR_GREEN, COLOR_CYAN, COLOR_BLACK,COLOR_CYAN,
               COLOR_YELLOW,COLOR_CYAN, COLOR_RED,  COLOR_CYAN,
               COLOR_GREEN, COLOR_GREEN,COLOR_CYAN, COLOR_GREEN,
               COLOR_GREEN, COLOR_GREEN,COLOR_GREEN,COLOR_CYAN,
               COLOR_GREEN, COLOR_GREEN,COLOR_MAGENTA,COLOR_WHITE,
               COLOR_YELLOW,COLOR_BLACK,COLOR_GREEN,COLOR_WHITE,
               COLOR_BLACK};
 static chtype xedit_back[ATTR_MAX] =
              {COLOR_BLACK,COLOR_BLACK,COLOR_GREEN,COLOR_GREEN,
               COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,
               COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,
               COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,
               COLOR_BLACK,COLOR_BLACK,COLOR_CYAN,COLOR_CYAN,
               COLOR_CYAN,COLOR_GREEN,COLOR_BLACK,COLOR_RED,
               COLOR_WHITE};
 static chtype xedit_mod[ATTR_MAX]  =
              {A_NORMAL,A_NORMAL,A_NORMAL,A_BOLD,
               A_NORMAL,A_NORMAL,A_BOLD,  A_NORMAL,
               A_NORMAL,A_BOLD,  A_NORMAL,A_NORMAL,
               A_BOLD,  A_BOLD,  A_NORMAL,A_NORMAL,
               A_BOLD,  A_BOLD,  A_BLINK, A_BOLD,
               A_BOLD,  A_BOLD,  A_NORMAL,A_NORMAL,
               A_NORMAL};
#else
 static chtype the_fore[ATTR_MAX]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype the_back[ATTR_MAX]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype the_mod[ATTR_MAX]    = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype kedit_fore[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype kedit_back[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype kedit_mod[ATTR_MAX]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype keditw_fore[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype keditw_back[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype keditw_mod[ATTR_MAX]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype xedit_fore[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype xedit_back[ATTR_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static chtype xedit_mod[ATTR_MAX]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
 static chtype the_mono[ATTR_MAX] =
              {A_NORMAL,A_BOLD,A_REVERSE,A_BOLD|A_REVERSE,
               A_BOLD,A_REVERSE,A_BLINK,A_BOLD,
               A_REVERSE,A_BOLD|A_REVERSE,A_BOLD,A_BOLD,
               A_BOLD,A_BOLD,A_BOLD,A_REVERSE,
               A_BOLD,A_BOLD,A_BLINK|A_REVERSE,A_REVERSE,
               A_BOLD|A_REVERSE,A_BOLD|A_REVERSE,A_REVERSE,A_REVERSE,
               A_NORMAL};
 static chtype kedit_mono[ATTR_MAX] =
              {A_NORMAL,A_BOLD,  A_REVERSE,A_REVERSE|A_BOLD,
               A_NORMAL,A_NORMAL,A_BOLD,  A_BOLD,
               A_NORMAL,A_BOLD,  A_NORMAL,A_NORMAL,
               A_BOLD,  A_NORMAL,A_NORMAL,A_BOLD,
               A_NORMAL,A_NORMAL,A_BLINK|A_REVERSE,A_REVERSE,
               A_BOLD|A_REVERSE,A_BOLD|A_REVERSE,A_NORMAL,A_REVERSE,
               A_NORMAL};
 static chtype keditw_mono[ATTR_MAX] =
              {A_NORMAL,A_BOLD,  A_REVERSE,A_REVERSE|A_BOLD,
               A_NORMAL,A_NORMAL,A_BOLD,  A_BOLD,
               A_NORMAL,A_BOLD,  A_NORMAL,A_NORMAL,
               A_BOLD,  A_NORMAL,A_NORMAL,A_BOLD,
               A_NORMAL,A_NORMAL,A_BLINK|A_REVERSE,A_REVERSE,
               A_BOLD|A_REVERSE,A_BOLD|A_REVERSE,A_NORMAL,A_REVERSE,
               A_NORMAL};
 static chtype xedit_mono[ATTR_MAX] =
              {A_NORMAL,A_BOLD,  A_REVERSE,A_BOLD|A_REVERSE,
               A_NORMAL,A_BOLD,  A_BOLD,  A_BOLD,
               A_NORMAL,A_BOLD,  A_BOLD,  A_NORMAL,
               A_BOLD,  A_BOLD,  A_NORMAL,A_BOLD,
               A_BOLD,  A_BOLD,  A_BLINK|A_REVERSE,A_REVERSE,
               A_BOLD|A_REVERSE,A_BOLD|A_REVERSE,A_NORMAL,A_REVERSE,
               A_NORMAL};

/* A - comments */
/* B - strings */
/* C - numbers */
/* D - keywords */
/* E - labels */
/* F - preprocessor directives */
/* G - header lines */
/* H - extra right paren, matchable keyword */
/* I - level 1 paren */
/* J - level 1 matchable keyword */
/* K - level 1 matchable preprocessor keyword */
/* L - level 2 paren, matchable keyword */
/* M - level 3 paren, matchable keyword */
/* N - level 4 paren, matchable keyword */
/* O - level 5 paren, matchable keyword */
/* P - level 6 paren, matchable keyword */
/* Q - level 7 paren, matchable keyword */
/* R - level 8 paren or higher, matchable keyword */
/* S - incomplete string */
/* T - HTML markup tags */
/* U - HTML character/entity references */
/* V - Builtin functions */
/* W - not used */
/* X - not used */
/* Y - not used */
/* Z - not used */
/* 1 - alternate keyword color 1 */
/* 2 - alternate keyword color 2 */
/* 3 - alternate keyword color 3 */
/* 4 - alternate keyword color 4 */
/* 5 - alternate keyword color 5 */
/* 6 - alternate keyword color 6 */
/* 7 - alternate keyword color 7 */
/* 8 - alternate keyword color 8 */
/* 9 - alternate keyword color 9 */

 static ECOLOURS the_ecolours[ECOLOUR_MAX] =
 {
  /* foreground   background   modifier  mono */
  {COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* A */
  {COLOR_YELLOW,  COLOR_BLUE , A_NORMAL, A_NORMAL}, /* B */
  {COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* C */
  {COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* D */
  {COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* E */
  {COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* F */
  {COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* G */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* H */
  {COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* I */
  {COLOR_BLUE,    COLOR_BLUE , A_BOLD  , A_NORMAL}, /* J */
  {COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* K */
  {COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* L */
  {COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* M */
  {COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* N */
  {COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* O */
  {COLOR_WHITE,   COLOR_BLUE , A_BOLD,   A_NORMAL}, /* P */
  {COLOR_BLUE,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Q */
  {COLOR_MAGENTA, COLOR_BLUE , A_BOLD  , A_NORMAL}, /* R */
  {COLOR_MAGENTA, COLOR_BLUE , A_BOLD  , A_NORMAL}, /* S */
  {COLOR_CYAN,    COLOR_BLUE , A_BOLD  , A_NORMAL}, /* T */
  {COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* U */
  {COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* V */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* W */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* X */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Y */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Z */
  {COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* 1 */
  {COLOR_BLUE,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 2 */
  {COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 3 */
  {COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 4 */
  {COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 5 */
  {COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 6 */
  {COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* 7 */
  {COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 8 */
  {COLOR_BLUE,    COLOR_BLUE , A_BOLD ,  A_NORMAL}, /* 9 */
 };

 static ECOLOURS xedit_ecolours[ECOLOUR_MAX] =
 {
  /* foreground   background   modifier  mono */
  {COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* A */
  {COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* B */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* C */
  {COLOR_BLUE,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* D */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* E */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* F */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* G */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* H */
  {COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* I */
  {COLOR_BLUE,    COLOR_BLACK, A_BOLD  , A_NORMAL}, /* J */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* K */
  {COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* L */
  {COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* M */
  {COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* N */
  {COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* O */
  {COLOR_WHITE,   COLOR_BLACK, A_BOLD,   A_NORMAL}, /* P */
  {COLOR_BLUE,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Q */
  {COLOR_MAGENTA, COLOR_BLACK, A_BOLD  , A_NORMAL}, /* R */
  {COLOR_MAGENTA, COLOR_BLACK, A_BOLD  , A_NORMAL}, /* S */
  {COLOR_BLUE,    COLOR_BLACK, A_BOLD  , A_NORMAL}, /* T */
  {COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* U */
  {COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* V */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* W */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* X */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Y */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Z */
  {COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* 1 */
  {COLOR_BLUE,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 2 */
  {COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 3 */
  {COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 4 */
  {COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 5 */
  {COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 6 */
  {COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* 7 */
  {COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 8 */
  {COLOR_BLUE,    COLOR_BLACK, A_BOLD ,  A_NORMAL}, /* 9 */
 };

 static ECOLOURS kedit_ecolours[ECOLOUR_MAX] =
 {
  /* foreground   background   modifier  mono */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* A */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* B */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* C */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* D */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* E */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* F */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* G */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* H */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* I */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD  , A_NORMAL}, /* J */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* K */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* L */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* M */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* N */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* O */
  {COLOR_WHITE,   COLOR_WHITE, A_BOLD,   A_NORMAL}, /* P */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Q */
  {COLOR_MAGENTA, COLOR_WHITE, A_BOLD  , A_NORMAL}, /* R */
  {COLOR_MAGENTA, COLOR_WHITE, A_BOLD  , A_NORMAL}, /* S */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD  , A_NORMAL}, /* T */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* U */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* V */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* W */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* X */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Y */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Z */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* 1 */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 2 */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 3 */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 4 */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 5 */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 6 */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* 7 */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 8 */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD ,  A_NORMAL}, /* 9 */
 };

 static ECOLOURS keditw_ecolours[ECOLOUR_MAX] =
 {
  /* foreground   background   modifier  mono */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* A */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* B */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* C */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* D */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* E */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* F */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* G */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* H */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* I */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD  , A_NORMAL}, /* J */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* K */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* L */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* M */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* N */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* O */
  {COLOR_WHITE,   COLOR_WHITE, A_BOLD,   A_NORMAL}, /* P */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Q */
  {COLOR_MAGENTA, COLOR_WHITE, A_BOLD  , A_NORMAL}, /* R */
  {COLOR_MAGENTA, COLOR_WHITE, A_BOLD  , A_NORMAL}, /* S */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD  , A_NORMAL}, /* T */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* U */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* V */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* W */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* X */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Y */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* Z */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* 1 */
  {COLOR_BLUE,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 2 */
  {COLOR_RED,     COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 3 */
  {COLOR_MAGENTA, COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 4 */
  {COLOR_GREEN,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 5 */
  {COLOR_CYAN,    COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 6 */
  {COLOR_RED,     COLOR_WHITE, A_BOLD  , A_NORMAL}, /* 7 */
  {COLOR_BLACK,   COLOR_WHITE, A_NORMAL, A_NORMAL}, /* 8 */
  {COLOR_BLUE,    COLOR_WHITE, A_BOLD ,  A_NORMAL}, /* 9 */
 };

 struct attributes
 {
  CHARTYPE *attrib;
  short attrib_min_len;
  chtype actual_attrib;
  chtype colour_modifier;
  bool attrib_modifier;
  bool attrib_allowed_on_mono;
 };
 typedef struct attributes ATTRIBS;
#define NO_ATTRIBS 22
 static ATTRIBS valid_attribs[NO_ATTRIBS] =
 {
  {(CHARTYPE *)"black",3,COLOR_BLACK,0,FALSE,TRUE},
#if 1
  {(CHARTYPE *)"white",1,COLOR_WHITE,A_BOLD,FALSE,TRUE},
  {(CHARTYPE *)"grey",3,COLOR_WHITE,0,FALSE,FALSE},
  {(CHARTYPE *)"gray",3,COLOR_WHITE,0,FALSE,FALSE},
#else
  {(CHARTYPE *)"white",1,COLOR_WHITE,0,FALSE,TRUE},
  {(CHARTYPE *)"grey",3,COLOR_BLACK,A_BOLD,FALSE,FALSE},
  {(CHARTYPE *)"gray",3,COLOR_BLACK,A_BOLD,FALSE,FALSE},
#endif
  {(CHARTYPE *)"blue",3,COLOR_BLUE,0,FALSE,FALSE},
  {(CHARTYPE *)"green",1,COLOR_GREEN,0,FALSE,FALSE},
  {(CHARTYPE *)"cyan",1,COLOR_CYAN,0,FALSE,FALSE},
  {(CHARTYPE *)"red",3,COLOR_RED,0,FALSE,FALSE},
  {(CHARTYPE *)"magenta",1,COLOR_MAGENTA,0,FALSE,FALSE},
  {(CHARTYPE *)"pink",1,COLOR_MAGENTA,A_BOLD,FALSE,FALSE},
  {(CHARTYPE *)"brown",1,COLOR_YELLOW,0,FALSE,FALSE},
  {(CHARTYPE *)"yellow",1,COLOR_YELLOW,A_BOLD,FALSE,FALSE},
  {(CHARTYPE *)"turquoise",1,COLOR_CYAN,0,FALSE,FALSE},
  {(CHARTYPE *)"normal",3,A_NORMAL,0,TRUE,TRUE},
  {(CHARTYPE *)"blink",3,A_BLINK,0,TRUE,TRUE},
  {(CHARTYPE *)"bold",2,A_BOLD,0,TRUE,TRUE},
  {(CHARTYPE *)"bright",3,A_BOLD,0,TRUE,TRUE},
  {(CHARTYPE *)"high",1,A_BOLD,0,TRUE,TRUE},
  {(CHARTYPE *)"reverse",3,A_REVERSE,0,TRUE,TRUE},
  {(CHARTYPE *)"underline",1,A_UNDERLINE,0,TRUE,TRUE},
  {(CHARTYPE *)"dark",4,A_NORMAL,0,TRUE,TRUE},
  {(CHARTYPE *)",",1,8,0,FALSE,TRUE},
 };

/***********************************************************************/
#ifdef HAVE_PROTO
short parse_colours(CHARTYPE *attrib,COLOUR_ATTR *pattr,CHARTYPE **rem,bool spare,bool *any_colours)
#else
short parse_colours(attrib,pattr,rem,spare,any_colours)
CHARTYPE *attrib;
COLOUR_ATTR *pattr;
CHARTYPE **rem;
bool spare;
bool *any_colours;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short num_colours=0;
 chtype mono=pattr->mono;
 chtype specified_mod=0L;
 chtype fg=FOREFROMPAIR(pattr->pair);
 chtype bg=BACKFROMPAIR(pattr->pair);
 CHARTYPE *string=NULL;
 CHARTYPE *p=NULL,*oldp=NULL;
 bool found=FALSE,any_found=FALSE;
 bool spare_pos=FALSE;
 int offset=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("colour.c:  parse_colours");
#endif
/*---------------------------------------------------------------------*/
/* Get a copy of the passed string and wreck it rather than the passed */
/* string.                                                             */
/*---------------------------------------------------------------------*/
 if ((string = (CHARTYPE *)my_strdup(attrib)) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 oldp = string;
 p = (CHARTYPE *)strtok((DEFCHAR *)string," \t");
 while(p != NULL)
   {
    found = FALSE;
    for (i=0;i<NO_ATTRIBS;i++)
       {
        if (equal(valid_attribs[i].attrib,p,valid_attribs[i].attrib_min_len))
          {
           any_found = found = TRUE;
           if (!valid_attribs[i].attrib_allowed_on_mono
           &&  !colour_support)
             {
              display_error(61,(CHARTYPE *)p,FALSE);
              (*the_free)(string);
#ifdef THE_TRACE
              trace_return();
#endif
              return(RC_INVALID_OPERAND);
             }
           if (valid_attribs[i].attrib_modifier)
             {
              if (colour_support)
                 specified_mod = (valid_attribs[i].actual_attrib==A_NORMAL)?A_NORMAL:specified_mod | valid_attribs[i].actual_attrib;
              else
                 mono = (valid_attribs[i].actual_attrib==A_NORMAL)?A_NORMAL:mono | valid_attribs[i].actual_attrib;
              offset = p-oldp+strlen((DEFCHAR *)p)+1;
              break;
             }
           else
             {
              switch(num_colours)
                {
                 case 0:
                      if (!colour_support
                      &&  valid_attribs[i].actual_attrib != COLOR_WHITE)
                        {
                         display_error(61,(CHARTYPE *)p,FALSE);
                         (*the_free)(string);
#ifdef THE_TRACE
                         trace_return();
#endif
                         return(RC_INVALID_OPERAND);
                        }
                      if (valid_attribs[i].actual_attrib != 8)
                        {
                         fg = valid_attribs[i].actual_attrib;
                         specified_mod |= valid_attribs[i].colour_modifier;
                        }
                      num_colours++;
                      offset = p-oldp+strlen((DEFCHAR *)p)+1;
                      break;
                 case 1:
                      if (!colour_support
                      &&  valid_attribs[i].actual_attrib != COLOR_BLACK)
                        {
                         display_error(61,(CHARTYPE *)p,FALSE);
                         (*the_free)(string);
#ifdef THE_TRACE
                         trace_return();
#endif
                         return(RC_INVALID_OPERAND);
                        }
                      if (valid_attribs[i].actual_attrib != 8)
                        {
                         bg = valid_attribs[i].actual_attrib;
                        }
                      num_colours++;
                      offset = p-oldp+strlen((DEFCHAR *)p)+1;
                      break;
                 default:
                      if (spare)
                        {
                         spare_pos = TRUE;
                         *rem = (CHARTYPE *)attrib+offset;
                         break;
                        }
                      display_error(1,(CHARTYPE *)p,FALSE);
                      (*the_free)(string);
#ifdef THE_TRACE
                      trace_return();
#endif
                      return(RC_INVALID_OPERAND);
                      break;
                }
              if (spare_pos)
                 break;
             }
           break;
          }
       }
    if (spare_pos && found)
       break;
    if (!found)
      {
       if (equal((CHARTYPE *)"on",p,2)
       && num_colours == 1)
          ;
       else
         {
          if (spare)
            {
             *rem = (CHARTYPE *)attrib+offset;
             break;
            }
          display_error(1,(CHARTYPE *)p,FALSE);
          (*the_free)(string);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
      }
    p = (CHARTYPE *)strtok(NULL," \t");
   }

 if (num_colours == 0)
   pattr->pair = ATTR2PAIR(COLOR_WHITE,COLOR_BLACK);
 else
   pattr->pair = ATTR2PAIR(fg,bg);
 pattr->mod = specified_mod;
 pattr->mono = mono;
 *any_colours = any_found;
 (*the_free)(string);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}

#if 0
#ifdef set_colour
# undef set_colour
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
chtype set_colour(COLOUR_ATTR *attr)
#else
chtype set_colour(attr)
COLOUR_ATTR *attr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 chtype color=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("colour.c:  set_colour");
#endif
 color = attr->mono;
#ifdef A_COLOR
 if (colour_support)
    color = (attr->pair) ? COLOR_PAIR(attr->pair) | attr->mod : attr->mod;
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(color);
}
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
chtype merge_curline_colour(COLOUR_ATTR *attr, COLOUR_ATTR *ecolour)
#else
chtype merge_curline_colour(attr, ecolour)
COLOUR_ATTR *attr,*ecolour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 chtype bg,fg,mod,pair;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("colour.c:  merge_curline_colour");
#endif
#ifdef A_COLOR
   bg = BACKFROMPAIR(attr->pair);
   fg = FOREFROMPAIR(ecolour->pair);
   mod = attr->mod | ecolour->mod;
   pair = ATTR2PAIR(fg,bg);
#else
   mod = attr->mod | ecolour->mod;
#endif
#ifdef THE_TRACE
   trace_return();
#endif
#ifdef A_COLOR
   return (COLOR_PAIR(pair) | mod);
#else
   return (mod);
#endif
}

/***********************************************************************/
#ifdef HAVE_PROTO
void set_up_default_colours(FILE_DETAILS *fd,COLOUR_ATTR *attr,int colour_num)
#else
void set_up_default_colours(fd,attr,colour_num)
FILE_DETAILS *fd;
COLOUR_ATTR *attr;
int colour_num;
#endif
/***********************************************************************/
/* This function is called as part of reading in a new file.           */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("colour.c:  set_up_default_colours");
#endif
/*---------------------------------------------------------------------*/
/* Set up default colours.                                             */
/*---------------------------------------------------------------------*/
   switch(compatible_look)
   {
      case COMPAT_THE:
         if (colour_num == ATTR_MAX)
         {
            for (i=0;i<ATTR_MAX;i++)
            {
               fd->attr[i].pair = ATTR2PAIR(the_fore[i],the_back[i]);
               fd->attr[i].mod = the_mod[i];
               fd->attr[i].mono = the_mono[i];
            }
         }
         else
         {
            attr->pair = ATTR2PAIR(the_fore[colour_num],the_back[colour_num]);
            attr->mod = the_mod[colour_num];
            attr->mono = the_mono[colour_num];
         }
         break;
      case COMPAT_XEDIT:
         if (colour_num == ATTR_MAX)
         {
            for (i=0;i<ATTR_MAX;i++)
            {
               fd->attr[i].pair = ATTR2PAIR(xedit_fore[i],xedit_back[i]);
               fd->attr[i].mod = xedit_mod[i];
               fd->attr[i].mono = xedit_mono[i];
            }
         }
         else
         {
            attr->pair = ATTR2PAIR(xedit_fore[colour_num],xedit_back[colour_num]);
            attr->mod = xedit_mod[colour_num];
            attr->mono = xedit_mono[colour_num];
         }
         break;
      case COMPAT_KEDIT:
         if (colour_num == ATTR_MAX)
         {
            for (i=0;i<ATTR_MAX;i++)
            {
               fd->attr[i].pair = ATTR2PAIR(kedit_fore[i],kedit_back[i]);
               fd->attr[i].mod = kedit_mod[i];
               fd->attr[i].mono = kedit_mono[i];
            }
         }
         else
         {
            attr->pair = ATTR2PAIR(kedit_fore[colour_num],kedit_back[colour_num]);
            attr->mod = kedit_mod[colour_num];
            attr->mono = kedit_mono[colour_num];
         }
         break;
      case COMPAT_KEDITW:
         if (colour_num == ATTR_MAX)
         {
            for (i=0;i<ATTR_MAX;i++)
            {
               fd->attr[i].pair = ATTR2PAIR(keditw_fore[i],keditw_back[i]);
               fd->attr[i].mod = keditw_mod[i];
               fd->attr[i].mono = keditw_mono[i];
            }
         }
         else
         {
            attr->pair = ATTR2PAIR(keditw_fore[colour_num],keditw_back[colour_num]);
            attr->mod = keditw_mod[colour_num];
            attr->mono = keditw_mono[colour_num];
         }
         break;
   }
#ifdef THE_TRACE
   trace_return();
#endif
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_up_default_ecolours(FILE_DETAILS *fd)
#else
void set_up_default_ecolours(fd)
FILE_DETAILS *fd;
#endif
/***********************************************************************/
/* This function is called as part of reading in a new file.           */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("colour.c:  set_up_default_ecolours");
#endif
/*---------------------------------------------------------------------*/
/* Set up default colours.                                             */
/*---------------------------------------------------------------------*/
   switch(compatible_look)
   {
      case COMPAT_THE:
         for (i=0;i<ECOLOUR_MAX;i++)
         {
            fd->ecolour[i].pair = ATTR2PAIR(the_ecolours[i].fore,the_ecolours[i].back);
            fd->ecolour[i].mod = the_ecolours[i].mod;
            fd->ecolour[i].mono = the_ecolours[i].mono;
         }
         break;
      case COMPAT_XEDIT:
         for (i=0;i<ECOLOUR_MAX;i++)
         {
            fd->ecolour[i].pair = ATTR2PAIR(xedit_ecolours[i].fore,xedit_ecolours[i].back);
            fd->ecolour[i].mod = xedit_ecolours[i].mod;
            fd->ecolour[i].mono = xedit_ecolours[i].mono;
         }
         break;
      case COMPAT_KEDIT:
         for (i=0;i<ECOLOUR_MAX;i++)
         {
            fd->ecolour[i].pair = ATTR2PAIR(kedit_ecolours[i].fore,kedit_ecolours[i].back);
            fd->ecolour[i].mod = kedit_ecolours[i].mod;
            fd->ecolour[i].mono = kedit_ecolours[i].mono;
         }
         break;
      case COMPAT_KEDITW:
         for (i=0;i<ECOLOUR_MAX;i++)
         {
            fd->ecolour[i].pair = ATTR2PAIR(keditw_ecolours[i].fore,keditw_ecolours[i].back);
            fd->ecolour[i].mod = keditw_ecolours[i].mod;
            fd->ecolour[i].mono = keditw_ecolours[i].mono;
         }
         break;
   }
#ifdef THE_TRACE
   trace_return();
#endif
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_colour_strings(COLOUR_ATTR *attr)
#else
CHARTYPE *get_colour_strings(attr)
COLOUR_ATTR *attr;
#endif
/***********************************************************************/
/* This function returns a pointer to an allocated block of memory with*/
/* textual descriptions of the colours associated with the attr.       */
/* The caller is responsible for freeing up the allocated memory.      */
/***********************************************************************/
{
#define GET_MOD 0
#define GET_FG  1
#define GET_BG  2
/*--------------------------- local data ------------------------------*/
 register int i=0,j=0;
 CHARTYPE *attr_string=NULL;
 int fg=FOREFROMPAIR(attr->pair),bg=BACKFROMPAIR(attr->pair);
 chtype mod=attr->mono;
 int start_with=0;
 bool colour_only=FALSE;
 chtype match_value=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("colour.c:  get_colour_strings");
#endif

 start_with = GET_MOD;
#ifdef A_COLOR
 if (colour_support)
   {
    start_with = GET_MOD;
    mod = attr->mod;
   }
#endif
 attr_string = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*70);
 if (attr_string == (CHARTYPE *)NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(NULL);
   }
 strcpy((DEFCHAR *)attr_string,"");
 for (j=start_with;j<3;j++)
   {
    switch(j)
      {
       case GET_FG:
            colour_only = TRUE;
            match_value = (chtype)fg;
            break;
       case GET_BG:
            strcat((DEFCHAR *)attr_string,"on ");
            colour_only = TRUE;
            match_value = (chtype)bg;
            break;
       default:
            colour_only = FALSE;
            match_value = mod;
            break;
      }
    for (i=0;i<NO_ATTRIBS;i++)
      {
       if (colour_only)
         {
          if (!valid_attribs[i].attrib_modifier
          &&  match_value == (chtype)valid_attribs[i].actual_attrib)
            {
             strcat((DEFCHAR *)attr_string,(DEFCHAR *)valid_attribs[i].attrib);
             strcat((DEFCHAR *)attr_string," ");
             break;
            }
         }
       else
         {
          if (valid_attribs[i].attrib_modifier
          &&  (match_value & valid_attribs[i].actual_attrib))
            {
             strcat((DEFCHAR *)attr_string,(DEFCHAR *)valid_attribs[i].attrib);
             strcat((DEFCHAR *)attr_string," ");
             break;
            }
         }
      }
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(attr_string);
}
