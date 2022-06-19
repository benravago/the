// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Colour related functions.
 * This file contains all commands that can be assigned to function keys or typed on the command line.
 */

#include "the.h"
#include "proto.h"

static COLOUR_DEF the_colours[ATTR_MAX] =
{
 /* foreground     background   modifier  mono                     */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* FILEAREA    */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* CURLINE     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* BLOCK       */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD|A_REVERSE         }, /* CBLOCK      */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_BOLD                   }, /* CMDLINE     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* IDLINE      */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BLINK                  }, /* MSGLINE     */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_BOLD                   }, /* ARROW       */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_REVERSE                }, /* PREFIX      */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_REVERSE                }, /* CPREFIX     */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD|A_REVERSE         }, /* PENDING     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* SCALE       */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* TOFEOF      */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* CTOFEOF     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* TABLINE     */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD                   }, /* SHADOW      */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* STATAREA    */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD                   }, /* DIVIDER     */
  { COLOR_WHITE   ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* RESERVED    */
  { COLOR_MAGENTA ,COLOR_CYAN  ,A_BLINK  ,A_BLINK|A_REVERSE        }, /* NONDISP     */
  { COLOR_WHITE   ,COLOR_CYAN  ,A_BOLD   ,A_REVERSE                }, /* HIGHLIGHT   */
  { COLOR_YELLOW  ,COLOR_CYAN  ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* CHIGHLIGHT  */
  { COLOR_WHITE   ,COLOR_RED   ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* THIGHLIGHT  */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* SLK         */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_REVERSE                }, /* GAP         */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_REVERSE                }, /* CGAP        */
  { COLOR_WHITE   ,COLOR_RED   ,A_NORMAL ,A_REVERSE                }, /* ALERT       */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL ,A_NORMAL                 }, /* DIALOG      */
  { COLOR_WHITE   ,COLOR_BLACK ,A_BOLD   ,A_NORMAL                 }, /* BOUNDMARK   */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* FILETABS    */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* FILETABSDIV */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* CURSORLINE  */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD                   }, /* DIA-BORDER    */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL ,A_BOLD                   }, /* DIA-EDITFIELD */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* DIA-BUTTON    */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD|A_REVERSE         }, /* DIA-ABUTTON   */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL ,A_BOLD                   }, /* POP-BORDER    */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* POP-CURLINE   */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* POPUP         */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL ,A_REVERSE                }, /* POP-DIVIDER   */
};

static COLOUR_DEF kedit_colours[ATTR_MAX] =
{
 /* foreground     background   modifier  mono                     */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* FILEAREA    */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* CURLINE     */
  { COLOR_CYAN    ,COLOR_WHITE ,A_BOLD   ,A_REVERSE                }, /* BLOCK       */
  { COLOR_YELLOW  ,COLOR_WHITE ,A_BOLD   ,A_REVERSE|A_BOLD         }, /* CBLOCK      */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* CMDLINE     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* IDLINE      */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* MSGLINE     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* ARROW       */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* PREFIX      */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* CPREFIX     */
  { COLOR_WHITE   ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* PENDING     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* SCALE       */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* TOFEOF      */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* CTOFEOF     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* TABLINE     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_NORMAL ,A_NORMAL                 }, /* SHADOW      */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* STATAREA    */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* DIVIDER     */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* RESERVED    */
  { COLOR_MAGENTA ,COLOR_CYAN  ,A_BLINK  ,A_BLINK|A_REVERSE        }, /* NONDISP     */
  { COLOR_WHITE   ,COLOR_CYAN  ,A_BOLD   ,A_REVERSE                }, /* HIGHLIGHT   */
  { COLOR_YELLOW  ,COLOR_CYAN  ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* CHIGHLIGHT  */
  { COLOR_WHITE   ,COLOR_RED   ,A_NORMAL ,A_REVERSE                }, /* THIGHLIGHT  */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* SLK         */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* GAP         */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* CGAP        */
  { COLOR_WHITE   ,COLOR_RED   ,A_NORMAL ,A_REVERSE                }, /* ALERT       */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL ,A_NORMAL                 }, /* DIALOG      */
  { COLOR_WHITE   ,COLOR_BLACK ,A_BOLD   ,A_NORMAL                 }, /* BOUNDMARK   */
  { COLOR_YELLOW  ,COLOR_BLACK ,A_BOLD   ,A_NORMAL                 }, /* FILETABS    */
  { COLOR_WHITE   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* FILETABSDIV */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* CURSORLINE   */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* DIA-BORDER    */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* DIA-EDITFIELD */
  { COLOR_CYAN    ,COLOR_WHITE ,A_BOLD   ,A_REVERSE                }, /* DIA-BUTTON    */
  { COLOR_YELLOW  ,COLOR_WHITE ,A_BOLD   ,A_REVERSE|A_BOLD         }, /* DIA-ABUTTON   */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* POP-BORDER    */
  { COLOR_CYAN    ,COLOR_BLUE  ,A_BOLD   ,A_NORMAL                 }, /* POP-CURLINE   */
  { COLOR_CYAN    ,COLOR_WHITE ,A_BOLD   ,A_REVERSE                }, /* POPUP         */
  { COLOR_YELLOW  ,COLOR_BLUE  ,A_BOLD   ,A_BOLD                   }, /* POP-DIVIDER   */
};

static COLOUR_DEF keditw_colours[ATTR_MAX] =
{
 /* foreground     background   modifier          mono                     */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* FILEAREA    */
  { COLOR_BLACK   ,COLOR_WHITE ,A_BOLD   |A_BLINK,A_BOLD                   }, /* CURLINE     */
  { COLOR_WHITE   ,COLOR_BLACK ,A_NORMAL |A_BOLD ,A_REVERSE                }, /* BLOCK       */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL         ,A_REVERSE|A_BOLD         }, /* CBLOCK      */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* CMDLINE     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* IDLINE      */
  { COLOR_BLACK   ,COLOR_CYAN  ,          A_BLINK,A_BOLD                   }, /* MSGLINE     */
  { COLOR_CYAN    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_BOLD                   }, /* ARROW       */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* PREFIX      */
  { COLOR_CYAN    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* CPREFIX     */
  { COLOR_RED     ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_BOLD                   }, /* PENDING     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* SCALE       */
  { COLOR_GREEN   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* TOFEOF      */
  { COLOR_GREEN   ,COLOR_WHITE ,A_BOLD   |A_BLINK,A_BOLD                   }, /* CTOFEOF     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* TABLINE     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_BOLD   |A_BLINK,A_NORMAL                 }, /* SHADOW      */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_BOLD                   }, /* STATAREA    */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* DIVIDER     */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* RESERVED    */
  { COLOR_MAGENTA ,COLOR_WHITE ,A_NORMAL         ,A_BLINK|A_REVERSE        }, /* NONDISP     */
  { COLOR_BLACK   ,COLOR_YELLOW,A_BOLD   |A_BLINK,A_REVERSE                }, /* HIGHLIGHT   */
  { COLOR_GREEN   ,COLOR_YELLOW,A_NORMAL         ,A_BOLD|A_REVERSE         }, /* CHIGHLIGHT  */
  { COLOR_BLACK   ,COLOR_GREEN ,A_NORMAL |A_BLINK,A_BOLD                   }, /* THIGHLIGHT  */
  { COLOR_BLACK   ,COLOR_CYAN  ,A_NORMAL         ,A_BOLD|A_REVERSE         }, /* SLK         */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* GAP         */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* CGAP        */
  { COLOR_WHITE   ,COLOR_RED   ,A_BOLD           ,A_REVERSE                }, /* ALERT       */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* DIALOG      */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* BOUNDMARK   */
  { COLOR_MAGENTA ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* FILETABS    */
  { COLOR_YELLOW  ,COLOR_WHITE ,A_BOLD           ,A_NORMAL                 }, /* FILETABSDIV */
  { COLOR_BLACK   ,COLOR_WHITE ,A_BOLD   |A_BLINK,A_NORMAL                 }, /* CURSORLINE   */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* DIA-BORDER    */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* DIA-EDITFIELD */
  { COLOR_WHITE   ,COLOR_BLACK ,A_NORMAL |A_BOLD ,A_REVERSE                }, /* DIA-BUTTON    */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL         ,A_REVERSE|A_BOLD         }, /* DIA-ABUTTON   */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_NORMAL                 }, /* POP-BORDER    */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL |A_BLINK,A_NORMAL                 }, /* POP-CURLINE   */
  { COLOR_WHITE   ,COLOR_BLACK ,A_NORMAL |A_BOLD ,A_REVERSE                }, /* POPUP         */
  { COLOR_BLUE    ,COLOR_WHITE ,A_NORMAL         ,A_BOLD                   }, /* POP-DIVIDER   */
};

static COLOUR_DEF xedit_colours[ATTR_MAX] =
{
 /* foreground     background   modifier  mono                     */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* FILEAREA    */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* CURLINE     */
  { COLOR_BLACK   ,COLOR_GREEN ,A_NORMAL ,A_REVERSE                }, /* BLOCK       */
  { COLOR_CYAN    ,COLOR_GREEN ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* CBLOCK      */
  { COLOR_YELLOW  ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* CMDLINE     */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* IDLINE      */
  { COLOR_RED     ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* MSGLINE     */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* ARROW       */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* PREFIX      */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* CPREFIX     */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* PENDING     */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* SCALE       */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* TOFEOF      */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* CTOFEOF     */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* TABLINE     */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* SHADOW      */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* STATAREA    */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* DIVIDER     */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* RESERVED    */
  { COLOR_MAGENTA ,COLOR_CYAN  ,A_BLINK  ,A_BLINK|A_REVERSE        }, /* NONDISP     */
  { COLOR_WHITE   ,COLOR_CYAN  ,A_BOLD   ,A_REVERSE                }, /* HIGHLIGHT   */
  { COLOR_YELLOW  ,COLOR_CYAN  ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* CHIGHLIGHT  */
  { COLOR_WHITE   ,COLOR_RED   ,A_NORMAL ,A_REVERSE                }, /* THIGHLIGHT  */
  { COLOR_BLACK   ,COLOR_GREEN ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* SLK         */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* GAP         */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* CGAP        */
  { COLOR_WHITE   ,COLOR_RED   ,A_NORMAL ,A_REVERSE                }, /* ALERT       */
  { COLOR_BLACK   ,COLOR_WHITE ,A_NORMAL ,A_NORMAL                 }, /* DIALOG      */
  { COLOR_WHITE   ,COLOR_BLACK ,A_BOLD   ,A_NORMAL                 }, /* BOUNDMARK   */
  { COLOR_GREEN   ,COLOR_WHITE ,A_BOLD   ,A_NORMAL                 }, /* FILETABS    */
  { COLOR_GREEN   ,COLOR_WHITE ,A_NORMAL ,A_NORMAL                 }, /* FILETABSDIV */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* CURSORLINE   */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* DIA-BORDER    */
  { COLOR_YELLOW  ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* DIA-EDITFIELD */
  { COLOR_BLACK   ,COLOR_GREEN ,A_NORMAL ,A_REVERSE                }, /* DIA-BUTTON    */
  { COLOR_CYAN    ,COLOR_GREEN ,A_BOLD   ,A_BOLD|A_REVERSE         }, /* DIA-ABUTTON   */
  { COLOR_GREEN   ,COLOR_BLACK ,A_BOLD   ,A_BOLD                   }, /* POP-BORDER    */
  { COLOR_GREEN   ,COLOR_BLACK ,A_NORMAL ,A_NORMAL                 }, /* POP-CURLINE   */
  { COLOR_BLACK   ,COLOR_GREEN ,A_NORMAL ,A_REVERSE                }, /* POPUP         */
  { COLOR_CYAN    ,COLOR_BLACK ,A_NORMAL ,A_BOLD                   }, /* POP-DIVIDER   */
};

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
/* W - directory */
/* X - link */
/* Y - extensions */
/* Z - executables */
/* 1 - alternate keyword color 1 */
/* 2 - alternate keyword color 2 */
/* 3 - alternate keyword color 3 */
/* 4 - alternate keyword color 4 */
/* 5 - alternate keyword color 5 */
/* 6 - alternate keyword color 6 */
/* 7 - alternate keyword color 7 */
/* 8 - alternate keyword color 8 */
/* 9 - alternate keyword color 9 */

static COLOUR_DEF the_ecolours[ECOLOUR_MAX] =
{
 /* foreground     background   modifier  mono */
  { COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* A */
  { COLOR_YELLOW,  COLOR_BLUE , A_NORMAL, A_NORMAL}, /* B */
  { COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* C */
  { COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* D */
  { COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* E */
  { COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* F */
  { COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* G */
  { COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* H */
  { COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* I */
  { COLOR_BLUE,    COLOR_BLUE , A_BOLD  , A_NORMAL}, /* J */
  { COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* K */
  { COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* L */
  { COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* M */
  { COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* N */
  { COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* O */
  { COLOR_WHITE,   COLOR_BLUE , A_BOLD,   A_NORMAL}, /* P */
  { COLOR_BLUE,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Q */
  { COLOR_MAGENTA, COLOR_BLUE , A_BOLD  , A_NORMAL}, /* R */
  { COLOR_MAGENTA, COLOR_BLUE , A_BOLD  , A_NORMAL}, /* S */
  { COLOR_CYAN,    COLOR_BLUE , A_BOLD  , A_NORMAL}, /* T */
  { COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* U */
  { COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* V */
  { COLOR_YELLOW,  COLOR_BLUE , A_BOLD,   A_NORMAL}, /* W */
  { COLOR_YELLOW,  COLOR_BLUE , A_NORMAL, A_NORMAL}, /* X */
  { COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Y */
  { COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* Z */
  { COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* 1 */
  { COLOR_YELLOW,  COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 2 */
  { COLOR_RED,     COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 3 */
  { COLOR_MAGENTA, COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 4 */
  { COLOR_GREEN,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 5 */
  { COLOR_CYAN,    COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 6 */
  { COLOR_RED,     COLOR_BLUE , A_BOLD  , A_NORMAL}, /* 7 */
  { COLOR_BLACK,   COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 8 */
  { COLOR_BLUE,    COLOR_BLUE , A_BOLD ,  A_NORMAL}, /* 9 */
};

static COLOUR_DEF xedit_ecolours[ECOLOUR_MAX] =
{
 /* foreground     background   modifier  mono */
  { COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* A */
  { COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* B */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* C */
  { COLOR_YELLOW,  COLOR_BLACK, A_NORMAL, A_NORMAL}, /* D */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* E */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* F */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* G */
  { COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* H */
  { COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* I */
  { COLOR_BLUE,    COLOR_BLACK, A_BOLD  , A_NORMAL}, /* J */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* K */
  { COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* L */
  { COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* M */
  { COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* N */
  { COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* O */
  { COLOR_WHITE,   COLOR_BLACK, A_BOLD,   A_NORMAL}, /* P */
  { COLOR_BLUE,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Q */
  { COLOR_MAGENTA, COLOR_BLACK, A_BOLD  , A_NORMAL}, /* R */
  { COLOR_MAGENTA, COLOR_BLACK, A_BOLD  , A_NORMAL}, /* S */
  { COLOR_BLUE,    COLOR_BLACK, A_BOLD  , A_NORMAL}, /* T */
  { COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* U */
  { COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* V */
  { COLOR_YELLOW,  COLOR_BLACK, A_BOLD  , A_NORMAL}, /* W */
  { COLOR_YELLOW,  COLOR_BLACK, A_NORMAL, A_NORMAL}, /* X */
  { COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Y */
  { COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* Z */
  { COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* 1 */
  { COLOR_BLUE,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 2 */
  { COLOR_RED,     COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 3 */
  { COLOR_MAGENTA, COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 4 */
  { COLOR_GREEN,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 5 */
  { COLOR_CYAN,    COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 6 */
  { COLOR_RED,     COLOR_BLACK, A_BOLD  , A_NORMAL}, /* 7 */
  { COLOR_BLACK,   COLOR_BLACK, A_NORMAL, A_NORMAL}, /* 8 */
  { COLOR_BLUE,    COLOR_BLACK, A_BOLD ,  A_NORMAL}, /* 9 */
};

static COLOUR_DEF kedit_ecolours[ECOLOUR_MAX] =
{
 /* foreground     background   modifier  mono */
  { COLOR_GREEN,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* A */
  { COLOR_CYAN,    COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* B */
  { COLOR_RED,     COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* C */
  { COLOR_BLUE,    COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* D */
  { COLOR_RED,     COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* E */
  { COLOR_RED,     COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* F */
  { COLOR_RED,     COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* G */
  { COLOR_BLACK,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* H */
  { COLOR_GREEN,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* I */
  { COLOR_BLUE,    COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* J */
  { COLOR_RED,     COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* K */
  { COLOR_GREEN,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* L */
  { COLOR_RED,     COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* M */
  { COLOR_CYAN,    COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* N */
  { COLOR_MAGENTA, COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* O */
  { COLOR_WHITE,   COLOR_BLUE,  A_BOLD,   A_NORMAL}, /* P */
  { COLOR_BLUE,    COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* Q */
  { COLOR_MAGENTA, COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* R */
  { COLOR_MAGENTA, COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* S */
  { COLOR_BLUE,    COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* T */
  { COLOR_RED,     COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* U */
  { COLOR_MAGENTA, COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* V */
  { COLOR_YELLOW,  COLOR_BLUE,  A_BOLD,   A_NORMAL}, /* W */
  { COLOR_YELLOW,  COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* X */
  { COLOR_BLACK,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* Y */
  { COLOR_BLACK,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* Z */
  { COLOR_RED,     COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* 1 */
  { COLOR_YELLOW,  COLOR_BLUE , A_NORMAL, A_NORMAL}, /* 2 */
  { COLOR_RED,     COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* 3 */
  { COLOR_MAGENTA, COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* 4 */
  { COLOR_GREEN,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* 5 */
  { COLOR_CYAN,    COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* 6 */
  { COLOR_RED,     COLOR_BLUE,  A_BOLD  , A_NORMAL}, /* 7 */
  { COLOR_BLACK,   COLOR_BLUE,  A_NORMAL, A_NORMAL}, /* 8 */
  { COLOR_BLUE,    COLOR_BLUE,  A_BOLD ,  A_NORMAL}, /* 9 */
};

static COLOUR_DEF keditw_ecolours[ECOLOUR_MAX] =
{
 /* foreground     background   modifier  mono */
  { COLOR_GREEN,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* A */
  { COLOR_CYAN,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* B */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* C */
  { COLOR_BLUE,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* D */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* E */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* F */
  { COLOR_RED,     COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* G */
  { COLOR_BLACK,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* H */
  { COLOR_GREEN,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* I */
  { COLOR_BLUE,    COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* J */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* K */
  { COLOR_GREEN,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* L */
  { COLOR_RED,     COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* M */
  { COLOR_CYAN,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* N */
  { COLOR_MAGENTA, COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* O */
  { COLOR_WHITE,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* P */
  { COLOR_BLUE,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* Q */
  { COLOR_MAGENTA, COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* R */
  { COLOR_MAGENTA, COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* S */
  { COLOR_BLUE,    COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* T */
  { COLOR_RED,     COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* U */
  { COLOR_MAGENTA, COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* V */
  { COLOR_YELLOW,  COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* W */
  { COLOR_GREEN,   COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* X */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* Y */
  { COLOR_BLACK,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* Z */
  { COLOR_RED,     COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* 1 */
  { COLOR_BLUE,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 2 */
  { COLOR_RED,     COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 3 */
  { COLOR_MAGENTA, COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 4 */
  { COLOR_GREEN,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 5 */
  { COLOR_CYAN,    COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 6 */
  { COLOR_YELLOW,  COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 7 */
  { COLOR_WHITE,   COLOR_WHITE, A_NORMAL|A_BLINK, A_NORMAL}, /* 8 */
  { COLOR_BLUE,    COLOR_WHITE, A_BOLD  |A_BLINK, A_NORMAL}, /* 9 */
};

struct attributes
{
  char* attrib;
  short attrib_min_len;
  int actual_attrib;
  chtype colour_modifier;
  bool attrib_modifier;
  bool attrib_allowed_on_mono;
  bool actual_colour;
};
typedef struct attributes ATTRIBS;

static ATTRIBS valid_attribs[] =
{
  { "black", 3,COLOR_BLACK,0,FALSE,TRUE,TRUE},
  { "grey", 3,COLOR_WHITE,0,FALSE,FALSE,TRUE},
  { "gray", 3,COLOR_WHITE,0,FALSE,FALSE,TRUE},
  { "white", 1,COLOR_WHITE,A_BOLD,FALSE,TRUE,TRUE},
  { "blue", 3,COLOR_BLUE,0,FALSE,FALSE,TRUE},
  { "green", 1,COLOR_GREEN,0,FALSE,FALSE,TRUE},
  { "cyan", 1,COLOR_CYAN,0,FALSE,FALSE,TRUE},
  { "red", 3,COLOR_RED,0,FALSE,FALSE,TRUE},
  { "magenta", 1,COLOR_MAGENTA,0,FALSE,FALSE,TRUE},
  { "pink", 1,COLOR_MAGENTA,A_BOLD,FALSE,FALSE,TRUE},
  { "brown", 1,COLOR_YELLOW,0,FALSE,FALSE,TRUE},
  { "yellow", 1,COLOR_YELLOW,A_BOLD,FALSE,FALSE,TRUE},
  { "turquoise", 1,COLOR_CYAN,0,FALSE,FALSE,TRUE},
  { "normal", 3,A_NORMAL,0,TRUE,TRUE,TRUE},
  { "backbold", 8,A_BLINK,0,TRUE,TRUE,FALSE},
  { "blink", 3,A_BLINK,0,TRUE,TRUE,FALSE},
  { "bold", 2,A_BOLD,0,TRUE,TRUE,FALSE},
  { "bright", 3,A_BOLD,0,TRUE,TRUE,FALSE},
  { "high", 1,A_BOLD,0,TRUE,TRUE,FALSE},
  { "reverse", 3,A_REVERSE,0,TRUE,TRUE,FALSE},
  { "underline", 1,A_UNDERLINE,0,TRUE,TRUE,FALSE},
  { "dark", 4,A_NORMAL,0,TRUE,TRUE,FALSE},
  { ", ",1,8,0,FALSE,TRUE,FALSE},
  { NULL, 0,0,0,FALSE,FALSE,FALSE},
};

short parse_colours(char* attrib, COLOUR_ATTR* pattr, char** rem, bool spare, bool* any_colours) {
  register short i = 0;
  short num_colours = 0;
  chtype mono = pattr->mono;
  chtype specified_mod = 0L;
  chtype fg = FOREFROMPAIR(pattr->pair);
  chtype bg = BACKFROMPAIR(pattr->pair);
  char* string = NULL;
  char *p = NULL, *oldp = NULL;
  bool found = FALSE, any_found = FALSE;
  bool spare_pos = FALSE;
  int offset = 0;

  /*
   * Get a copy of the passed string and wreck it rather than the passed string.
   */
  if ((string = my_strdup(attrib)) == NULL) {
    display_error(30, "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  oldp = string;
  p = strtok(string, " \t");
  while (p != NULL) {
    found = FALSE;
    for (i = 0; valid_attribs[i].attrib != NULL; i++) {
      if (equal(valid_attribs[i].attrib, p, valid_attribs[i].attrib_min_len)) {
        any_found = found = TRUE;
        if (!valid_attribs[i].attrib_allowed_on_mono && !colour_support) {
          display_error(61, p, FALSE);
          free(string);
          return (RC_INVALID_OPERAND);
        }
        if (valid_attribs[i].attrib_modifier) {
          if (colour_support) {
            specified_mod = (valid_attribs[i].actual_attrib == A_NORMAL) ? A_NORMAL : specified_mod | valid_attribs[i].actual_attrib;
          } else {
            mono = (valid_attribs[i].actual_attrib == A_NORMAL) ? A_NORMAL : mono | valid_attribs[i].actual_attrib;
          }
          offset = p - oldp + strlen(p) + 1;
          break;
        } else {
          switch (num_colours) {

            case 0:
              if (!colour_support && valid_attribs[i].actual_attrib != COLOR_WHITE) {
                display_error(61, p, FALSE);
                free(string);
                return (RC_INVALID_OPERAND);
              }
              if (valid_attribs[i].actual_attrib != 8) {
                fg = valid_attribs[i].actual_attrib;
                specified_mod |= valid_attribs[i].colour_modifier;
              }
              num_colours++;
              offset = p - oldp + strlen(p) + 1;
              break;

            case 1:
              if (!colour_support && valid_attribs[i].actual_attrib != COLOR_BLACK) {
                display_error(61, p, FALSE);
                free(string);
                return (RC_INVALID_OPERAND);
              }
              if (valid_attribs[i].actual_attrib != 8) {
                bg = valid_attribs[i].actual_attrib;
              }
              num_colours++;
              offset = p - oldp + strlen(p) + 1;
              break;

            default:
              if (spare) {
                spare_pos = TRUE;
                *rem = attrib + offset;
                break;
              }
              display_error(1, p, FALSE);
              free(string);
              return (RC_INVALID_OPERAND);
              break;
          }
          if (spare_pos) {
            break;
          }
        }
        break;
      }
    }
    if (spare_pos && found) {
      break;
    }
    if (!found) {
      if (equal("on", p, 2) && num_colours == 1) {
        // ok
      } else {
        if (spare) {
          *rem = attrib + offset;
          break;
        }
        display_error(1, p, FALSE);
        free(string);
        return (RC_INVALID_OPERAND);
      }
    }
    p = strtok(NULL, " \t");
  }

  if (num_colours == 0) {
    pattr->pair = ATTR2PAIR(COLOR_WHITE, COLOR_BLACK);
  } else {
    pattr->pair = ATTR2PAIR(fg, bg);
  }
  pattr->mod = specified_mod;
  pattr->mono = mono;
  *any_colours = any_found;
  free(string);
  return (RC_OK);
}

short parse_modifiers(char* attrib, COLOUR_ATTR* pattr) {
  register short i = 0;
  chtype mono = pattr->mono;
  chtype specified_mod = 0L;
  char* string = NULL;
  char *p = NULL, *last_word = NULL;
  bool found = FALSE;

  /*
   * Get a copy of the passed string and wreck it rather than the passed string.
   */
  if ((string = my_strdup(attrib)) == NULL) {
    display_error(30, "", FALSE);
    return (RC_OUT_OF_MEMORY);
  }
  p = strtok(string, " \t");
  while (p != NULL) {
    found = FALSE;
    for (i = 0; valid_attribs[i].attrib != NULL; i++) {
      if (equal(valid_attribs[i].attrib, p, valid_attribs[i].attrib_min_len) && valid_attribs[i].attrib_modifier) {
        found = TRUE;
        if (!valid_attribs[i].attrib_allowed_on_mono && !colour_support) {
          display_error(61, p, FALSE);
          free(string);
          return (RC_INVALID_OPERAND);
        }
        if (colour_support) {
          specified_mod = (valid_attribs[i].actual_attrib == A_NORMAL) ? A_NORMAL : specified_mod | valid_attribs[i].actual_attrib;
        } else {
          mono = (valid_attribs[i].actual_attrib == A_NORMAL) ? A_NORMAL : mono | valid_attribs[i].actual_attrib;
        }
        break;
      }
    }
    if (!found) {
      if (equal("on", p, 2) || equal("off", p, 3)) {
        last_word = p;
      } else {
        display_error(1, p, FALSE);
        free(string);
        return (RC_INVALID_OPERAND);
      }
    }
    p = strtok(NULL, " \t");
  }

  if (equal("on", last_word, 2) || equal("off", last_word, 3)) {
    free(string);
  } else {
    display_error(1, p, FALSE);
    /*
     * Free the memory after we finish referencing it; p points to somewhere in string.
     */
    free(string);
    return (RC_INVALID_OPERAND);
  }

  pattr->mod = specified_mod;
  pattr->mono = mono;
  return (RC_OK);
}

chtype merge_curline_colour(COLOUR_ATTR* attr, COLOUR_ATTR* ecolour) {
/*
 * Combines the foreground of ecolour colour with the background of the attr colour.
 * Also combines the modifiers from both colours to become the new modifier for the combined colours.
 */
  chtype bg, fg, mod, pair;

  bg = BACKFROMPAIR(attr->pair);
  fg = FOREFROMPAIR(ecolour->pair);
  mod = attr->mod | ecolour->mod;
  pair = ATTR2PAIR(fg, bg);
  fg = (COLOR_PAIR(pair) | mod);
  return fg;
}

/*
 * This function is called as part of reading in a new file.
 */
void set_up_default_colours(FILE_DETAILS* fd, COLOUR_ATTR* attr, int colour_num) {
  register short i = 0;

  /*
   * Set up default colours.
   */
  switch (compatible_look) {

    case COMPAT_THE:
      if (colour_num == ATTR_MAX) {
        for (i = 0; i < ATTR_MAX; i++) {
          fd->attr[i].pair = ATTR2PAIR(the_colours[i].fore, the_colours[i].back);
          fd->attr[i].mod = the_colours[i].mod;
          fd->attr[i].mono = the_colours[i].mono;
        }
      } else {
        attr->pair = ATTR2PAIR(the_colours[colour_num].fore, the_colours[colour_num].back);
        attr->mod = the_colours[colour_num].mod;
        attr->mono = the_colours[colour_num].mono;
      }
      break;

    case COMPAT_XEDIT:
      if (colour_num == ATTR_MAX) {
        for (i = 0; i < ATTR_MAX; i++) {
          fd->attr[i].pair = ATTR2PAIR(xedit_colours[i].fore, xedit_colours[i].back);
          fd->attr[i].mod = xedit_colours[i].mod;
          fd->attr[i].mono = xedit_colours[i].mono;
        }
      } else {
        attr->pair = ATTR2PAIR(xedit_colours[colour_num].fore, xedit_colours[colour_num].back);
        attr->mod = xedit_colours[colour_num].mod;
        attr->mono = xedit_colours[colour_num].mono;
      }
      break;

    case COMPAT_KEDIT:
      if (colour_num == ATTR_MAX) {
        for (i = 0; i < ATTR_MAX; i++) {
          fd->attr[i].pair = ATTR2PAIR(kedit_colours[i].fore, kedit_colours[i].back);
          fd->attr[i].mod = kedit_colours[i].mod;
          fd->attr[i].mono = kedit_colours[i].mono;
        }
      } else {
        attr->pair = ATTR2PAIR(kedit_colours[colour_num].fore, kedit_colours[colour_num].back);
        attr->mod = kedit_colours[colour_num].mod;
        attr->mono = kedit_colours[colour_num].mono;
      }
      break;

    case COMPAT_KEDITW:
      if (colour_num == ATTR_MAX) {
        for (i = 0; i < ATTR_MAX; i++) {
          fd->attr[i].pair = ATTR2PAIR(keditw_colours[i].fore, keditw_colours[i].back);
          fd->attr[i].mod = keditw_colours[i].mod;
          fd->attr[i].mono = keditw_colours[i].mono;
        }
      } else {
        attr->pair = ATTR2PAIR(keditw_colours[colour_num].fore, keditw_colours[colour_num].back);
        attr->mod = keditw_colours[colour_num].mod;
        attr->mono = keditw_colours[colour_num].mono;
      }
      break;
  }
  return;
}

/*
 * This function is called as part of reading in a new file.
 */
void set_up_default_ecolours(FILE_DETAILS* fd) {
  register short i = 0;

  /*
   * Set up default colours.
   */
  switch (compatible_look) {

    case COMPAT_THE:
      for (i = 0; i < ECOLOUR_MAX; i++) {
        fd->ecolour[i].pair = ATTR2PAIR(the_ecolours[i].fore, the_ecolours[i].back);
        fd->ecolour[i].mod = the_ecolours[i].mod;
        fd->ecolour[i].mono = the_ecolours[i].mono;
      }
      break;

    case COMPAT_XEDIT:
      for (i = 0; i < ECOLOUR_MAX; i++) {
        fd->ecolour[i].pair = ATTR2PAIR(xedit_ecolours[i].fore, xedit_ecolours[i].back);
        fd->ecolour[i].mod = xedit_ecolours[i].mod;
        fd->ecolour[i].mono = xedit_ecolours[i].mono;
      }
      break;

    case COMPAT_KEDIT:
      for (i = 0; i < ECOLOUR_MAX; i++) {
        fd->ecolour[i].pair = ATTR2PAIR(kedit_ecolours[i].fore, kedit_ecolours[i].back);
        fd->ecolour[i].mod = kedit_ecolours[i].mod;
        fd->ecolour[i].mono = kedit_ecolours[i].mono;
      }
      break;

    case COMPAT_KEDITW:
      for (i = 0; i < ECOLOUR_MAX; i++) {
        fd->ecolour[i].pair = ATTR2PAIR(keditw_ecolours[i].fore, keditw_ecolours[i].back);
        fd->ecolour[i].mod = keditw_ecolours[i].mod;
        fd->ecolour[i].mono = keditw_ecolours[i].mono;
      }
      break;
  }
  return;
}

#define GET_MOD 0
#define GET_FG  1
#define GET_BG  2

/*
 * This function returns a pointer to an allocated block of memory with textual descriptions of the colours associated with the attr.
 * The caller is responsible for freeing up the allocated memory.
 */
char* get_colour_strings(COLOUR_ATTR* attr) {
  register int i = 0, j = 0;
  char* attr_string = NULL;
  int fg = FOREFROMPAIR(attr->pair), bg = BACKFROMPAIR(attr->pair);
  chtype mod = attr->mono;
  int start_with = 0;
  bool colour_only = FALSE;
  chtype match_value = 0L;
  chtype matched_modifiers = 0L;

  start_with = GET_MOD;
  if (colour_support) {
    start_with = GET_MOD;
    mod = attr->mod;
  }
  attr_string = malloc(sizeof(char) * 70);
  if (attr_string == NULL) {
    display_error(30, "", FALSE);
    return (NULL);
  }
  strcpy(attr_string, "");
  /*
   * If mono, we start with the modifier (GET_MOD) and end with the modifier (GET_MOD).
   * For colour, we start with the modifier (GET_MOD) and end with the background (GET_BG)
   */
  for (j = start_with; j < 3; j++) {
    switch (j) {

      case GET_FG:
        colour_only = TRUE;
        match_value = (chtype) fg;
        break;

      case GET_BG:
        strcat(attr_string, "on ");
        colour_only = TRUE;
        match_value = (chtype) bg;
        break;

      default:
        colour_only = FALSE;
        match_value = mod;
        break;
    }
    for (i = 0; valid_attribs[i].attrib != NULL; i++) {
      if (colour_only) {
        /*
         * Foreground or background
         */
        if (!valid_attribs[i].attrib_modifier && match_value == (chtype) valid_attribs[i].actual_attrib && valid_attribs[i].colour_modifier == 0) {
          strcat(attr_string, (char *) valid_attribs[i].attrib);
          strcat(attr_string, " ");
          break;
        }
      } else {
        /*
         * Modifiers only - find all non-duplicate modifiers
         */
        if (valid_attribs[i].attrib_modifier && (match_value & valid_attribs[i].actual_attrib) && !(matched_modifiers & valid_attribs[i].actual_attrib)) {
          strcat(attr_string, (char *) valid_attribs[i].attrib);
          strcat(attr_string, " ");
          matched_modifiers |= valid_attribs[i].actual_attrib;
        }
      }
    }
  }
  return (attr_string);
}

/*
 * This function determines if a colour name is passed as the only argument.
 */
int is_valid_colour(char* colour) {
  int i;

  for (i = 0; valid_attribs[i].attrib != NULL; i++) {
    if (equal(valid_attribs[i].attrib, colour, valid_attribs[i].attrib_min_len) && valid_attribs[i].actual_colour) {
      return valid_attribs[i].actual_attrib;
    }
  }
  return (-1);
}
