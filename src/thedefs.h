// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#define THE_VERSION      "3.3r8"
#define THE_VERSION_DATE "today"
#define THE_COPYRIGHT    "Copyright 1991-2020 Mark Hessling";

/* Define this always; build number.  */
/* When next build requires configure to be run again, increment this */
/* Format is xxyy where xx is version, yy build number */
#define BUILD3001 1

/* Defines the kernel name */
#define MH_KERNEL_NAME "Linux"

/*
 * The following values can be changed to suit your needs.
 */
#define MAX_FILE_NAME             1000  /* maximum length of fully qualified file */
#define MAX_LENGTH_OF_LINE        1000  /* default maximum length of a line */
#define MAX_COMMANDS                10  /* default maximum number of commands allowed on command line */
#define MAX_RECV                    20  /* number of lines that can be recovered */
#define MAX_SAVED_COMMANDS          20  /* number of commands that can be retrieved */
#define MAX_NUMTABS                 32  /* number of tab stops that can be defined */
#define MAXIMUM_POPUP_KEYS          20  /* maximum number of keys in popup menu */
#define MAXIMUM_DIALOG_LINES       100  /* maximum number of lines in a dailog */

/*
 * The following values should not be changed unless you know what you are doing !
 */
typedef unsigned short row_t;
typedef unsigned short col_t;
typedef long length_t;        /* was unsigned short -- MUST be signed!!! */
typedef long line_t;
typedef unsigned char select_t;

typedef unsigned char byte;

/*
 * A default char type to avoid compiler warnings.
 */
typedef char DEFCHAR;
typedef unsigned char uchar;
typedef unsigned char char_t;
typedef unsigned short ROWTYPE;
typedef unsigned short COLTYPE;
typedef unsigned char CHARTYPE;
typedef long LENGTHTYPE;
typedef long LINETYPE;
typedef unsigned char SELECTTYPE;

