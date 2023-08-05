// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Definitions of various CUA behaviours
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
 */

/*
 * Definitions of various THIGHLIGHT behaviours
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
  char_t *text;
  unsigned short min_len;
  int funkey;
  short (*function)(char_t *);
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
  char_t *params;
};
typedef struct commands COMMAND;

#include "proto.h"

/*
 *---------------------- *** IMPORTANT *** ----------------------------
 * Make sure that there is no chance two or more commands can be
 * matched based on number of significant characters.
 * To allow a command to be referenced from the command line, it must
 * have a min_len > 0. When the same command is in the table more than
 * once (because multiple keys are defined), only one of the definitions
 * can have a min_len > 0.
 *---------------------- *** IMPORTANT *** ----------------------------
 */

/*                                                                  Batch/ Set / SOS / RO   /edit/strip     /readv/input/reprof/CUA             /THIGHLIGHT  */
COMMAND command[] = {
  { (char_t *) "cursor", 0, KEY_RIGHT, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "right" },     /* comm1.c */
  { (char_t *) "cursor", 0, KEY_LEFT, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "left" },       /* comm1.c */
  { (char_t *) "cursor", 0, KEY_DOWN, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "down" },      /* comm1.c */
  { (char_t *) "cursor", 0, KEY_UP, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "up" },  /* comm1.c */
  { (char_t *) "add", 1, (-1), Add, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* comm1.c */
  { (char_t *) "alert", 5, (-1), Alert, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm1.c */
  { (char_t *) "all", 3, (-1), All, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm1.c */
  { (char_t *) "arbchar", 3, (-1), Arbchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "alt", 3, (-1), Alt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "autocolour", 10, (-1), Autocolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "autocolor", 9, (-1), Autocolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commset1.c */
  { (char_t *) "autosave", 2, (-1), Autosave, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "autoscroll", 6, (-1), Autoscroll, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "backward", 2, KEY_PPAGE, Backward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm1.c */
  { (char_t *) "backward", 2, KEY_PrevScreen, Backward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm1.c */
  { (char_t *) "backup", 4, (-1), Backup, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "beep", 4, (-1), BeepSound, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "bottom", 1, (-1), Bottom, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm1.c */
  { (char_t *) "boundmark", 9, (-1), Boundmark, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "cancel", 3, (-1), Cancel, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm1.c */
  { (char_t *) "cappend", 2, (-1), Cappend, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm1.c */
  { (char_t *) "case", 4, (-1), Case, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "ccancel", 2, (-1), Ccancel, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm1.c */
  { (char_t *) "cdelete", 2, (-1), Cdelete, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },     /* comm1.c */
  { (char_t *) "cfirst", 2, (-1), Cfirst, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm1.c */
  { (char_t *) "change", 1, (-1), Change, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" }, /* comm1.c */
  { (char_t *) "cinsert", 2, (-1), Cinsert, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },     /* comm1.c */
  { (char_t *) "clast", 3, (-1), Clast, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm1.c */
  { (char_t *) "clipboard", 9, (-1), THEClipboard, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (char_t *) "" },     /* comm1.c */
  { (char_t *) "clearerrorkey", 6, (-1), Clearerrorkey, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "clearscreen", 6, (-1), Clearscreen, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "clocate", 2, (-1), Clocate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm1.c */
  { (char_t *) "clock", 5, (-1), Clock, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "cmatch", 6, KEY_F(7), Cmatch, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm1.c */
  { (char_t *) "cmdarrows", 4, (-1), Cmdarrows, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "cmdline", 3, (-1), Cmdline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "cmsg", 4, (-1), Cmsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm1.c */
  { (char_t *) "command", 7, (-1), THECommand, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm1.c */
  { (char_t *) "colour", 6, (-1), Colour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "color", 5, (-1), Colour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commset1.c */
  { (char_t *) "colouring", 9, (-1), Colouring, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "coloring", 8, (-1), Colouring, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commset1.c */
  { (char_t *) "compat", 6, (-1), Compat, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "compress", 4, (-1), Compress, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },      /* comm1.c */
  { (char_t *) "copy", 4, (-1), Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },    /* comm1.c */
  { (char_t *) "controlchar", 8, KEY_F(11), ControlChar, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm1.c */
  { (char_t *) "copy", 0, KEY_C_c, Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "block reset" },      /* comm1.c */
  { (char_t *) "copy", 0, KEY_C_k, Copy, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "block" },    /* comm1.c */
  { (char_t *) "cursor", 3, KEY_HOME, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "home save" }, /* comm1.c */
  { (char_t *) "cursor", 0, KEY_F16, Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "home save" },  /* comm1.c */
  { (char_t *) "cursor", 0, KEY_F(12), Cursor, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "prefix" },   /* comm1.c */
  { (char_t *) "ctlchar", 3, (-1), Ctlchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "coverlay", 3, (-1), Coverlay, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm1.c */
  { (char_t *) "creplace", 2, (-1), Creplace, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm1.c */
  { (char_t *) "curline", 4, (-1), Curline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "cursorstay", 8, (-1), CursorStay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "define", 3, (-1), Define, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm2.c */
  { (char_t *) "delete", 3, (-1), DeleteLine, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (char_t *) "" },    /* comm2.c */
  { (char_t *) "delete", 0, KEY_C_g, DeleteLine, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (char_t *) "block" },   /* comm2.c */
  { (char_t *) "dialog", 6, (-1), Dialog, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm2.c */
  { (char_t *) "directory", 3, (-1), Directory, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm2.c */
  { (char_t *) "ls", 2, (-1), Directory, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm2.c */
  { (char_t *) "defsort", 7, (-1), Defsort, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "display", 4, (-1), Display, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "down", 1, (-1), THENext, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm3.c */
  { (char_t *) "duplicate", 3, (-1), Duplicate, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm2.c */
  { (char_t *) "duplicate", 0, KEY_C_d, Duplicate, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "1 BLOCK" },      /* comm2.c */
  { (char_t *) "enter", 0, KEY_C_m, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },        /* comm2.c */
  { (char_t *) "enter", 0, KEY_ENTER, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },      /* comm2.c */
  { (char_t *) "enter", 5, KEY_RETURN, Enter, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm2.c */
  { (char_t *) "ecolor", 6, (-1), Ecolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "ecolour", 7, (-1), Ecolour, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "editv", 5, (-1), THEEditv, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm2.c */
  { (char_t *) "emsg", 4, (-1), Emsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm2.c */
  { (char_t *) "eolout", 4, (-1), Eolout, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "equivchar", 6, (-1), Equivchar, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "errorformat", 6, (-1), Errorformat, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "erroroutput", 8, (-1), Erroroutput, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "etmode", 6, (-1), Etmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "expand", 3, (-1), Expand, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },  /* comm2.c */
  { (char_t *) "extract", 3, (-1), Extract, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm2.c */
  { (char_t *) "ffile", 2, (-1), Ffile, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm2.c */
  { (char_t *) "file", 4, KEY_F(3), File, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm2.c */
  { (char_t *) "file", 0, KEY_PF3, File, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm2.c */
  { (char_t *) "fillbox", 4, KEY_C_f, Fillbox, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* comm2.c */
  { (char_t *) "filetabs", 8, (-1), THEFiletabs, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commset1.c */
  { (char_t *) "find", 1, (-1), Find, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },      /* comm2.c */
  { (char_t *) "findup", 5, (-1), Findup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },  /* comm2.c */
  { (char_t *) "fup", 2, (-1), Findup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm2.c */
  { (char_t *) "fext", 2, (-1), Fext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "filename", 5, (-1), Filename, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "fmode", 2, (-1), Fmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "fname", 2, (-1), Fname, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "fpath", 2, (-1), Fpath, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "ftype", 2, (-1), Fext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commset1.c */
  { (char_t *) "forward", 2, KEY_NPAGE, Forward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm2.c */
  { (char_t *) "forward", 2, KEY_NextScreen, Forward, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm2.c */
  { (char_t *) "fullfname", 6, (-1), Fullfname, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "get", 3, (-1), Get, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* comm2.c */
  { (char_t *) "header", 4, (-1), THEHeader, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commset1.c */
  { (char_t *) "help", 4, KEY_F(1), Help, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm2.c */
  { (char_t *) "help", 0, KEY_PF1, Help, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm2.c */
  { (char_t *) "hex", 3, (-1), Hex, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "hexdisplay", 7, (-1), Hexdisplay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "hexshow", 4, (-1), Hexshow, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "highlight", 4, (-1), Highlight, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "hit", 3, (-1), Hit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm2.c */
  { (char_t *) "idline", 2, (-1), Idline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "impmacro", 5, (-1), Impmacro, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "impos", 5, (-1), Impos, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "impcmscp", 3, (-1), Impos, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "input", 1, (-1), Input, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm2.c */
  { (char_t *) "inputmode", 6, (-1), Inputmode, TRUE, TRUE, FALSE, FALSE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "interface", 3, (-1), THEInterface, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "insertmode", 7, KEY_IC, Insertmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "toggle" }, /* commset1.c */
  { (char_t *) "locate", 1, KEY_F(11), Locate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm3.c */
  { (char_t *) "left", 2, (-1), Left, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm3.c */
  { (char_t *) "lineflag", 8, (-1), Lineflag, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "linend", 5, (-1), Linend, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "lastop", 6, (-1), Lastop, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset1.c */
  { (char_t *) "lowercase", 3, (-1), Lowercase, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm3.c */
  { (char_t *) "macro", 5, (-1), Macro, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm3.c */
  { (char_t *) "macro", 5, (-1), SetMacro, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset1.c */
  { (char_t *) "macroext", 6, (-1), Macroext, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "macropath", 6, (-1), Macropath, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset1.c */
  { (char_t *) "margins", 3, (-1), Margins, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "mark", 4, (-1), Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (char_t *) "" },    /* comm3.c */
  { (char_t *) "modify", 3, (-1), Modify, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm3.c */
  { (char_t *) "mouse", 5, (-1), Mouse, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (char_t *) "" }, /* commset1.c */
  { (char_t *) "", 0, KEY_MOUSE, THEMouse, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (char_t *) "" },    /* comm3.c */
  { (char_t *) "mark", 0, KEY_C_l, Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (char_t *) "line" },     /* comm3.c */
  { (char_t *) "mark", 0, KEY_C_b, Mark, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_ALL, (char_t *) "box" },      /* comm3.c */
  { (char_t *) "move", 4, KEY_C_v, THEMove, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "block reset" },   /* comm3.c */
  { (char_t *) "nextwindow", 5, KEY_F(2), Nextwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm3.c */
  { (char_t *) "nextwindow", 5, KEY_PF2, Nextwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm3.c */
  { (char_t *) "overlaybox", 8, KEY_C_o, Overlaybox, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm3.c */
  { (char_t *) "nfind", 2, (-1), Nfind, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },    /* comm3.c */
  { (char_t *) "nfindup", 6, (-1), Nfindup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },        /* comm3.c */
  { (char_t *) "nfup", 3, (-1), Nfindup, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* comm3.c */
  { (char_t *) "msg", 3, (-1), Msg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm3.c */
  { (char_t *) "msgline", 4, (-1), Msgline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "msgmode", 4, (-1), Msgmode, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "newlines", 4, (-1), Newlines, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset1.c */
  { (char_t *) "next", 1, (-1), THENext, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm3.c */
  { (char_t *) "nomsg", 5, (-1), Nomsg, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm3.c */
  { (char_t *) "nondisp", 4, (-1), Nondisp, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset1.c */
  { (char_t *) "nop", 3, (-1), Nop, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm3.c */
  { (char_t *) "number", 3, (-1), Number, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "osnowait", 3, (-1), Osnowait, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm3.c */
  { (char_t *) "osquiet", 3, (-1), Osquiet, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm3.c */
  { (char_t *) "osredir", 3, (-1), Osredir, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm3.c */
  { (char_t *) "os", 2, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm3.c */
  { (char_t *) "dosnowait", 4, (-1), Osnowait, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm3.c */
  { (char_t *) "dosquiet", 4, (-1), Osquiet, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm3.c */
  { (char_t *) "dos", 3, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm3.c */
  { (char_t *) "!", 1, (-1), Os, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm3.c */
  { (char_t *) "pagewrap", 8, (-1), Pagewrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "parser", 6, (-1), Parser, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "pending", 4, (-1), Pending, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "point", 1, (-1), Point, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "popup", 5, (-1), Popup, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "point", 0, KEY_F(31), Point, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) ".a" },   /* commset2.c */
  { (char_t *) "locate", 0, KEY_F(32), Locate, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) ".a" },        /* commset2.c */
  { (char_t *) "position", 3, (-1), Position, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "prefix", 3, (-1), Prefix, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "preserve", 4, (-1), Preserve, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm4.c */
  { (char_t *) "prevwindow", 5, (-1), Prevwindow, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm4.c */
  { (char_t *) "printer", 7, (-1), THEPrinter, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commset2.c */
  { (char_t *) "print", 2, (-1), Print, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm4.c */
  { (char_t *) "pscreen", 4, (-1), Pscreen, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "put", 3, (-1), Put, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm4.c */
  { (char_t *) "putd", 4, (-1), Putd, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },     /* comm4.c */
  { (char_t *) "qquit", 2, KEY_F(13), Qquit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm4.c */
  { (char_t *) "quit", 4, (-1), Quit, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm4.c */
  { (char_t *) "query", 1, (-1), Query, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "record", 6, (-1), THERecord, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm4.c */
  { (char_t *) "recover", 3, KEY_F(8), Recover, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },  /* comm4.c */
  { (char_t *) "readv", 5, (-1), Readv, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm4.c */
  { (char_t *) "readonly", 8, (-1), THEReadonly, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commset2.c */
  { (char_t *) "redit", 5, (-1), Redit, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "redraw", 6, KEY_C_r, Redraw, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm4.c */
  { (char_t *) "refresh", 7, (-1), THERefresh, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm4.c */
  { (char_t *) "repeat", 4, (-1), Repeat, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm4.c */
  { (char_t *) "replace", 1, (-1), Replace, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },     /* comm4.c */
  { (char_t *) "reprofile", 6, (-1), Reprofile, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "reserved", 5, (-1), Reserved, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "reset", 3, (-1), Reset, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "reset", 0, KEY_C_u, Reset, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "block" },        /* comm4.c */
  { (char_t *) "restore", 3, (-1), Restore, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm4.c */
  { (char_t *) "rexxhalt", 8, (-1), Rexxhalt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "rexxoutput", 7, (-1), Rexxoutput, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "rgtleft", 7, (-1), Rgtleft, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm4.c */
  { (char_t *) "rexx", 4, (-1), THERexx, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "right", 2, (-1), Right, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm4.c */
  { (char_t *) "=", 1, (-1), Reexecute, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm5.c */
  { (char_t *) "?", 1, KEY_F(6), Retrieve, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm5.c */
  { (char_t *) "save", 4, (-1), Save, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm4.c */
  { (char_t *) "scale", 4, (-1), Scale, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "schange", 3, (-1), Schange, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },      /* comm4.c */
  { (char_t *) "screen", 3, (-1), THEScreen, FALSE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "scope", 5, (-1), Scope, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "search", 3, (-1), THESearch, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm4.c */
  { (char_t *) "select", 3, (-1), Select, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "set", 3, (-1), Set, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "shadow", 4, (-1), Shadow, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "shift", 2, (-1), Shift, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* comm4.c */
  { (char_t *) "showkey", 4, (-1), ShowKey, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm4.c */
  { (char_t *) "slk", 3, (-1), Slk, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "sort", 4, (-1), Sort, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },      /* comm4.c */
  { (char_t *) "sos", 3, (-1), Sos, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (char_t *) "" },    /* comm4.c */
  { (char_t *) "bottomedge", 7, (-1), Sos_bottomedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "current", 4, (-1), Sos_current, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commsos.c */
  { (char_t *) "endchar", 4, (-1), Sos_endchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commsos.c */
  { (char_t *) "execute", 2, (-1), Sos_execute, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },    /* commsos.c */
  { (char_t *) "firstchar", 7, (-1), Sos_firstchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commsos.c */
  { (char_t *) "firstcol", 7, (-1), Sos_firstcol, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commsos.c */
  { (char_t *) "instab", 6, (-1), Sos_instab, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },     /* commsos.c */
  { (char_t *) "lastcol", 6, (-1), Sos_lastcol, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commsos.c */
  { (char_t *) "leftedge", 5, (-1), Sos_leftedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commsos.c */
  { (char_t *) "marginl", 7, (-1), Sos_marginl, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commsos.c */
  { (char_t *) "marginr", 7, (-1), Sos_marginr, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commsos.c */
  { (char_t *) "pastecmdline", 6, KEY_C_p, Sos_pastecmdline, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commsos.c */
  { (char_t *) "parindent", 6, (-1), Sos_parindent, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "prefix", 3, (-1), Sos_prefix, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "qcmnd", 2, (-1), Sos_qcmnd, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "rightedge", 6, (-1), Sos_rightedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "tabfieldb", 9, (-1), Sos_tabfieldb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "tabfieldf", 8, (-1), Sos_tabfieldf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "topedge", 4, (-1), Sos_topedge, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* commsos.c */
  { (char_t *) "blockstart", 6, (-1), Sos_blockstart, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "blockend", 6, (-1), Sos_blockend, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commsos.c */
  { (char_t *) "cursoradj", 7, KEY_C_a, Sos_cursoradj, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },  /* commsos.c */
  { (char_t *) "cursorshift", 11, (-1), Sos_cursorshift, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },        /* commsos.c */
  { (char_t *) "cuadelback", 8, (-1), Sos_cuadelback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },    /* commsos.c */
  { (char_t *) "cuadelchar", 7, (-1), Sos_cuadelchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },    /* commsos.c */
  { (char_t *) "addline", 3, KEY_F(4), Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* commsos.c */
  { (char_t *) "addline", 3, KEY_PF4, Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },        /* commsos.c */
  { (char_t *) "tabb", 4, (-1), Sos_tabb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commsos.c */
  { (char_t *) "delback", 4, KEY_C_h, Sos_delback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },       /* commsos.c */
  { (char_t *) "delback", 4, KEY_BACKSPACE, Sos_delback, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" }, /* commsos.c */
  { (char_t *) "delchar", 4, KEY_Remove, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },    /* commsos.c */
  { (char_t *) "delchar", 4, KEY_DELETE, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },    /* commsos.c */
  { (char_t *) "delchar", 4, KEY_DC, Sos_delchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },        /* commsos.c */
  { (char_t *) "delend", 4, KEY_C_e, Sos_delend, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },  /* commsos.c */
  { (char_t *) "delline", 4, KEY_F(9), Sos_delline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* commsos.c */
  { (char_t *) "delword", 4, KEY_C_w, Sos_delword, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },        /* commsos.c */
  { (char_t *) "doprefix", 5, KEY_C_p, Sos_doprefix, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commsos.c */
  { (char_t *) "edit", 4, KEY_C_x, Sos_edit, FALSE, FALSE, TRUE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "startendchar", 9, KEY_F(20), Sos_startendchar, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "makecurr", 8, KEY_F(5), Sos_makecurr, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "tabf", 3, KEY_C_i, Sos_tabf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commsos.c */
  { (char_t *) "tabwordb", 8, KEY_F(18), Sos_tabwordb, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "tabwordf", 7, KEY_F(19), Sos_tabwordf, FALSE, FALSE, TRUE, TRUE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* commsos.c */
  { (char_t *) "undo", 4, KEY_C_q, Sos_undo, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, TRUE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },     /* commsos.c */
  { (char_t *) "split", 2, KEY_C_s, Split, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "aligned cursor" },      /* comm4.c */
  { (char_t *) "join", 1, KEY_C_j, Join, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "aligned cursor" },   /* comm2.c */
  { (char_t *) "spltjoin", 8, KEY_F(10), Spltjoin, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* comm4.c */
  { (char_t *) "lineadd", 5, (-1), Sos_addline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* commsos.c */
  { (char_t *) "linedel", 5, (-1), Sos_delline, FALSE, FALSE, TRUE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* commsos.c */
  { (char_t *) "ssave", 2, (-1), Ssave, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* comm4.c */
  { (char_t *) "status", 4, (-1), Status, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm4.c */
  { (char_t *) "statusline", 7, (-1), Statusline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "statopt", 7, (-1), Statopt, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "stay", 4, (-1), Stay, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "synonym", 3, (-1), Synonym, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "suspend", 4, KEY_C_z, Suspend, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm4.c */
  { (char_t *) "tabpre", 0, (-1), Tabpre, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* comm5.c */
  { (char_t *) "tabfile", 7, (-1), Tabfile, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* comm5.c */
  { (char_t *) "tabline", 4, (-1), Tabline, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "tabkey", 4, (-1), Tabkey, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "tabs", 4, (-1), Tabs, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "tabsin", 5, (-1), Tabsin, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "tabsout", 5, (-1), Tabsout, TRUE, TRUE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* commset2.c */
  { (char_t *) "tag", 3, (-1), Tag, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm5.c */
  { (char_t *) "targetsave", 10, (-1), Targetsave, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset2.c */
  { (char_t *) "text", 4, (-1), Text, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_NONE, TRUE, FALSE, TRUE, CUA_DELETE_BLOCK, THIGHLIGHT_RESET_FOCUS, (char_t *) "" },   /* comm5.c */
  { (char_t *) "thighlight", 5, (-1), THighlight, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "timecheck", 9, (-1), Timecheck, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "toascii", 7, (-1), Toascii, FALSE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },       /* comm5.c */
  { (char_t *) "tofeof", 6, (-1), Tofeof, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "top", 3, (-1), Top, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },     /* comm5.c */
  { (char_t *) "trailing", 8, (-1), Trailing, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "trunc", 5, (-1), Trunc, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "typeahead", 5, (-1), THETypeahead, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* commset2.c */
  { (char_t *) "undoing", 7, (-1), Undoing, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },      /* commset2.c */
  { (char_t *) "up", 1, (-1), Up, TRUE, FALSE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },       /* comm5.c */
  { (char_t *) "uppercase", 3, (-1), Uppercase, TRUE, FALSE, FALSE, FALSE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_RESET_ALL, (char_t *) "" },   /* comm5.c */
  { (char_t *) "verify", 1, (-1), Verify, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },        /* commset2.c */
  { (char_t *) "width", 5, (-1), Width, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "word", 4, (-1), Word, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "wrap", 2, (-1), Wrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "wordwrap", 5, (-1), Wordwrap, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { (char_t *) "edit", 1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* comm5.c */
  { (char_t *) "xedit", 1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm5.c */
  { (char_t *) "kedit", 1, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" }, /* comm5.c */
  { (char_t *) "the", 3, (-1), Xedit, TRUE, FALSE, FALSE, TRUE, TRUE, STRIP_BOTH, FALSE, FALSE, FALSE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },   /* comm5.c */
  { (char_t *) "untaa", 5, (-1), Untaa, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },  /* commset2.c */
  { (char_t *) "zone", 1, (-1), Zone, TRUE, TRUE, FALSE, TRUE, FALSE, STRIP_BOTH, FALSE, FALSE, TRUE, CUA_RESET_BLOCK, THIGHLIGHT_NONE, (char_t *) "" },    /* commset2.c */
  { NULL, 0, (-1), NULL, FALSE, FALSE, FALSE, FALSE, FALSE, STRIP_NONE, FALSE, FALSE, TRUE, CUA_NONE, THIGHLIGHT_NONE, (char_t *) "" }
};

// removed -> dirinclude, fdisplay, mouseclick, regexp, xterminal, span, spill

