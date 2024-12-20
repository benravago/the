// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "proto.h"

/*
 * Definitions of various CUA behaviours
 *
 * CUA_NONE:
 * - no affect. ie no automatic processing is done, but the
 *   individual command can handle CUA behaviour itself. eg
 *   CURSOR command would need to handle CUA behaviour.
 *
 * CUA_DELETE_BLOCK
 * - save first line/column position (not anchor position)
 * - delete the marked CUA block (ignores other blocks )
 * - move cursor to first line/column (not anchor position)
 *
 * CUA_RESET_BLOCK
 * - reset CUA block
 *
 * Definitions of various THIGHLIGHT behaviours
 *
 * THIGHLIGHT_NONE:
 * - no affect. ie no automatic resetting of the THIGHLIGHT
 *   area.
 *
 * THIGHLIGHT_RESET_ALL
 * - reset the THIGHLIGHT area in all circumstances
 *
 * THIGHLIGHT_RESET_FOCUS
 * - reset CUA THIGHLIGHT area if this command executed on
 *   the focus line and this line contains the THIGHLIGHT area
 */

struct commands {
  uchar *text;
  unsigned short min_len;
  int funkey;
  short (*function)(uchar *);
  bool valid_batch_command;
  bool set_command;
  bool sos_command;
  bool valid_in_readonly;
  bool edit_command;
  bool strip_param;
  bool valid_readv;
  bool valid_input_full;
  bool valid_for_reprofile;
  short cua_behaviour;
  short thighlight_behaviour;
  uchar *params;
};
typedef struct commands COMMAND;

/* IMPORTANT
 *
 * Make sure that there is no chance two or more commands can be matched based on number of significant characters.
 * To allow a command to be referenced from the command line, it must have a min_len > 0.
 * When the same command is in the table more than once (because multiple keys are defined),
 * only one of the definitions can have a min_len > 0.
 */

COMMAND command[] =
{
{(uchar*) "cursor",        0, KEY_RIGHT, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "right" },   /* comm1.c */
{(uchar*) "cursor",        0, KEY_LEFT, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "left" },     /* comm1.c */
{(uchar*) "cursor",        0, KEY_DOWN, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "down" },    /* comm1.c */
{(uchar*) "cursor",        0, KEY_UP, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "up" },        /* comm1.c */
{(uchar*) "add",           1, (-1), Add, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* comm1.c */
{(uchar*) "alert",         5, (-1), Alert, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm1.c */
{(uchar*) "all",           3, (-1), All, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm1.c */
{(uchar*) "arbchar",       3, (-1), Arbchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "alt",           3, (-1), Alt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "autocolour",   10, (-1), Autocolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "autocolor",     9, (-1), Autocolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commset1.c */
{(uchar*) "autosave",      2, (-1), Autosave, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "autoscroll",    6, (-1), Autoscroll, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "backward",      2, KEY_PPAGE, Backward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm1.c */
{(uchar*) "backward",      2, KEY_PrevScreen, Backward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm1.c */
{(uchar*) "backup",        4, (-1), Backup, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "beep",          4, (-1), BeepSound, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "bottom",        1, (-1), Bottom, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm1.c */
{(uchar*) "boundmark",     9, (-1), Boundmark, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "cancel",        3, (-1), Cancel, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm1.c */
{(uchar*) "cappend",       2, (-1), Cappend, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm1.c */
{(uchar*) "case",          4, (-1), Case, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "ccancel",       2, (-1), Ccancel, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm1.c */
{(uchar*) "cdelete",       2, (-1), Cdelete, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },   /* comm1.c */
{(uchar*) "cfirst",        2, (-1), Cfirst, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm1.c */
{(uchar*) "change",        1, (-1), Change, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },       /* comm1.c */
{(uchar*) "cinsert",       2, (-1), Cinsert, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },   /* comm1.c */
{(uchar*) "clast",         3, (-1), Clast, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm1.c */
{(uchar*) "clearerrorkey", 6, (-1), Clearerrorkey, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "clearscreen",   6, (-1), Clearscreen, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "clocate",       2, (-1), Clocate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm1.c */
{(uchar*) "clock",         5, (-1), Clock, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "cmatch",        6, KEY_F(7), Cmatch, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm1.c */
{(uchar*) "cmdarrows",     4, (-1), Cmdarrows, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "cmdline",       3, (-1), Cmdline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "cmsg",          4, (-1), Cmsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm1.c */
{(uchar*) "command",       7, (-1), THECommand, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm1.c */
{(uchar*) "colour",        6, (-1), Colour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "color",         5, (-1), Colour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commset1.c */
{(uchar*) "colouring",     9, (-1), Colouring, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "coloring",      8, (-1), Colouring, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commset1.c */
{(uchar*) "compat",        6, (-1), Compat, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "compress",      4, (-1), Compress, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },    /* comm1.c */
{(uchar*) "copy",          4, (-1), Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },  /* comm1.c */
{(uchar*) "controlchar",   8, KEY_F(11), ControlChar, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm1.c */
{(uchar*) "copy",          0, KEY_C_c, Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "block reset" },    /* comm1.c */
{(uchar*) "copy",          0, KEY_C_k, Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "block" },  /* comm1.c */
{(uchar*) "cursor",        3, KEY_HOME, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "home save" },       /* comm1.c */
{(uchar*) "cursor",        0, KEY_F16, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "home save" },        /* comm1.c */
{(uchar*) "cursor",        0, KEY_F(12), Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "prefix" }, /* comm1.c */
{(uchar*) "ctlchar",       3, (-1), Ctlchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "coverlay",      3, (-1), Coverlay, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm1.c */
{(uchar*) "creplace",      2, (-1), Creplace, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm1.c */
{(uchar*) "curline",       4, (-1), Curline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "cursorstay",    8, (-1), CursorStay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "define",        3, (-1), Define, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm2.c */
{(uchar*) "delete",        3, (-1), DeleteLine, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (uchar *) "" },  /* comm2.c */
{(uchar*) "delete",        0, KEY_C_g, DeleteLine, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (uchar *) "block" }, /* comm2.c */
{(uchar*) "dialog",        6, (-1), Dialog, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm2.c */
{(uchar*) "directory",     3, (-1), Directory, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm2.c */
{(uchar*) "ls",            2, (-1), Directory, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm2.c */
{(uchar*) "defsort",       7, (-1), Defsort, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "display",       4, (-1), Display, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "down",          1, (-1), THENext, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm3.c */
{(uchar*) "duplicate",     3, (-1), Duplicate, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm2.c */
{(uchar*) "duplicate",     0, KEY_C_d, Duplicate, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "1 BLOCK" },    /* comm2.c */
{(uchar*) "enter",         0, KEY_C_m, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },      /* comm2.c */
{(uchar*) "enter",         0, KEY_ENTER, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },    /* comm2.c */
{(uchar*) "enter",         5, KEY_RETURN, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm2.c */
{(uchar*) "ecolor",        6, (-1), Ecolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "ecolour",       7, (-1), Ecolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "editv",         5, (-1), THEEditv, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm2.c */
{(uchar*) "emsg",          4, (-1), Emsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm2.c */
{(uchar*) "eolout",        4, (-1), Eolout, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "equivchar",     6, (-1), Equivchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "errorformat",   6, (-1), Errorformat, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "erroroutput",   8, (-1), Erroroutput, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "etmode",        6, (-1), Etmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "expand",        3, (-1), Expand, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },        /* comm2.c */
{(uchar*) "extract",       3, (-1), Extract, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm2.c */
{(uchar*) "fdisplay",      8, (-1), Fdisplay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "ffile",         2, (-1), Ffile, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm2.c */
{(uchar*) "file",          4, KEY_F(3), File, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm2.c */
{(uchar*) "file",          0, KEY_PF3, File, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm2.c */
{(uchar*) "fillbox",       4, KEY_C_f, Fillbox, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* comm2.c */
{(uchar*) "filetabs",      8, (-1), THEFiletabs, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commset1.c */
{(uchar*) "find",          1, (-1), Find, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },    /* comm2.c */
{(uchar*) "findup",        5, (-1), Findup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },        /* comm2.c */
{(uchar*) "fup",           2, (-1), Findup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm2.c */
{(uchar*) "fext",          2, (-1), Fext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "filename",      5, (-1), Filename, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "fname",         2, (-1), Fname, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "fpath",         2, (-1), Fpath, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "ftype",         2, (-1), Fext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commset1.c */
{(uchar*) "forward",       2, KEY_NPAGE, Forward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm2.c */
{(uchar*) "forward",       2, KEY_NextScreen, Forward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm2.c */
{(uchar*) "fullfname",     6, (-1), Fullfname, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "get",           3, (-1), Get, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* comm2.c */
{(uchar*) "header",        4, (-1), THEHeader, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commset1.c */
{(uchar*) "help",          4, KEY_F(1), Help, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm2.c */
{(uchar*) "help",          0, KEY_PF1, Help, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm2.c */
{(uchar*) "hex",           3, (-1), Hex, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "hexdisplay",    7, (-1), Hexdisplay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "hexshow",       4, (-1), Hexshow, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "highlight",     4, (-1), Highlight, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "hit",           3, (-1), Hit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm2.c */
{(uchar*) "idline",        2, (-1), Idline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "impmacro",      5, (-1), Impmacro, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "impos",         5, (-1), Impos, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "impcmscp",      3, (-1), Impos, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "input",         1, (-1), Input, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm2.c */
{(uchar*) "inputmode",     6, (-1), Inputmode, TRUE, TRUE, FALSE, FALSE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "interface",     3, (-1), THEInterface, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "insertmode",    7, KEY_IC, Insertmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "toggle" },       /* commset1.c */
{(uchar*) "locate",        1, KEY_F(11), Locate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm3.c */
{(uchar*) "left",          2, (-1), Left, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm3.c */
{(uchar*) "lineflag",      8, (-1), Lineflag, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "linend",        5, (-1), Linend, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "lastop",        6, (-1), Lastop, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset1.c */
{(uchar*) "lowercase",     3, (-1), Lowercase, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm3.c */
{(uchar*) "macro",         5, (-1), Macro, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm3.c */
{(uchar*) "macro",         5, (-1), SetMacro, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "macroext",      6, (-1), Macroext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "macropath",     6, (-1), Macropath, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset1.c */
{(uchar*) "margins",       3, (-1), Margins, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "mark",          4, (-1), Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (uchar *) "" },  /* comm3.c */
{(uchar*) "modify",        3, (-1), Modify, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm3.c */
{(uchar*) "mouse",         5, (-1), Mouse, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (uchar *) "" },       /* commset1.c */
{(uchar*) "mouseclick",    7, (-1), Mouseclick, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (uchar *) "" },     /* commset1.c */
{(uchar*) "",              0, KEY_MOUSE, THEMouse, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (uchar *) "" },  /* comm3.c */
{(uchar*) "mark",          0, KEY_C_l, Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (uchar *) "line" },   /* comm3.c */
{(uchar*) "mark",          0, KEY_C_b, Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (uchar *) "box" },    /* comm3.c */
{(uchar*) "move",          4, KEY_C_v, THEMove, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "block reset" }, /* comm3.c */
{(uchar*) "nextwindow",    5, KEY_F(2), Nextwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm3.c */
{(uchar*) "nextwindow",    5, KEY_PF2, Nextwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm3.c */
{(uchar*) "overlaybox",    8, KEY_C_o, Overlaybox, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm3.c */
{(uchar*) "nfind",         2, (-1), Nfind, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },  /* comm3.c */
{(uchar*) "nfindup",       6, (-1), Nfindup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },      /* comm3.c */
{(uchar*) "nfup",          3, (-1), Nfindup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* comm3.c */
{(uchar*) "msg",           3, (-1), Msg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm3.c */
{(uchar*) "msgline",       4, (-1), Msgline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "msgmode",       4, (-1), Msgmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "newlines",      4, (-1), Newlines, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset1.c */
{(uchar*) "next",          1, (-1), THENext, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm3.c */
{(uchar*) "nomsg",         5, (-1), Nomsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm3.c */
{(uchar*) "nondisp",       4, (-1), Nondisp, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset1.c */
{(uchar*) "nop",           3, (-1), Nop, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm3.c */
{(uchar*) "number",        3, (-1), Number, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "osnowait",      3, (-1), Osnowait, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm3.c */
{(uchar*) "osquiet",       3, (-1), Osquiet, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm3.c */
{(uchar*) "osredir",       3, (-1), Osredir, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm3.c */
{(uchar*) "os",            2, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm3.c */
{(uchar*) "dosnowait",     4, (-1), Osnowait, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm3.c */
{(uchar*) "dosquiet",      4, (-1), Osquiet, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm3.c */
{(uchar*) "dos",           3, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm3.c */
{(uchar*) "!",             1, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm3.c */
{(uchar*) "pagewrap",      8, (-1), Pagewrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "parser",        6, (-1), Parser, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "pending",       4, (-1), Pending, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "point",         1, (-1), Point, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "popup",         5, (-1), Popup, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "point",         0, KEY_F(31), Point, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) ".a" }, /* commset2.c */
{(uchar*) "locate",        0, KEY_F(32), Locate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) ".a" },      /* commset2.c */
{(uchar*) "position",      3, (-1), Position, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "prefix",        3, (-1), Prefix, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "preserve",      4, (-1), Preserve, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm4.c */
{(uchar*) "prevwindow",    5, (-1), Prevwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* comm4.c */
{(uchar*) "printer",       7, (-1), THEPrinter, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commset2.c */
{(uchar*) "print",         2, (-1), Print, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm4.c */
{(uchar*) "pscreen",       4, (-1), Pscreen, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "put",           3, (-1), Put, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm4.c */
{(uchar*) "putd",          4, (-1), Putd, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },   /* comm4.c */
{(uchar*) "qquit",         2, KEY_F(13), Qquit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm4.c */
{(uchar*) "quit",          4, (-1), Quit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm4.c */
{(uchar*) "query",         1, (-1), Query, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "record",        6, (-1), THERecord, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm4.c */
{(uchar*) "recover",       3, KEY_F(8), Recover, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },        /* comm4.c */
{(uchar*) "readv",         5, (-1), Readv, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm4.c */
{(uchar*) "readonly",      8, (-1), THEReadonly, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commset2.c */
{(uchar*) "redit",         5, (-1), Redit, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "redraw",        6, KEY_C_r, Redraw, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm4.c */
{(uchar*) "refresh",       7, (-1), THERefresh, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm4.c */
{(uchar*) "repeat",        4, (-1), Repeat, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm4.c */
{(uchar*) "replace",       1, (-1), Replace, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },   /* comm4.c */
{(uchar*) "reprofile",     6, (-1), Reprofile, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "reserved",      5, (-1), Reserved, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "reset",         3, (-1), Reset, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "reset",         0, KEY_C_u, Reset, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "block" },      /* comm4.c */
{(uchar*) "restore",       3, (-1), Restore, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm4.c */
{(uchar*) "rexxhalt",      8, (-1), Rexxhalt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "rexxoutput",    7, (-1), Rexxoutput, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "rgtleft",       7, (-1), Rgtleft, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm4.c */
{(uchar*) "rexx",          4, (-1), THERexx, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "right",         2, (-1), Right, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm4.c */
{(uchar*) "=",             1, (-1), Reexecute, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm5.c */
{(uchar*) "?",             1, KEY_F(6), Retrieve, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm5.c */
{(uchar*) "save",          4, (-1), Save, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm4.c */
{(uchar*) "scale",         4, (-1), Scale, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "schange",       3, (-1), Schange, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },    /* comm4.c */
{(uchar*) "screen",        3, (-1), THEScreen, FALSE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "scope",         5, (-1), Scope, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "search",        3, (-1), THESearch, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm4.c */
{(uchar*) "select",        3, (-1), Select, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "set",           3, (-1), Set, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "shadow",        4, (-1), Shadow, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "shift",         2, (-1), Shift, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* comm4.c */
{(uchar*) "showkey",       4, (-1), ShowKey, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm4.c */
{(uchar*) "slk",           3, (-1), Slk, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "sort",          4, (-1), Sort, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },    /* comm4.c */
{(uchar*) "sos",           3, (-1), Sos, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (uchar *) "" },  /* comm4.c */
{(uchar*) "bottomedge",    7, (-1), Sos_bottomedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "current",       4, (-1), Sos_current, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commsos.c */
{(uchar*) "endchar",       4, (-1), Sos_endchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commsos.c */
{(uchar*) "execute",       2, (-1), Sos_execute, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },  /* commsos.c */
{(uchar*) "firstchar",     7, (-1), Sos_firstchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commsos.c */
{(uchar*) "firstcol",      7, (-1), Sos_firstcol, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commsos.c */
{(uchar*) "instab",        6, (-1), Sos_instab, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },   /* commsos.c */
{(uchar*) "lastcol",       6, (-1), Sos_lastcol, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commsos.c */
{(uchar*) "leftedge",      5, (-1), Sos_leftedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commsos.c */
{(uchar*) "marginl",       7, (-1), Sos_marginl, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commsos.c */
{(uchar*) "marginr",       7, (-1), Sos_marginr, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commsos.c */
{(uchar*) "pastecmdline",  6, KEY_C_p, Sos_pastecmdline, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commsos.c */
{(uchar*) "parindent",     6, (-1), Sos_parindent, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "prefix",        3, (-1), Sos_prefix, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "qcmnd",         2, (-1), Sos_qcmnd, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "rightedge",     6, (-1), Sos_rightedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "tabfieldb",     9, (-1), Sos_tabfieldb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "tabfieldf",     8, (-1), Sos_tabfieldf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "topedge",       4, (-1), Sos_topedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* commsos.c */
{(uchar*) "blockstart",    6, (-1), Sos_blockstart, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "blockend",      6, (-1), Sos_blockend, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commsos.c */
{(uchar*) "cursoradj",     7, KEY_C_a, Sos_cursoradj, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },        /* commsos.c */
{(uchar*) "cursorshift",  11, (-1), Sos_cursorshift, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },      /* commsos.c */
{(uchar*) "cuadelback",    8, (-1), Sos_cuadelback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },  /* commsos.c */
{(uchar*) "cuadelchar",    7, (-1), Sos_cuadelchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },  /* commsos.c */
{(uchar*) "addline",       3, KEY_F(4), Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* commsos.c */
{(uchar*) "addline",       3, KEY_PF4, Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },      /* commsos.c */
{(uchar*) "tabb",          4, (-1), Sos_tabb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commsos.c */
{(uchar*) "delback",       4, KEY_C_h, Sos_delback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },     /* commsos.c */
{(uchar*) "delback",       4, KEY_BACKSPACE, Sos_delback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },       /* commsos.c */
{(uchar*) "delchar",       4, KEY_Remove, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },  /* commsos.c */
{(uchar*) "delchar",       4, KEY_DELETE, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },  /* commsos.c */
{(uchar*) "delchar",       4, KEY_DC, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },      /* commsos.c */
{(uchar*) "delend",        4, KEY_C_e, Sos_delend, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },        /* commsos.c */
{(uchar*) "delline",       4, KEY_F(9), Sos_delline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* commsos.c */
{(uchar*) "delword",       4, KEY_C_w, Sos_delword, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },      /* commsos.c */
{(uchar*) "doprefix",      5, KEY_C_p, Sos_doprefix, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commsos.c */
{(uchar*) "edit",          4, KEY_C_x, Sos_edit, FALSE, FALSE, TRUE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "startendchar",  9, KEY_F(20), Sos_startendchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "makecurr",      8, KEY_F(5), Sos_makecurr, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "tabf",          3, KEY_C_i, Sos_tabf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commsos.c */
{(uchar*) "tabwordb",      8, KEY_F(18), Sos_tabwordb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "tabwordf",      7, KEY_F(19), Sos_tabwordf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* commsos.c */
{(uchar*) "undo",          4, KEY_C_q, Sos_undo, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" },   /* commsos.c */
{(uchar*) "split",         2, KEY_C_s, Split, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "aligned cursor" },    /* comm4.c */
{(uchar*) "join",          1, KEY_C_j, Join, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "aligned cursor" }, /* comm2.c */
{(uchar*) "spltjoin",      8, KEY_F(10), Spltjoin, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* comm4.c */
{(uchar*) "lineadd",       5, (-1), Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* commsos.c */
{(uchar*) "linedel",       5, (-1), Sos_delline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* commsos.c */
{(uchar*) "ssave",         2, (-1), Ssave, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* comm4.c */
{(uchar*) "status",        4, (-1), Status, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm4.c */
{(uchar*) "statusline",    7, (-1), Statusline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "statopt",       7, (-1), Statopt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "stay",          4, (-1), Stay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "synonym",       3, (-1), Synonym, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "suspend",       4, KEY_C_z, Suspend, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm4.c */
{(uchar*) "tabfile",       7, (-1), Tabfile, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* comm5.c */
{(uchar*) "tabline",       4, (-1), Tabline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "tabkey",        4, (-1), Tabkey, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "tabs",          4, (-1), Tabs, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "tabsin",        5, (-1), Tabsin, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "tabsout",       5, (-1), Tabsout, TRUE, TRUE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* commset2.c */
{(uchar*) "tag",           3, (-1), Tag, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm5.c */
{(uchar*) "targetsave",   10, (-1), Targetsave, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset2.c */
{(uchar*) "text",          4, (-1), Text, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, TRUE, FALSE, TRUE, CUA_DELETE_BLOCK, THIGHLIGHT_RESET_FOCUS, (uchar *) "" }, /* comm5.c */
{(uchar*) "thighlight",    5, (-1), THighlight, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "timecheck",     9, (-1), Timecheck, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "toascii",       7, (-1), Toascii, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" },     /* comm5.c */
{(uchar*) "tofeof",        6, (-1), Tofeof, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "top",           3, (-1), Top, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },   /* comm5.c */
{(uchar*) "trailing",      8, (-1), Trailing, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "typeahead",     5, (-1), THETypeahead, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* commset2.c */
{(uchar*) "undoing",       7, (-1), Undoing, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },    /* commset2.c */
{(uchar*) "up",            1, (-1), Up, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },     /* comm5.c */
{(uchar*) "uppercase",     3, (-1), Uppercase, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (uchar *) "" }, /* comm5.c */
{(uchar*) "verify",        1, (-1), Verify, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },      /* commset2.c */
{(uchar*) "width",         5, (-1), Width, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "word",          4, (-1), Word, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "wrap",          2, (-1), Wrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "wordwrap",      5, (-1), Wordwrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */
{(uchar*) "edit",          1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* comm5.c */
{(uchar*) "xedit",         1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm5.c */
{(uchar*) "kedit",         1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },       /* comm5.c */
{(uchar*) "the",           3, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" }, /* comm5.c */
{(uchar*) "untaa",         5, (-1), Untaa, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },        /* commset2.c */
{(uchar*) "zone",          1, (-1), Zone, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (uchar *) "" },  /* commset2.c */

{NULL, 0, (-1), NULL, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (uchar *) "" }
};
