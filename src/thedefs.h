// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#define THE_VERSION      "4x"
#define THE_VERSION_DATE "today"
#define THE_COPYRIGHT    "Copyright 1991-2020 Mark Hessling"

/*---------------------------------------------------------------------*/
/* The following values can be changed to suit your needs.             */
/*---------------------------------------------------------------------*/
#define MAX_FILE_NAME             1000  /* maximum length of fully qualified file */
#define MAX_LENGTH_OF_LINE        1000  /* default maximum length of a line */
#define MAX_COMMANDS                10  /* default maximum number of commands allowed on command line */
#define MAX_RECV                    20  /* number of lines that can be recovered */
#define MAX_SAVED_COMMANDS          20  /* number of commands that can be retrieved */
#define MAX_NUMTABS                 32  /* number of tab stops that can be defined */
#define MAXIMUM_POPUP_KEYS          20  /* maximum number of keys in popup menu */
#define MAXIMUM_DIALOG_LINES       100  /* maximum number of lines in a dailog */

/*---------------------------------------------------------------------*/
/* The following values should not be changed unless you know what you */
/* are doing !                                                         */
/*---------------------------------------------------------------------*/
typedef unsigned short row_t;
typedef unsigned short col_t;
typedef unsigned char char_t;
typedef long length_t;        /* was unsigned short *//* MUST be signed!!! */
typedef long line_t;
typedef unsigned short select_t;

